#ifndef _PT_REGS_
#define _PT_REGS_ 1
/*https://www.felixcloutier.com/x86/pusha:pushad pushal*/
struct pt_regs {
        unsigned int edi;
        unsigned int esi;
        unsigned int ebp;
        unsigned int exx; /* unkown */
        unsigned int ebx;
        unsigned int edx;
        unsigned int ecx;
        unsigned int eax;
        unsigned int  ds;
        unsigned int  es;
        unsigned int  num;
	unsigned int  err_code;
        unsigned int eip;
        unsigned int cs;
        unsigned int eflags;
        unsigned int esp;
        unsigned int ss;
};
#endif
