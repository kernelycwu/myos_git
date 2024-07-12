as  -32 -o boot.o boot.S
ld -m elf_i386 -Ttext 0x7C00 boot.o -o testboot
cp testboot{,.bak}
objcopy -O binary testboot testboot.bin
xxd -i testboot.bin

