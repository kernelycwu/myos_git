#ifndef _X86_H
#define _X86_H  
static inline void
outb(unsigned short port, unsigned char value)
{
        asm volatile("outb %0, %1"
                        :
                        :"a"(value), "d"(port)
                        :);
}
static inline unsigned char
inb(unsigned short port)
{
        unsigned char value;
        asm volatile( "inb %1, %0"
                        : "=a" (value)
                        : "d"(port)
                        : );
        return value;
}
static inline void
insw(unsigned short port, void *buf, unsigned short count)
{
        asm volatile("cld\t\n rep insw"
                    : "=D" (buf)
                    :"D" (buf), "d" (port), "c"(count)
                    : "memory");
}

static inline void
outsw(unsigned short port, void *buf, unsigned short count)
{
        asm volatile("cld\t\n rep outsw"
                    :
                    :"S" (buf), "d" (port), "c"(count)
                    : "memory");
}

#endif

