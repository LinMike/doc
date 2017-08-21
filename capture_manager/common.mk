#NFSPATH=/home/yuanf/nfs
NFSPATH=/home/robot/arm_nfs
#CC=arm-none-linux-gnueabi-gcc
CC=arm-linux-gnueabi-gcc
#CC=gcc
define postbuild
    @echo "Copy target $(TARGET) to NFS folder $(NFSPATH)"
    @cp $(TARGET) $(NFSPATH)
endef

define postclean
    @rm -vf $(TARGET) $(NFSPATH)/$(TARGET)
endef
