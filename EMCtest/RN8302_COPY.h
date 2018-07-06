#ifndef _RN8302_H
#define _RN8302_H
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define  P_CONST      6400
#define  Q_CONST      6400
#define  HARMONIC_NUM 19
#define uchar unsigned char
#define ushort unsigned short
#define uint unsigned int
typedef struct
{
    ushort E_PA;    //读取出来的脉冲个数-A相正向有功
    ushort E_PB;
    ushort E_PC;
    ushort E_P;
    ushort NE_PA;    //读取出来的脉冲个数-A相反向有功
    ushort NE_PB;
    ushort NE_PC;
    ushort NE_P;
    ushort E_QA;    //读取出来的脉冲个数-A相正向无功
    ushort E_QB;
    ushort E_QC;
    ushort E_Q;
    ushort NE_QA;    //读取出来的脉冲个数-A相反向无功
    ushort NE_QB;
    ushort NE_QC;
    ushort NE_Q;
    ushort E_Q1;     //四象限电能脉冲
    ushort E_Q2;
    ushort E_Q3;
    ushort E_Q4;
}ENERGY_PULSE;

typedef struct
{
    double UHarmonic[HARMONIC_NUM+1];
    double IHarmonic[HARMONIC_NUM+1];
}PHASE_HARMONIC;

/**
  *该结构体为工作状态及数据结构
  */
typedef struct
{
    short Temperature;    ///<温度采样原始ADC值
    double Frequency;      ///<电网频率

    ENERGY_PULSE   En_Pulse;
    PHASE_HARMONIC Harmonic[3]; //三相电压电流2-19次谐波有效值
}SAMPLE_ST;
SAMPLE_ST AC_Sample;

typedef struct
{
   double Ua;
   double  Ub;
   double  Uc;
   double Ia;
   double  Ib;
   double  Ic;
   double  Io;
   double  Pa;
   double  Pb;
   double  Pc;
   double  P;
   double  Qa;
   double  Qb;
   double  Qc;
   double  Q;
   double  Sa;
   double  Sb;
   double  Sc;
   double  Sz;
   double  UbAngle;
   double  UcAngle;
   double  IaAngle;
   double  IbAngle;
   double  IcAngle;
   double  CosA;
   double  CosB;
   double  CosC;
   double  Cos;
}POINT_DATA;
POINT_DATA PointData;

#if ((AC_SAMPLE_TYPE == AC_3P3W) || (AC_SAMPLE_TYPE == AC_3P4W))

#define  FFT_num  64
#define  _PI      3.14159265
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define MALLOC_SIZE(a) (malloc_usable_size(a) / sizeof(a[0])/2)

#define  RN8302_CS_1    GPIO_SetBits(R8302_CS_GPIO_PORT,R8302_CS_PIN)
#define  RN8302_CS_0    GPIO_ResetBits(R8302_CS_GPIO_PORT,R8302_CS_PIN)
#define  RN8302_RST_1   GPIO_SetBits(R8302_RST_GPIO_PORT,R8302_RST_PIN)
#define  RN8302_RST_0   GPIO_ResetBits(R8302_RST_GPIO_PORT,R8302_RST_PIN)
#define  RN8302_SI_1    GPIO_SetBits(SPI_MO_GPIO_PORT,SPI_MO_PIN)
#define  RN8302_SI_0    GPIO_ResetBits(SPI_MO_GPIO_PORT,SPI_MO_PIN)
#define  RN8302_SO      GPIO_ReadInputDataBit(SPI_MI_GPIO_PORT,SPI_MI_PIN)
#define  RN8302_SCK_1   GPIO_SetBits(SPI_SCK_GPIO_PORT,SPI_SCK_PIN)
#define  RN8302_SCK_0   GPIO_ResetBits(SPI_SCK_GPIO_PORT,SPI_SCK_PIN)

#define  RN8302_IRQ     GPIOD->IDR  & GPIO_Pin_8

#define  WREN_8302      write_8302(WREN,0xE5,1)//写使能命令
#define  WRDIS_8302     write_8302(WREN,0xDC,1)//写保护命令

typedef struct
{
    ushort KU_A;
    ushort KU_B;
    ushort KU_C;
    ushort KI_A;
    ushort KI_B;
    ushort KI_C;
    ushort KX_A[3];
    ushort KX_B[3];
    ushort KX_C[3];
    ushort Checksum;
}RN8302_PARA_STRUCT;             //校准参数
RN8302_PARA_STRUCT rn8302_para;

/* external functions------------------------------------------------------------------*/
//ushort Read8302U(int fd,ushort reg_addr);
//ushort Read8302I(int fd,ushort reg_addr);
//int Read8302PQS(int fd,ushort reg_addr);
void WriteRn8302Gain(int fd);
bool InitRn8302(int fd);
bool Check8302Exist(int fd);
void WriteRn8302Gain(int fd);
double AbsC(double a,double b);

static const char *device = "/dev/spidev1.0";
static uchar mode=1;
static uchar bits = 8;
static uint speed = 50000;
static ushort delay;
uchar* Read8302(int fd,ushort Addr, int datalen);
void Write8302(int fd,ushort Addr,uchar* pData,int DataLen);
void parse_opts(int argc, char *argv[]);
void print_usage(const char *prog);
void pabort(const char *s);
uchar* ReadBurst8302(int fd,ushort wReg,int DataNum);
double ReadWaveOne(int fd,ushort Addr);
double Read8302Freq(int fd);
double Read8302U(int fd,ushort Addr);
double Read8302I(int fd,ushort Addr);
double Read8302PQS(int fd,ushort Addr);
double Read8302PF(int fd,ushort Addr);
double Read8302Angle(int fd,ushort Addr);
void Read8302Energy(int fd);
void ReadRn8302All(int fd);
void GetHarmonicSample(int fd);
void ReformAcData();
bool Check8302WavebufRdy(int fd);

//extern void GetKVKI();
//extern void SaveKVKI();
//extern bool ReadKVKI();
//extern void SetDefaultKVKI();

#define WAVEREG_CONF      ((1<<7)|(1<<6)|(0x02<<4)) //波形采样配置
#define ENERGECONST   6400      //电表常数
#define SAMPLECHANNEL 6         //采样通道
#define SAMPLENUM 64            //采样点数
#define UA_CHANNEL 0
#define UB_CHANNEL 1
#define UC_CHANNEL 2
#define IA_CHANNEL 3
#define IB_CHANNEL 4
#define IC_CHANNEL 5

/* RN8302 run param register addr ------------------------------------------------------------------*/

#define r_Ua        0x0007
#define r_Ub        0x0008
#define r_Uc        0x0009
#define r_Ia        0x000b
#define r_Ib        0x000c
#define r_Ic        0x000d
#define r_Io        0x000e

#define r_Pa        0x0014   /* A Phase Power */
#define r_Pb        0x0015   /* B Phase Power */
#define r_Pc        0x0016   /* C Phase Power */
#define r_Pt        0x0017   /* 3 Phase Power */
#define r_Qa        0x0018   /* A Phase Q Power */
#define r_Qb        0x0019   /* B Phase Q Power */
#define r_Qc        0x001a   /* C Phase Q Power */
#define r_Qt        0x001b   /* 3 Phase Q Power */
#define r_Sa        0x001c   /* A Phase S Power */
#define r_Sb        0x001d   /* B Phase S Power */
#define r_Sc        0x001e   /* C Phase S Power */
#define r_St        0x001f   /* 3 Phase S Power */

#define r_PfA       0x0020
#define r_PfB       0x0021
#define r_PfC       0x0022
#define r_PfT       0x0023

#define EPA         0x0030
#define EPB         0x0031
#define EPC         0x0032
#define EPT         0x0033
#define PosEPA      0x0034
#define PosEPB      0x0035
#define PosEPC      0x0036
#define PosEPT      0x0037
#define NegEPA      0x0038
#define NegEPB      0x0039
#define NegEPC      0x003A
#define NegEPT      0x003B

#define EQA         0x003C
#define EQB         0x003D
#define EQC         0x003E
#define EQT         0x003F
#define PosEQA      0x0040
#define PosEQB      0x0041
#define PosEQC      0x0042
#define PosEQT      0x0043
#define NegEQA      0x0044
#define NegEQB      0x0045
#define NegEQC      0x0046
#define NegEQT      0x0047

#define ESA         0x0048
#define ESB         0x0049
#define ESC         0x004A
#define EST         0x004B

#define YUA         0x0050   /* UA Phase Angle */
#define YUB         0x0051   /* Ub Phase Angle */
#define YUC         0x0052   /* UC Phase Angle */
#define YIA         0x0053   /* IA Phase Angle */
#define YIB         0x0054   /* IB Phase Angle */
#define YIC         0x0055   /* IC Phase Angle */
#define YIN         0x0056   /* IO Phase Angle */

#define UFreq       0x0057

#define FEPA        0x007A
#define FEPB        0x007B
#define FEPC        0x007C
#define FEPT        0x007D
#define PosFEPA     0x007E
#define PosFEPB     0x007F
#define PosFEPC     0x0080
#define PosFEPT     0x0081
#define NegFEPA     0x0082
#define NegFEPB     0x0083
#define NegFEPC     0x0084
#define NegFEPT     0x0085

#define FEQA        0x0086
#define FEQB        0x0087
#define FEQC        0x0088
#define FEQT        0x0089
#define PosFEQA     0x008A
#define PosFEQB     0x008B
#define PosFEQC     0x008C
#define PosFEQT     0x008D
#define NegFEQA     0x008E
#define NegFEQB     0x008F
#define NegFEQC     0x0090
#define NegFEQT     0x0091

#define FESA        0x0092
#define FESB        0x0093
#define FESC        0x0094
#define FEST        0x0095


/* RN8302 config and status register addr ------------------------------------------------------------------*/
/****************************************
参照RN8302的手册，在读寄存器和写寄存器时，
配置和状态寄存器地址空间的高3位地址，即Bank地址
为二进制001，故以下的地址定义中，BANK地址全为1
*****************************************/
#define HFCONST1    0x0100      //高频脉冲计数寄存器1
#define HFCONST2    0x0101      //高频脉冲计数寄存器2
#define IStart_PS   0x0102
#define ZXOT        0x0105
#define PRTH1L      0x0106
#define PRTH1H      0x0107
#define PRTH2L      0x0108
#define PRTH2H      0x0109
#define PHSUA       0x010C
#define PHSUB       0x010D
#define PHSUC       0x010E
#define PHSIA       0x010F
#define PHSIB       0x0110
#define PHSIC       0x0111
#define GSUA        0x0113      //采样通道Ua通道增益
#define GSUB        0x0114      //采样通道Ub通道增益
#define GSUC        0x0115      //采样通道Uc通道增益
#define GSIA        0x0116      //采样通道Ia通道增益
#define GSIB        0x0117      //采样通道Ib通道增益
#define GSIC        0x0118      //采样通道Ic通道增益
#define GSIN        0x0119      //采样通道In通道增益

#define PA_PHSL     0x0131
#define PB_PHSL     0x0132
#define PC_PHSL     0x0133
#define QA_PHSL     0x0134
#define QB_PHSL     0x0135
#define QC_PHSL     0x0136
#define PA_PHSM     0x01B0
#define PA_PHSH     0x01B1
#define PB_PHSM     0x01B2
#define PB_PHSH     0x01B3
#define PC_PHSM     0x01B4
#define PC_PHSH     0x01B5
#define QA_PHSM     0x01B6
#define QA_PHSH     0x01B7
#define QB_PHSM     0x01B8
#define QB_PHSH     0x01B9
#define QC_PHSM     0x01BA
#define QC_PHSH     0x01BB

#define PA_OS       0x0137
#define PB_OS       0x0138
#define PC_OS       0x0139
#define CFCFG       0x0160
#define EMUCON      0x0162
#define WSAVECON    0x0163
#define PQSign      0x0166
#define CheckSum1   0x016A
#define WREN        0x0180
#define WMSW        0x0181
#define SOFTRST     0x0182
#define ADCCFG      0x0183
#define MODSEL      0x0186
#define LRBufAddr   0x018E
#define DeviceID    0x018F
#endif
#endif

