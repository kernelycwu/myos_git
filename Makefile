TOP := .
SRC := 

CC := gcc  
LD := ld

ARCH := amd64
#VMX  := host
VMX  := guest
LDFLAGS := 
CFLAGS  := -g -nostdlib -fno-builtin -O0  -D$(ARCH)  -D$(VMX)
CFLAGS += -I include

BOOTCFLAGS := -m32 -D$(ARCH) -fno-omit-frame-pointer -mno-red-zone -fno-stack-protector
BOOTLDFLAGS := -m elf_i386 

#Architecture configurations
include $(TOP)/config/$(ARCH).mk

#Make the source code (.c .S) files variable XXX_SRC
include $(TOP)/kernel/module.mk
include $(TOP)/user/module.mk
include $(TOP)/boot/module.mk
include $(TOP)/lib/module.mk

# Maket the object code (.o) files ariable XXX_OBJ
BOOT_OBJ := $(patsubst %.S, %.S.o, \
        $(filter %.S, $(BOOT_SRC))) \
        $(patsubst %.c, %.c.o, \
        $(filter %.c, $(BOOT_SRC)))
	
KERN_OBJ := $(patsubst %.S, %.o, \
        $(filter %.S, $(KERN_SRC))) \
        $(patsubst %.c, %.o, \
        $(filter %.c, $(KERN_SRC)))

USER_OBJ :=  $(patsubst %.S, %.o, \
	$(filter %.S, $(USER_SRC))) \
	$(patsubst %.c, %.o, \
	$(filter %.c, $(USER_SRC)))

LIB_OBJ :=  $(patsubst %.S, %.o, \
	$(filter %.S, $(LIB_SRC))) \
	$(patsubst %.c, %.o, \
	$(filter %.c, $(LIB_SRC)))

#Maket the user binary code  files ariable XXX_BIN
#USER_BIN := hello 
USER_BIN += init vmexec\
	world

KERN_IMG   := kernel.img
DISK_IMG   := xxx.img

# Project Builds  
all: bootloader kernel 
	objcopy -O binary  bootloader bootloader.bin
	dd if=/dev/zero of=$(KERN_IMG) bs=512 count=20480
	gcc scripts/writebootsect.c -o scripts/writebootsect
	./scripts/writebootsect
diskimg:
	dd if=/dev/zero of=$(DISK_IMG) bs=4096  count=32768
	gcc ./scripts/mkxxx.c -o ./scripts/mkxxx -g
	cp ./user/world  world
	./scripts/mkxxx $(DISK_IMG)  world


bootloader: $(BOOT_OBJ)
	$(LD) $(BOOTLDFLAGS)   -Ttext 0x7C00 $(BOOT_OBJ) -o $@

kernel : $(KERN_OBJ) $(USER_BIN) 
	$(LD) $(LDFLAGS)  -T kernel/$(ARCH)/kernel.ld $(KERN_OBJ) -b binary user/init  -o kernel.elf 

$(USER_BIN):%:  $(LIB_OBJ) user/%.o 
	$(LD)  $(LDFLAGS) -T user/$(ARCH)/user.ld -o user/$@  $^

#hello: libmyc.a  $(USER_OBJ) 
#	$(LD)  $(LDFLAGS) -T user/user.ld -o user/$@  -L ./lib -lmyc $(USER_OBJ) 


#Make the dependency (.d) files
-include $(KERN_OBJ:.o=.d)
-include $(USER_OBJ:.o=.d)
-include $(LIB_OBJ:.o=.d)

%.d : %.S
	@set -e ; \
	gcc -MM -MG  $(CFLAGS) $< >$@. ; \
	sed -e "s@^\(.*\)\.o:@$(@D)/\1.d $(@D)/\1.o:@" $@. >$@; \
	rm -f $@.
%.d: %.c
	@set -e; \
	gcc -MM -MG  $(CFLAGS) $< >$@. ; \
        sed -e "s@^\(.*\)\.o:@$(@D)/\1.d $(@D)/\1.o:@" $@. >$@; \
        rm -f $@.

# Special handling  bootloader
%.S.o: %.S
	$(CC) -m32 -Os -c $<  -o $@
%.c.o: %.c
	$(CC) $(BOOTCFLAGS) -m32 -Os -c $<  -o $@

qemu:
	qemu-system-x86_64 -cpu qemu64,+vmx  -drive file=kernel.img,index=0,media=disk,format=raw -drive file=xxx.img,index=1,media=disk,format=raw -serial mon:stdio -gdb tcp::1234
#	qemu-system-x86_64  -drive file=kernel.img,index=0,media=disk,format=raw -drive file=xxx.img,index=1,media=disk,format=raw -serial mon:stdio -gdb tcp::1234 
# 	qemu-x86_64 -S -drive file=kernel.img,index=0,media=disk,format=raw -serial mon:stdio 
	#qemu-system-x86_64 -drive file=kernel.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp::1234 

gdb:
	gdb -n -x .gdbinit
	
clean:
	-rm $(KERN_OBJ) $(USER_OBJ)  $(LIB_OBJ)
	-rm $(KERN_OBJ:.o=.d) $(USER_OBJ:.o=.d)   $(LIB_OBJ:.o=.d) 
	-rm $(KERN_IMG) $(DISK_IMG)
	-rm  user/world boot/*.o 
	-rm bootloader* kernel.elf world

.PHONY: all clean qemu gdb diskimg
