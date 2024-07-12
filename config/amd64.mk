LDFLAGS += -m elf_x86_64 -z max-page-size=0x1000 -nostdlib --print-gc-sections --no-gc-sections
CFLAGS  += -mcmodel=large -m64 -fno-omit-frame-pointer -mno-red-zone -fno-stack-protector 

