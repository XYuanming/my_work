#include "RN8302_COPY.h"
#pragma comment(lib,"libfftw3-3.lib")
//#include <fftw3.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

void pabort(const char *s)
{
    perror(s);
    abort();
}
void print_usage(const char *prog)
{
    printf("Usage: %s [-DsbdlHOLC3]\n", prog);
    puts("  -D --device   device to use (default /dev/spidev1.1)\n"
            "  -s --speed    max speed (Hz)\n"
            "  -d --delay    delay (usec)\n"
            "  -b --bpw      bits per word \n"
            "  -l --loop     loopback\n"
            "  -H --cpha     clock phase\n"
            "  -O --cpol     clock polarity\n"
            "  -L --lsb      least significant bit first\n"
            "  -C --cs-high  chip select active high\n"
            "  -3 --3wire    SI/SO signals shared\n");
    exit(1);
}

void parse_opts(int argc, char *argv[])  
{  
    while (1) {  
        static const struct option lopts[] = {  
            { "device",  1, 0, 'D' },  
            { "speed",   1, 0, 's' },  
            { "delay",   1, 0, 'd' },  
            { "bpw",     1, 0, 'b' },  
            { "loop",    0, 0, 'l' },  
            { "cpha",    0, 0, 'H' },  
            { "cpol",    0, 0, 'O' },  
            { "lsb",     0, 0, 'L' },  
            { "cs-high", 0, 0, 'C' },  
            { "3wire",   0, 0, '3' },  
            { "no-cs",   0, 0, 'N' },  
            { "ready",   0, 0, 'R' },  
            { NULL, 0, 0, 0 },  
        };  
        int c;  

        c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR", lopts, NULL);  

        if (c == -1)  
            break;  

        switch (c) {  
            case 'D':  
                device = optarg;  
                break;  
            case 's':  
                speed = atoi(optarg);  
                break;  
            case 'd':  
                delay = atoi(optarg);  
                break;  
            case 'b':  
                bits = atoi(optarg);  
                break;  
            case 'l':  
                mode |= SPI_LOOP;  
                break;  
            case 'H':  
                mode |= SPI_CPHA;  
                break;  
            case 'O':  
                mode |= SPI_CPOL;  
                break;  
            case 'L':  
                mode |= SPI_LSB_FIRST;  
                break;  
            case 'C':  
                mode |= SPI_CS_HIGH;  
                break;  
            case '3':  
                mode |= SPI_3WIRE;  
                break;  
            case 'N':  
                mode |= SPI_NO_CS;  
                break;  
            case 'R':  
                mode |= SPI_READY;  
                break;  
            default:  
                print_usage(argv[0]);  
                break;  
        }  
    }  
}  


uchar* Read8302(int fd,ushort Addr, int datalen)
{
    int ret;
    uchar ADDR=Addr & 0x00ff;
    uchar CMD=(Addr >>4)& 0xf0;
    uchar* tx = (uchar*)malloc(sizeof(uchar)*(datalen+3));
    uchar* rx = (uchar*)malloc(sizeof(uchar)*(datalen+3));
    //  memset(tx,0,sizeof(tx));
    tx[0]=ADDR;
    tx[1]=CMD;
    //    uchar rx[MALLOC_SIZE(tx)] = {0, };
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = datalen+3,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
    };
#if 0
    printf("\nwrite to spi => ");
    for (ret = 0; ret < datalen+3; ret++)
    {
        printf("%02X ", tx[ret]);
    }
#endif
    ret = ioctl(fd, SPI_IOC_MESSAGE(1),&tr);
    if (ret == 1)
    { 
        pabort("can't send spi message");
        free(tx);
        return NULL;
    }
#if 0
    printf("\nread from spi <= ");
    for (ret = 0; ret < datalen+3; ret++)
    {
        printf("%02X ", rx[ret]);
    }
    printf("\n");
#endif
    free(tx);
    uchar CS=ADDR+CMD;
    for (int i=0;i<datalen;i++)
    {
        CS+=rx[i+2];
    }
    CS=(CS&0xff)^0xff;
    if(rx[datalen+2]==CS)
        return rx+2;
    else
    { 
        printf("read failed :cs check error\n");
        return NULL;
    }
}
void Write8302(int fd,ushort Addr,uchar* pData,int DataLen)
{
    uchar ADDR=Addr & 0x00ff;
    uchar CMD=((Addr >>4)& 0xf0)+0x80;
    uchar CS=ADDR+CMD;
    uchar* message= (uchar*)malloc(DataLen+3);
    message[0]=ADDR;
    message[1]=CMD;
    for(int i=0;i<DataLen;i++)
    {
        message[2+i]=pData[i];
        CS+=pData[i];
    }
    CS=(CS&0xff)^0xff;
    message[DataLen+2]=CS;
    write(fd,message,DataLen+3);
#if 0
    printf("\nwrite to spi =>");
    for(int i=0;i<DataLen+3;i++)
    {
        printf("%02X ",message[i]);
    }
    printf("\n");
#endif
}

bool Check8302Exist(int fd)
{
    uchar* result = Read8302(fd,0x018f,3);
    if(result==NULL)
        return false;
    if(result[0]!=0x83 ||result[1]!=0x02 || result[2]!=0x00)
        return false;
    return true;
}

void  WriteRn8302Gain(int fd)
{
    uint ltmp=0xE5;
    Write8302(fd,WREN,(uchar*)&ltmp, 1);    // 电压增益初始化
    Write8302(fd,GSUA,(uchar*)&rn8302_para.KU_A,2);
    Write8302(fd,GSUB,(uchar*)&rn8302_para.KU_B,2);
    Write8302(fd,GSUC,(uchar*)&rn8302_para.KU_C,2);
    Write8302(fd,GSIA,(uchar*)&rn8302_para.KI_A,2);   //// 电流增益初始化
    Write8302(fd,GSIB,(uchar*)&rn8302_para.KI_B,2);
    Write8302(fd,GSIC,(uchar*)&rn8302_para.KI_C,2);
    if ((rn8302_para.KX_A[0]==0)&&(rn8302_para.KX_A[2]==0))
    {
        ltmp=0;
        Write8302(fd,PRTH1L,(uchar*)&ltmp, 2);
        /*功率相位校正参数，如果是单步校准方式，则只写入标称电流时的相位系数*/
        Write8302(fd,PA_PHSL, (uchar*)&rn8302_para.KX_A[1], 2);
        Write8302(fd,PB_PHSL, (uchar*)&rn8302_para.KX_B[1], 2);
        Write8302(fd,PC_PHSL, (uchar*)&rn8302_para.KX_C[1], 2);
        Write8302(fd,QA_PHSL, (uchar*)&rn8302_para.KX_A[1], 2);
        Write8302(fd,QB_PHSL, (uchar*)&rn8302_para.KX_B[1], 2);
        Write8302(fd,QC_PHSL, (uchar*)&rn8302_para.KX_C[1], 2);

    }
    else
    {
        ltmp = 0xB0;
        Write8302(fd,PRTH1L, (uchar*)&ltmp, 2);
        ltmp = 0xC0;
        Write8302(fd,PRTH1H, (uchar*)&ltmp, 2);
        ltmp = 0xE30;
        Write8302(fd,PRTH2L, (uchar*)&ltmp, 2);
        ltmp = 0xE5F;
        Write8302(fd,PRTH2H, (uchar*)&ltmp, 2);

        /*功率相位校正参数，如果是三步校准方式，则分段写入三档电流时的相位系数*/
        Write8302(fd,PA_PHSL, (uchar*)&rn8302_para.KX_A[0], 2);
        Write8302(fd,PB_PHSL, (uchar*)&rn8302_para.KX_B[0], 2);
        Write8302(fd,PC_PHSL, (uchar*)&rn8302_para.KX_C[0], 2);
        Write8302(fd,QA_PHSL, (uchar*)&rn8302_para.KX_A[0], 2);
        Write8302(fd,QB_PHSL, (uchar*)&rn8302_para.KX_B[0], 2);
        Write8302(fd,QC_PHSL, (uchar*)&rn8302_para.KX_C[0], 2);

        ltmp = rn8302_para.KX_A[1];
        Write8302(fd,PA_PHSM, (uchar*)&ltmp, 3);
        Write8302(fd,QA_PHSM, (uchar*)&ltmp, 3);
        ltmp = rn8302_para.KX_B[1];
        Write8302(fd,PB_PHSM, (uchar*)&ltmp, 3);
        Write8302(fd,QB_PHSM, (uchar*)&ltmp, 3);
        ltmp = rn8302_para.KX_C[1];
        Write8302(fd,PC_PHSM, (uchar*)&ltmp, 3);
        Write8302(fd,QC_PHSM, (uchar*)&ltmp, 3);

        ltmp = rn8302_para.KX_A[2];
        Write8302(fd,PA_PHSH, (uchar*)&ltmp, 3);
        Write8302(fd,QA_PHSH, (uchar*)&ltmp, 3);
        ltmp = rn8302_para.KX_B[2];
        Write8302(fd,PB_PHSH, (uchar*)&ltmp, 3);
        Write8302(fd,QB_PHSH, (uchar*)&ltmp, 3);
        ltmp = rn8302_para.KX_C[2];
        Write8302(fd,PC_PHSH, (uchar*)&ltmp, 3);
        Write8302(fd,QC_PHSH, (uchar*)&ltmp, 3);

    }
    ltmp = 0xDC;
    Write8302(fd,WREN, (uchar*)&ltmp, 1);

}


bool Check8302WavebufRdy(int fd)
{
    uchar* tmp=Read8302(fd,WSAVECON,1);
    if((tmp[0]&0x30)==0)
        return true;
    return false;
}

#ifdef DEBUG
uchar* ReadBurst8302(int fd,ushort wReg,int DataNum)
{
    uchar Burst_LEN,DatLen,ADDR,CMD,CS;
    if(DataNum == 1)
        Burst_LEN = 0;
    else if(DataNum == 4)
        Burst_LEN = 1;
    else if(DataNum == 8)
        Burst_LEN = 2;
    else if(DataNum == 16)
        Burst_LEN = 3;
    else
        return NULL;
    DatLen = DataNum*3;
    ADDR =(wReg & 0x00ff);
    CMD = (wReg >>4)& 0x70;
    CMD |= (Burst_LEN << 2);
    ushort Addr=(CMD<<4)|ADDR;
    uchar* result=Read8302(fd,Addr,DatLen);
    return result;
}
#endif

#ifdef DEBUG
double ReadWaveOne(int fd,ushort Addr)
{
    uchar* result=ReadBurst8302(fd,Addr,1);
    int ltmp=(result[0]*65536)+(result[1]*256)+result[2];
    if (ltmp &0x800000) 
    {
        ltmp = (0xffffff - ltmp) + 1;
        return (double)(ltmp/-83.88608);
    }
    return (double)(ltmp/83.88608);
}

double Read8302Freq(int fd)
{
    uchar* result=Read8302(fd,UFreq,3);
    if (result != NULL)
    {
        uint ltmp=(result[0]*65536)+(result[1]*256)+result[2];
        return (double)(8192000*8.0/ltmp*100);
    }
    else
        return 0;
}
#endif

double Read8302U(int fd,ushort Addr)
{
    uchar* result=Read8302(fd,Addr,4);
    if (result !=NULL)
    {
        uint ltmp=result[0]*16777216+result[1]*65536+result[2]*256+result[3];
        return (ushort)(ltmp/20000);
    }
    else
    {
        return 0;
    }
}

double Read8302I(int fd,ushort Addr)
{
    uchar* result=Read8302(fd,Addr,4);
    if (result !=NULL)
    {
        uint ltmp=(result[0]*16777216)+(result[1]*65536)+(result[2]*256)+result[3];
        return (double)(ltmp/5000);
    }
    else
        return 0;
}

#ifdef DEBUG
double Read8302PQS(int fd,ushort Addr)
{
    uchar* result=Read8302(fd,Addr,4);
    if (result !=NULL)
    {
        uint ltmp=(result[0]*16777216)+(result[1]*65536)+(result[2]*256)+result[3];
        if ((result[0]&0x80)==0)
            return (double)(ltmp/1192);
        else
        {
            ltmp=ltmp|0x80000000;
            ltmp=(0xffffffff-ltmp)+1;     //若为负数，则取绝对值
            return -((double)(ltmp/1192));
        }
    }
    else
        return 0;
}

double Read8302PF(int fd,ushort Addr)
{
    double retval=0;
    uchar* result=Read8302(fd,Addr,3);
    if (result!=NULL)
    {
        int ltmp=(result[0]*65536)+(result[1]*256)+result[2];
        retval=(ltmp&0x7fffff)/8388.608;
        if((ltmp & 0x800000)!=0)   //bit23表示符号位
        {
            ltmp=(ltmp^0xffffff)+1;
            retval =ltmp/-8388.608;
        }
    }
    return retval;
}

double Read8302Angle(int fd,ushort Addr)
{
    uchar* result=Read8302(fd,Addr,3);
    if (result!=NULL)
    {
        double ltmp=(result[0]*65536)+(result[1]*256)+result[2];
        return (ltmp/16777216.0*3600.0);
    }
    return 0;
}

void Read8302Energy(int fd)
{
    uchar i;
    ushort *p_pluse;
    ushort en_pulse_q, en_pulse_nq;
    uint ltmp;
    uchar* result;
    p_pluse = &AC_Sample.En_Pulse.E_PA;
    for(i=0; i<8; i++)
    {
        ltmp = 0;
        result=Read8302(fd,PosEPA+i,3);
        if(result!=NULL)
        {
            ltmp=(result[0]*65536)+(result[1]*256)+result[2];
            p_pluse[i] += ltmp;
        }
    }
    p_pluse = &AC_Sample.En_Pulse.E_QA;
    for(i=0; i<8; i++)
    {
        ltmp = 0;
        result=Read8302(fd,PosEQA+i,3);
        if(result!=NULL)
        {
            ltmp=(result[0]*65536)+(result[1]*256)+result[2];
            p_pluse[i] += ltmp;
            if(i == 3)              //记录无功脉冲，为了累加四象限电能
                en_pulse_q = ltmp;
            if(i == 7)
                en_pulse_nq = ltmp;
        }
    }

    //自己虚拟出四象限电能脉冲个数值
    if(PointData.P>0 && PointData.Q>0)
        AC_Sample.En_Pulse.E_Q1 += en_pulse_q;
    if(PointData.P>0 && PointData.Q<0)
        AC_Sample.En_Pulse.E_Q4 += en_pulse_nq;
    if(PointData.P<0 && PointData.Q>0)
        AC_Sample.En_Pulse.E_Q2 += en_pulse_q;
    if(PointData.P<0 && PointData.Q<0)
        AC_Sample.En_Pulse.E_Q3 += en_pulse_nq;
}

#endif
void ReadRn8302All(int fd)
{
    static uchar DataValid = 0;
    long tmp;
    if(!Check8302Exist(fd))
    {
        printf("\nRn8302 doesn't exist!");
        return;
    }
    PointData.Ua = Read8302U(fd,r_Ua);
    PointData.Ub = Read8302U(fd,r_Ub);
    PointData.Uc = Read8302U(fd,r_Uc);

    PointData.Ia = Read8302I(fd,r_Ia);
    PointData.Ib = Read8302I(fd,r_Ib);
    PointData.Ic = Read8302I(fd,r_Ic);
    PointData.Io = Read8302I(fd,r_Io);

#ifdef DEBUG
    PointData.Pa = Read8302PQS(fd,r_Pa)/10;
    PointData.Pb = Read8302PQS(fd,r_Pb)/10;
    PointData.Pc = Read8302PQS(fd,r_Pc)/10;
    PointData.P =  Read8302PQS(fd,r_Pt)/10;

    PointData.Qa = Read8302PQS(fd,r_Qa)/10;
    PointData.Qb = Read8302PQS(fd,r_Qb)/10;
    PointData.Qc = Read8302PQS(fd,r_Qc)/10;
    PointData.Q =  Read8302PQS(fd,r_Qt)/10;

    PointData.Sa = Read8302PQS(fd,r_Sa)/10;
    PointData.Sb = Read8302PQS(fd,r_Sb)/10;
    PointData.Sc = Read8302PQS(fd,r_Sc)/10;

    PointData.UbAngle = Read8302Angle(fd,YUB);
    PointData.UcAngle = Read8302Angle(fd,YUC);

    PointData.IaAngle = Read8302Angle(fd,YIA);
    PointData.IbAngle = Read8302Angle(fd,YIB);
    PointData.IcAngle = Read8302Angle(fd,YIC);

    AC_Sample.Frequency = Read8302Freq(fd);
    GetHarmonicSample(fd);          //获取谐波
    Read8302Energy(fd);             //读电能量
    ReformAcData();               //过滤
    if(PointData.Ia > 10)
        PointData.CosA = (ushort)(PointData.Pa*1000.0/PointData.Sa);//read_8302_PF(r_PfA);
    else
        PointData.CosA = 1000;

    if(PointData.Ib > 10)
        PointData.CosB = (ushort)(PointData.Pb*1000.0/PointData.Sb);//read_8302_PF(r_PfB);
    else
        PointData.CosB = 1000;

    if(PointData.Ic > 10)
        PointData.CosC = (ushort)(PointData.Pc*1000.0/PointData.Sc);//read_8302_PF(r_PfC);
    else
        PointData.CosC = 1000;

    if(PointData.Sz > 10)
        PointData.Cos = (ushort)(PointData.P*1000.0/PointData.Sz);//read_8302_PF(r_PfT);
    else
        PointData.Cos = 1000;
#endif
}

#ifdef DEBUG
void GetHarmonicSample(int fd)
{
    int ch=0;
    uchar i;
    ushort regaddr;
    double SampleBuf[SAMPLECHANNEL][SAMPLENUM];
    if(Check8302WavebufRdy(fd) == false)
        return;
    //read all channel's wave data
    for(ch=0;ch<SAMPLECHANNEL;ch++)
    {
        regaddr=0x200+ch*0x80;
        for(i=0; i<SAMPLENUM; i++)
        {
            SampleBuf[ch][i] = (double)ReadWaveOne(fd,regaddr+i+1);
        }
    }
    i = 0xE5;
    Write8302(fd,WREN,(uchar*)&i, 1);
    i = WAVEREG_CONF;
    Write8302(fd,WSAVECON,(uchar*)&i, 1);
    i = 0xDC;
    Write8302(fd,WREN,(uchar*)&i, 1);

    fftw_complex* fft_0;
    fftw_plan p;
    int N=SAMPLENUM;
    fft_0=(fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* fft_1=(fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* fft_2=(fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* fft_3=(fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* fft_4=(fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* fft_5=(fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    p=fftw_plan_dft_r2c_1d(N,&SampleBuf[0],fft_0,FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);
    p=fftw_plan_dft_r2c_1d(N,&SampleBuf[1],fft_1,FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);
    p=fftw_plan_dft_r2c_1d(N,&SampleBuf[2],fft_2,FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);
    p=fftw_plan_dft_r2c_1d(N,&SampleBuf[3],fft_3,FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);
    p=fftw_plan_dft_r2c_1d(N,&SampleBuf[4],fft_4,FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);
    p=fftw_plan_dft_r2c_1d(N,&SampleBuf[5],fft_5,FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);

    printf("\nfft_0:\n{");
    for(int jj=0;jj<64;jj++)
    {
        printf("[%f,%f] ",fft_0[jj][0],fft_0[jj][1]);
    }
    printf("}\n");
    double result_0[SAMPLENUM],result_1[SAMPLENUM],result_2[SAMPLENUM],result_3[SAMPLENUM],result_4[SAMPLENUM],result_5[SAMPLENUM];
    for(i=0;i<SAMPLENUM;i++)
    {
        result_0[i]=AbsC(fft_0[i][0],fft_0[i][1]);
        if(result_0[i]<2000)   result_0[i]=0;
        result_1[i]=AbsC(fft_1[i][0],fft_1[i][1]);
        if(result_1[i]<2000)   result_1[i]=0;
        result_2[i]=AbsC(fft_2[i][0],fft_2[i][1]);
        if(result_2[i]<2000)   result_2[i]=0;
        result_3[i]=AbsC(fft_3[i][0],fft_3[i][1]);
        if(result_3[i]<2000)   result_3[i]=0;
        result_4[i]=AbsC(fft_4[i][0],fft_4[i][1]);
        if(result_4[i]<2000)   result_4[i]=0;
        result_5[i]=AbsC(fft_5[i][0],fft_5[i][1]);
        if(result_5[i]<2000)   result_5[i]=0;
    }
    printf("\nresult_0:\n{");
    for(int jj=0;jj<64;jj++)
    {
        printf("[%f] ",result_0[jj]);
    }
    printf("}\n");


    for(i=0;i<HARMONIC_NUM;i++)
    {
        AC_Sample.Harmonic[0].UHarmonic[i]=(result_0[1]>0)?(result_0[i+1]/result_0[1]):0;
        AC_Sample.Harmonic[0].IHarmonic[i]=(result_3[1]>0)?(result_3[i+1]/result_3[1]):0;
        AC_Sample.Harmonic[1].UHarmonic[i]=(result_1[1]>0)?(result_1[i+1]/result_1[1]):0;
        AC_Sample.Harmonic[1].IHarmonic[i]=(result_4[1]>0)?(result_4[i+1]/result_4[1]):0;
        AC_Sample.Harmonic[2].UHarmonic[i]=(result_2[1]>0)?(result_2[i+1]/result_2[1]):0;
        AC_Sample.Harmonic[2].IHarmonic[i]=(result_5[1]>0)?(result_5[i+1]/result_5[1]):0;
    }
    AC_Sample.Harmonic[0].UHarmonic[0]=result_0[1];
    AC_Sample.Harmonic[0].IHarmonic[0]=result_3[1];
    AC_Sample.Harmonic[1].UHarmonic[0]=result_1[1];
    AC_Sample.Harmonic[1].IHarmonic[0]=result_4[1];
    AC_Sample.Harmonic[2].UHarmonic[0]=result_2[1];
    AC_Sample.Harmonic[2].IHarmonic[0]=result_5[1];
    fftw_free(fft_0);
    fftw_free(fft_1);
    fftw_free(fft_2);
    fftw_free(fft_3);
    fftw_free(fft_4);
    fftw_free(fft_5);
}


double AbsC(double a,double b)
{
    return sqrt(a*a+b*b);
}

void ReformAcData()
{
    if(PointData.Ua <= 80) PointData.Ua = 0;
    if(PointData.Ub <= 80) PointData.Ub = 0;
    if(PointData.Uc <= 80) PointData.Uc = 0;
    if(PointData.Ia <= 1) PointData.Ia = 0;
    if(PointData.Ib <= 1) PointData.Ib = 0;
    if(PointData.Ic <= 1) PointData.Ic = 0;
    if(PointData.Pa <= 5 && PointData.Pa>=-5) PointData.Pa = 0;
    if(PointData.Pb <= 5 && PointData.Pb>=-5) PointData.Pb = 0;
    if(PointData.Pc <= 5 && PointData.Pc>=-5) PointData.Pc = 0;
    if(PointData.Qa <= 5 && PointData.Qa>=-5) PointData.Qa = 0;
    if(PointData.Qb <= 5 && PointData.Qb>=-5) PointData.Qb = 0;
    if(PointData.Qc <= 5 && PointData.Qc>=-5) PointData.Qc = 0;
}

#endif

bool InitRn8302(int fd)
{
    WriteRn8302Gain(fd);
    printf("\nstart to init rn8302\n");
    uint ltmp;
    ltmp=0xe5;
    Write8302(fd,0x0180,(uchar*)&ltmp,1);    //开启写使能
    ltmp=0xa2;
    Write8302(fd,0x0181,(uchar*)&ltmp,1);  //切换到EMM模式
    ltmp=0xfa;
    Write8302(fd,0x0182,(uchar*)&ltmp,1);   //复位
    sleep(0.1);
    ltmp=0xe5;
    Write8302(fd,0x0180,(uchar*)&ltmp,1);   //开启写使能
    //    Write8302(0x0181,[0xa2],1)   #切换到EMM模式

    // 读spi芯片ID，判断是否存在
    printf("开始读SPI芯片ID\n");
    if (Check8302Exist(fd))
        printf("RN8302 exists!\n");
    else
    {
        printf("RN8302 doesn't exist!\n");
        return false;
    }
    ltmp=0x7f;
    Write8302(fd,0x0191,(uchar*)&ltmp,2);   //AUTODC_EN,直流OFFSET自动校正
    ltmp=7994;
    Write8302(fd,HFCONST1,(uchar*)&ltmp,2);  // #高频脉冲计数寄存器
    Write8302(fd,HFCONST2,(uchar*)&ltmp,2);
    ltmp=0x10;
    Write8302(fd,WSAVECON,(uchar*)&ltmp,1);  //  #清空采样数据缓存区
    while (Check8302WavebufRdy(fd)==false)    //  #if wavebuf busy, we wait here
        continue;
    ltmp=0xe0;
    Write8302(fd,WSAVECON,(uchar*)&ltmp,1);  // #同步采样，64个点
    ltmp=0x777777;
    Write8302(fd,EMUCON,(uchar*)&ltmp,3);   // #计量控制位，通道使能
    ltmp=0x0015;
    Write8302(fd,ADCCFG,(uchar*)&ltmp,2);  //  #电压通道2倍增益
    //#过零阈值寄存器,,当电流小于0.002A时，不在显示相角
    ltmp=0x0002;
    Write8302(fd,ZXOT,(uchar*)&ltmp,2);
    ltmp=0xdc;
    Write8302(fd,WREN,(uchar*)&ltmp,1);
    return true;

}

#define K 1
#define F 0.5
#define S 1
#define T 2.5

void *func(void *arg)
{
	FILE *pfd = (FILE *)arg;
	time_t timep;
	while(1)
	{
		sleep(30);
		time(&timep);
		fprintf(pfd, "%sspi  is working.\n", asctime(gmtime(&timep)));
		fflush(pfd);
	}
	pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    int ret = 0;  
    int fd;  
    FILE *pfd;
    time_t timep;
    float a, b, c;
    char *portname = "/root/log/spi.log";
    pthread_t pthid;

    pfd = fopen(portname, "a");
    //setbuf(pfd, NULL);
    if (pfd == NULL)
    {
        printf("can't open spi.log");
        return 0;
    }

    time(&timep);
    fprintf(pfd, "%sspi start work.\n", asctime(gmtime(&timep)));
    fflush(pfd);

    pthread_create(&pthid, NULL, func, (void *)pfd);

    parse_opts(argc, argv);  

    fd = open(device, O_RDWR);  
    if (fd < 0)  
        pabort("can't open device");  

    /* 
     * spi mode 
     */  
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);  
    if (ret == -1)  
        pabort("can't set spi mode");  

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);  
    if (ret == -1)  
        pabort("can't get spi mode");  

    /* 
     * bits per word 
     */  
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);  
    if (ret == -1)  
        pabort("can't set bits per word");  

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);  
    if (ret == -1)  
        pabort("can't get bits per word");  

    /* 
     * max speed hz 
     */  
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);  
    if (ret == -1)  
        pabort("can't set max speed hz");  

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);  
    if (ret == -1)  
        pabort("can't get max speed hz");  
    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

    InitRn8302(fd);
    while(1)
    {
        while(Check8302WavebufRdy(fd)==false)
        {
            printf("read_8302 0x0163:寄存器读无效\n");
            sleep(2);
        }
        printf("开始读取各项值\n");

        time(&timep);
        ReadRn8302All(fd);
        printf("ia = %f; ib = %f; ic = %f\n", PointData.Ia, PointData.Ib, PointData.Ic);
        a = PointData.Ia * K;
        b = PointData.Ib * K;
        c = PointData.Ic * K;

        if( (fabs(a-F) > 0.1) && (fabs(a-S) > 0.1) && (fabs(a-T) > 0.1) )
        {
            fprintf(pfd, "%sIa = %f, Ib = %f, Ic = %f\n", asctime(gmtime(&timep)), PointData.Ia,  PointData.Ib, PointData.Ic);
            fflush(pfd);
        }
        else if((fabs(b-F) > 0.1) && (fabs(b-S) > 0.1) && (fabs(b-T) > 0.1))
        {
            fprintf(pfd, "%sIa = %f, Ib = %f, Ic = %f\n", asctime(gmtime(&timep)), PointData.Ia,  PointData.Ib, PointData.Ic);
            fflush(pfd); 
        }
        else if((fabs(c-F) > 0.1) && (fabs(c-S) > 0.1) && (fabs(c-T) > 0.1))
        {
            fprintf(pfd, "%sIa = %f, Ib = %f, Ic = %f\n", asctime(gmtime(&timep)), PointData.Ia,  PointData.Ib, PointData.Ic);
            fflush(pfd); 
        }

        sleep(2);
    }
    return 0;
}
