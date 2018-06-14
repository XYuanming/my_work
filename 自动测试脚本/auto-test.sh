#!/bin/bash
#铁电测试
echo "铁电测试"
i2cset -f -y 2 0x50 0x21 0x4d
value=`i2cget -f -y 2 0x50 0x21`
if [ "$value" != "0x4d" ] > /dev/null 2>&1; then
	echo "i2cset ERR"
else
	echo "铁电 OK"
fi

#USB接口测试
echo "USB接口测试"
file="/mnt"
mount /dev/sda1 /mnt > /dev/null 2>&1
if [ "`ls $file`" == "" ]
then
	echo "USB not recognized"
else
	echo "USB recognized"
fi

#USB扩�UART测试
./Auto-test-scr/auto_ACM0_2

#IIC扩UART测试
./Auto-test-scr/iic-auto


#自带GPIO测试
echo "自带GPIO测试"
echo 8 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio8/direction
echo 0 > /sys/class/gpio/gpio8/value
if [ "`cat /sys/class/gpio/gpio8/direction`" != "out" ]
then
	echo "gpio8 set out fail"
elif [ "`cat /sys/class/gpio/gpio8/value`" != "0" ]
then
	echo "set gpio8 value=0 fail"
fi
echo in > /sys/class/gpio/gpio8/direction
#echo 1 > /sys/class/gpio/gpio8/value
if [ "`cat /sys/class/gpio/gpio8/direction`" != "in" ]
then
	echo "gpio8 set in fail"
#elif [ "`cat /sys/class/gpio/gpio8/value`" != "" ]
#then
#	echo "set gpio8 value=1 fail"
else
	echo "set gpio ok"
fi
echo 8 > /sys/class/gpio/unexport

#IICGPIO
echo "IICGPIO测试"
if [ "`sh ./test-xdc/IIC2GPIO/iicexport.sh`" != "" ]
then
	echo "do iicexport fail"
elif [ "`sh ./test-xdc/IIC2GPIO/iicvalue0.sh`" != "" ]
then
	echo "do iicvalue0.sh fail"
elif [ "`sh ./test-xdc/IIC2GPIO/iicvalue1.sh`" != "" ]
then
	echo "do iicvalue1.sh fail"
elif [ "`sh ./test-xdc/IIC2GPIO/iicunexport.sh`" ]
then
	echo "do iicunexport.sh fail"
else
	echo "IICGPIO OK"
fi

#wireless test
#echo "wireless test"
#./Auto-test-scr/wireless

#载波测试
echo "载波测试"
./Auto-test-scr/carrierDetect



#watchdog test
#echo "watchdog test"
#./wdt
