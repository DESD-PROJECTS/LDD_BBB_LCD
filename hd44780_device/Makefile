TARGET = lcd
obj-m = $(TARGET).o

modules :
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C /home/sunbeam/FINAL_BBB/linux M=`pwd` modules

clean : 
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C /home/sunbeam/FINAL_BBB/linux M=`pwd` clean

copy : 
	scp `pwd`/$(TARGET).ko debian@192.168.6.2:/home/debian/DOLBY/
	
.phony : modules clean copy
