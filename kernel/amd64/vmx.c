#include <kern/arch.h>
#include <kern/mm.h>
#include <kern/proc.h>
#include <kern/lib.h>
#include "vmx.h"
#include "ept.h"
#include "msr_index.h"

void *ept_pgdir = NULL;
/*****************************************************
 *                       VMCS
 * Bit   Note
 * 0     0: 16,32,
 * 1-9   index
 * 11-10 0:control 1: vm-exit info 2: guest state 3: host state
 * 12
 * 14:13 0: 16 1:64 2:32 3:
 * 31:15
 ******************************************************/
struct vmcs_region {
	unsigned int revision;
	unsigned int indicator;
	unsigned char data[0];
};
static unsigned long vmcsptr = 0;

enum x86_64_reg {
        VCPU_REGS_RAX = 0,
        VCPU_REGS_RCX = 1,
        VCPU_REGS_RDX = 2,
        VCPU_REGS_RBX = 3,
        VCPU_REGS_RSP = 4,
        VCPU_REGS_RBP = 5,
        VCPU_REGS_RSI = 6,
        VCPU_REGS_RDI = 7,
        VCPU_REGS_R8 = 8,
        VCPU_REGS_R9 = 9,
        VCPU_REGS_R10 = 10,
        VCPU_REGS_R11 = 11,
        VCPU_REGS_R12 = 12,
        VCPU_REGS_R13 = 13,
        VCPU_REGS_R14 = 14,
        VCPU_REGS_R15 = 15,
        VCPU_REGS_RIP,
        NR_VCPU_REGS
};


unsigned long allregs[NR_VCPU_REGS];

struct descriptor_table {
        unsigned short limit;
        unsigned long base;
} __attribute__((packed));


static inline unsigned long read_cr0()
{
        unsigned long cr0;
        asm volatile("movq %%cr0,%0" : "=r" (cr0));
        return cr0;
}


static inline void write_cr0(unsigned long cr0)
{
        asm volatile("movq %0,%%cr0" : : "r" (cr0));
}

static inline unsigned long read_cr3()
{
	unsigned long cr3;
	asm volatile("movq %%cr3,%0" : "=r" (cr3));	
	return cr3;
}

static inline void write_cr3(unsigned long cr3)
{
	asm volatile("movq %0,%%cr3" : : "r" (cr3));
}

static inline unsigned long read_cr4()
{
	unsigned long cr4;
	asm volatile("movq %%cr4,%0" : "=r" (cr4));	
	return cr4;
}

static inline void write_cr4(unsigned long cr4)
{
	asm volatile("movq %0,%%cr4" : : "r" (cr4));
}

static inline unsigned long read_rsp()
{
        unsigned long rsp;
        asm volatile("movq %%rsp,%0" : "=r" (rsp));
        return rsp;
}


static inline void get_idt(struct descriptor_table *idt)
{
        asm volatile("sidt %0" : "=m"(*idt));
}

static inline void get_gdt(struct descriptor_table *gdt)
{
        asm("sgdt %0" : "=m"(*gdt));
}

/*
 * To load a MSR, 
 * you put its number to RCX and execute the rdmsr opcode. 
 *  The result is in RAX.
 */
static inline unsigned long rdmsr(unsigned long msr)
{
    unsigned int low, high;
    asm volatile (
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(msr)
    );
    return ((unsigned long)high << 32) | low;
}

static inline void wrmsr(unsigned long msr, unsigned long value)
{
    unsigned int low = value & 0xFFFFFFFF;
    unsigned int high = value >> 32;

    asm volatile (
        "wrmsr"
	:
        : "a"(low), "d"(high), "c"(msr)
    );
}


static void vmxon(unsigned long addr)
{

	asm volatile("vmxon %0 \t\n"
			:
			: "m" (addr)
			: "memory");

/*
#define ASM_VMX_VMXON_RAX         ".byte 0xf3, 0x0f, 0xc7, 0x30"
asm volatile (ASM_VMX_VMXON_RAX
: : "a"(&addr), "m"(addr)
: "memory", "cc");
*/
}

static void  vmptrld(unsigned long addr)
{

	asm volatile("vmptrld %0 \t\n"
			:
			: "m" (addr)
			: "memory");

/*
#define ASM_VMX_VMPTRLD_RAX       ".byte 0x0f, 0xc7, 0x30"
	unsigned char  error;

	asm volatile (ASM_VMX_VMPTRLD_RAX "; setna %0"
			: "=g"(error) : "a"(&addr), "m"(addr)
			: "cc", "memory");
*/
}

/*
static unsigned char
vmclear( unsigned long addr ) {
        unsigned char error = 0;

    asm volatile("clc; vmclear %1; setna %0"
            : "=q"( error ) : "m" ( addr ) : "cc");
    return error;
}
*/

static void vmclear(unsigned long addr)
{
	 asm volatile("vmclear %0 \t\n"
                     : 
                     : "m" (addr)
                     : "memory");

}

static void vmwrite(unsigned long index, unsigned long value)
{

	asm volatile("vmwrite %1, %0"
			:
			:"a" (index), "d" (value)
		    );


/*
        unsigned char  error;
#define ASM_VMX_VMWRITE_RAX_RDX   ".byte 0x0f, 0x79, 0xd0"
        asm volatile (ASM_VMX_VMWRITE_RAX_RDX "; setna %0"
                       : "=q"(error) : "a"(value), "d"(index) : "cc");
*/
}
static unsigned long vmread(unsigned long index)
{

	unsigned long value;
	asm volatile("vmread %1, %0"
		     : "=d" (value) 
		     : "a" (index));
	return value;

/*
#define ASM_VMX_VMREAD_RDX_RAX    ".byte 0x0f, 0x78, 0xd0"
	   unsigned long value;

        asm volatile (ASM_VMX_VMREAD_RDX_RAX
                      : "=a"(value) : "d"(index) : "cc");
        return value;
*/
}

static void vmresume()
{
             /* Load guest registers.  Don't clobber flags. */
 asm volatile(  "mov %c[rax](%0), %%rax \n\t"
                "mov %c[rbx](%0), %%rbx \n\t"
                "mov %c[rdx](%0), %%rdx \n\t"
                "mov %c[rsi](%0), %%rsi \n\t"
                "mov %c[rdi](%0), %%rdi \n\t"
                "mov %c[rsp](%0), %%rsp \n\t"
                "mov %c[rbp](%0), %%rbp \n\t"
                "mov %c[r8](%0),  %%r8  \n\t"
                "mov %c[r9](%0),  %%r9  \n\t"
                "mov %c[r10](%0), %%r10 \n\t"
                "mov %c[r11](%0), %%r11 \n\t"
                "mov %c[r12](%0), %%r12 \n\t"
                "mov %c[r13](%0), %%r13 \n\t"
                "mov %c[r14](%0), %%r14 \n\t"
                "mov %c[r15](%0), %%r15 \n\t"
                "mov %c[rcx](%0), %%rcx \n\t" /* kills %0 (ecx) */
                :
                : "c"(&allregs),
           	[rax]"i"(VCPU_REGS_RAX * 8),
                [rbx]"i"(VCPU_REGS_RBX * 8 ),
                [rcx]"i"(VCPU_REGS_RCX * 8 ),
                [rdx]"i"(VCPU_REGS_RDX * 8),
                [rsi]"i"(VCPU_REGS_RSI * 8),
                [rdi]"i"(VCPU_REGS_RDI * 8),
                [rsp]"i"(VCPU_REGS_RSP * 8),
                [rbp]"i"(VCPU_REGS_RBP * 8),
                [r8]"i"(VCPU_REGS_R8 *8 ),
                [r9]"i"(VCPU_REGS_R9 *8 ),
                [r10]"i"(VCPU_REGS_R10 *8),
                [r11]"i"(VCPU_REGS_R11*8),
                [r12]"i"(VCPU_REGS_R12*8),
                [r13]"i"(VCPU_REGS_R13*8),
                [r14]"i"(VCPU_REGS_R14*8),
                [r15]"i"(VCPU_REGS_R15*8)
		              : "cc", "memory"
                , "rbx","rdi", "rsi", "rax","rdx"
                , "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"

			);

	asm volatile("vmresume");
}

static void vmlaunch()
{
	asm volatile("vmlaunch");
}


int sys_run_vm()
{
//	vmlaunch();
	vmexit(&allregs);
	return 0;
	
}
extern struct proc *current;
int sys_mmap_mem(void *addr, unsigned long length, void *gpa,  int prot)
{
	struct page *pp;
	unsigned long start;
        unsigned long sz;
	int r;
	unsigned int offset; 

	offset = (unsigned long)gpa - ((unsigned long)gpa / PAGESIZE * PAGESIZE);
	//offset = 0;
	pp = alloc_page(1);
	memcpy((void *)PAGE2VA(pp) + offset, addr, length);

        start = (unsigned long)addr / PAGESIZE * PAGESIZE;
        sz = (length + PAGESIZE - 1) / PAGESIZE * PAGESIZE;

	r = insert_page(ept_pgdir, gpa, pp, VMX_EPT_PTE_FULL);
/*
	  unsigned long *pte;
	r = walk_page(ept_pgdir, 0xb8000, 1, &pte);
        if(r < 0 ) {
                print("walk_page error\n");
                return r;
        }
        *pte = 0xb8000| VMX_EPT_PTE_FULL | 0x40;
*/
	//r = insert_page(ept_pgdir, 0xb8000, PA2PAGE(0xb8000), VMX_EPT_PTE_FULL);
	//print("ssss:%lx, %lx\n", (unsigned long)gpa, length);

        return r;
}

static inline unsigned long check_vmx_cpuid()
{
	unsigned long vmx_flags;
	asm volatile(
		"movl $1, %%eax \t\n"
		"cpuid"
		: "=c" (vmx_flags)
	);
}

unsigned int cpu_based_exec_control = 0;
unsigned int cpu_based_2nd_exec_control;
unsigned int cpu_ept_vpid_cap;

static inline unsigned long check_ept()
{
	unsigned int low, high;
	unsigned long msr_value;

	msr_value = rdmsr(MSR_IA32_VMX_PROCBASED_CTLS);
	high = msr_value >> 32;
	cpu_based_exec_control |= high;
	print("cpu_based_exec_control:%lx\n",cpu_based_exec_control);

	if(cpu_based_exec_control & 0x80000000) {
		msr_value = rdmsr(MSR_IA32_VMX_PROCBASED_CTLS2);
		high = msr_value >> 32;
		cpu_based_2nd_exec_control |=  high;
		print("cpu_based_2nd_exec_control:%lx\n", cpu_based_2nd_exec_control);

		if(cpu_based_2nd_exec_control & 0x00000002) {
			cpu_ept_vpid_cap = rdmsr(MSR_IA32_VMX_EPT_VPID_CAP);
			print("check_ept:%lx\n", cpu_ept_vpid_cap);
			print("Enable intel EPT support\n");
			return 0;	
		}
	}
	return -1;
}

static int handle_io()
{
	unsigned long rip;

        rip = vmread(GUEST_RIP);
        rip += vmread(VM_EXIT_INSTRUCTION_LEN);
        vmwrite(GUEST_RIP, rip);

        vmresume();
        return 0;
	
}
static int handle_rdmsr()
{

	unsigned long rip;
	//print("handle_rdmsr======================\n");	
        rip = vmread(GUEST_RIP);
        rip += vmread(VM_EXIT_INSTRUCTION_LEN);
	vmwrite(GUEST_RIP, rip);
	vmresume();
	return 0;
}

static int handle_intr()
{
	unsigned int exit_intr_info;
	unsigned int entry_intr_info = 0;

	unsigned char nr, type;
	/* Get exit  intr info */
	exit_intr_info = vmread(VM_EXIT_INTR_INFO);
	nr = exit_intr_info & 0xff;
	type = (exit_intr_info >> 8) & 0x7;

	/* Insert  intr info */
	entry_intr_info |= 0x80000000;
	entry_intr_info |= (type << 8);
	entry_intr_info |= nr;
	
	vmwrite(VM_ENTRY_INTR_INFO_FIELD, entry_intr_info);
	//vmwrite(0x4826, 0);
	print("vm exit intr info :%lx\n",vmread(VM_EXIT_INTR_INFO));
	print("vm entry intr info :%lx\n",vmread(VM_ENTRY_INTR_INFO_FIELD));
	print("vm err code:%lx\n",vmread(VM_EXIT_INTR_ERROR_CODE));
	//send_eoi(0);
	asm volatile("sti");
	
	unsigned long rflags;
	asm volatile("pushfq \t\n"
		      "popq %0 "
			: "=c" (rflags));
	print("ssssssssssssss:%lx\n", rflags);
	rflags |= 0x200;
	asm volatile("pushq %0 \t\n"
                      "popfq "
                        : 
			: "c" (rflags));
	   asm volatile("pushfq \t\n"
                      "popq %0 "
                        : "=c" (rflags));
        print("ssssssssssssss:%lx\n", rflags);
	
	print("vm err eflag:%lx\n",vmread(GUEST_RFLAGS));
	//vmwrite(GUEST_RFLAGS, 0x2);
	//print("handle_intr\n");	
	vmresume();
	return 0;
}
#define VM_ENTRY_IA32E_MODE                     0x00000200
static int handle_wrmsr()
{
	unsigned cr0;

	cr0 = vmread(GUEST_CR0);
	cr0 |= 0x80000001;
	vmwrite(GUEST_CR0, cr0);
	vmwrite(VM_ENTRY_CONTROLS,
			vmread(VM_ENTRY_CONTROLS) |
			VM_ENTRY_IA32E_MODE);

	unsigned long rip;

	rip = vmread(GUEST_RIP);
	rip += vmread(VM_EXIT_INSTRUCTION_LEN);
	vmwrite(GUEST_RIP, rip);
	vmresume();

	return 0;
}

static int handle_eptexit()
{
	int r;
	unsigned long gpa;
	unsigned long rip;

	gpa = vmread(GUEST_PHYSICAL_ADDRESS);

	print("ept violation:%lx\n", gpa);
	print("cr3:%lx\n", vmread(GUEST_CR3));
	
        rip = vmread(GUEST_RIP);
	print("rip :%lx\n", rip);

        //vmresume();
        //vmlaunch();

	return 0;
}

unsigned long exit_reason = 0;
void vmexit(unsigned long *pregs)
{

	asm volatile("vmlaunch \n\t"
		     "Vmx_exit:  nop \n\t"
		      "pushq %rcx \n\t"
		    );
#if 1
	asm volatile(   
			"mov %%rax, %c[rax](%0) \n\t"
			"mov %%rbx, %c[rbx](%0) \n\t"
			"mov %%rdx, %c[rdx](%0) \n\t"
			"mov %%rsi, %c[rsi](%0) \n\t"
			"mov %%rdi, %c[rdi](%0) \n\t"
			"mov %%rsp, %c[rsp](%0) \n\t"
			"mov %%rbp, %c[rbp](%0) \n\t"
			"mov %%r8,  %c[r8](%0) \n\t"
			"mov %%r9,  %c[r9](%0) \n\t"
			"mov %%r10, %c[r10](%0) \n\t"
			"mov %%r11, %c[r11](%0) \n\t"
			"mov %%r12, %c[r12](%0) \n\t"
			"mov %%r13, %c[r13](%0) \n\t"
			"mov %%r14, %c[r14](%0) \n\t"
			"mov %%r15, %c[r15](%0) \n\t"
			"popq %c[rcx](%0)"
			:
			: "c"(&allregs),
			[rax]"i"(VCPU_REGS_RAX*8),
			[rbx]"i"(VCPU_REGS_RBX*8),
			[rcx]"i"(VCPU_REGS_RCX*8),
			[rdx]"i"(VCPU_REGS_RDX*8),
			[rsi]"i"(VCPU_REGS_RSI*8),
			[rdi]"i"(VCPU_REGS_RDI*8),
			[rsp]"i"(VCPU_REGS_RSP*8),
			[rbp]"i"(VCPU_REGS_RBP*8),
			[r8]"i"(VCPU_REGS_R8*8),
			[r9]"i"(VCPU_REGS_R9*8),
			[r10]"i"(VCPU_REGS_R10*8),
			[r11]"i"(VCPU_REGS_R11*8),
			[r12]"i"(VCPU_REGS_R12*8),
			[r13]"i"(VCPU_REGS_R13*8),
			[r14]"i"(VCPU_REGS_R14*8),
			[r15]"i"(VCPU_REGS_R15*8)
			: "cc", "memory"
			, "rbx","rdi", "rsi", "rax","rdx"
			, "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
		);
#endif
/*
	int i;
	for(i=0;i<16;i++)
	print("%lx ", allregs[i]);	
*/
/*

	print("aaaaaaaaaaaaa\n");
        print("Error code :%lx", vmread(VM_INSTRUCTION_ERROR));
        print("physical :%lx\n",vmread(GUEST_PHYSICAL_ADDRESS));
        print("physical rip :%lx\n",vmread(GUEST_RIP));
*/
        exit_reason = vmread(VM_EXIT_REASON);
        //print("exit_reason_xxx:%d\n", exit_reason );
        switch(exit_reason & 0xFFFF) {
		case 1:
			handle_intr(); 		
			break;
		case 12:
			while(1) ;
			break;
		case 30:
			handle_io();
			break;
		case 31:
			handle_rdmsr();	
			break;
		case 32:
			handle_wrmsr();				
			break;
		case 48:
			handle_eptexit();
			break;
			
                default:
			//handle_rdmsr();			
                        print("exit_reason:%d\n", exit_reason & 0xFFFF);
                        break;
        }
        print("exit_qualification:%lx\n", vmread(EXIT_QUALIFICATION));

}
/*
#define VMX_EPT_WB   0x6
#define VMX_EPT_PWL  0x18
#define VMX_EPT_AD  (1ull << 6) 
*/
static unsigned long construct_eptp(unsigned long root_hpa)
{
        unsigned long eptp = 0;

//        eptp = VMX_EPT_WB | VMX_EPT_PWL | 0x7;
//	eptp |= VMX_EPT_AD;
	eptp |= 0x18;
        eptp |= (root_hpa / 4096 * 4096);

        return eptp;
}


static void init_vmx_control()
{
	unsigned int low, high;
        unsigned long msr_value;
	unsigned int vm_entry_control;

	/*1. vmx entry control filed */
	msr_value = rdmsr(MSR_IA32_VMX_ENTRY_CTLS);
	high = msr_value >> 32;
	low = msr_value ;
	vm_entry_control = low & high;
	print("vm_entry_control:%lx ", vm_entry_control);
	vmwrite(VM_ENTRY_CONTROLS, vm_entry_control);


	//vmwrite(VM_ENTRY_CONTROLS,0x11fb);
	//vmwrite(VM_ENTRY_CONTROLS, 0x00FF);
	//0x4000 : A value of 2 in bits 14:13 of an encoding indicates a 32-bit field.
	
	/*2. vmx execute control filed */
	unsigned int pin_based_vm_exec_control;
	msr_value = rdmsr(MSR_IA32_VMX_PINBASED_CTLS);	
	high = msr_value >> 32;
       //low = msr_value ; //external interrupts cause VM exits
       low = msr_value | 0x1; //external interrupts cause VM exits
	pin_based_vm_exec_control = high & low;
	vmwrite(PIN_BASED_VM_EXEC_CONTROL, pin_based_vm_exec_control); //Interrupts
//	vmwrite(PIN_BASED_VM_EXEC_CONTROL, 0x1F); //Interrupts
	print("pin_based_vm_exec_control:%lx ",pin_based_vm_exec_control);
	
//	vmwrite(CPU_BASED_VM_EXEC_CONTROL, 0x860061f2); //Processor
	//vmwrite(CPU_BASED_VM_EXEC_CONTROL, 0x8401e9f2); //Processor
	//vmwrite(CPU_BASED_VM_EXEC_CONTROL, 0x8401e9f2); //Processor
	unsigned int cpu_based_exec_control;
        msr_value = rdmsr(MSR_IA32_VMX_PROCBASED_CTLS);
        high = msr_value >> 32;
	low = msr_value | 0x80 | 0x80000000 ;
        cpu_based_exec_control = low & high;
	print("cpu_based_exec_control:%lx ", cpu_based_exec_control);
	vmwrite(CPU_BASED_VM_EXEC_CONTROL, 0x860061f2); //Processor
//	vmwrite(CPU_BASED_VM_EXEC_CONTROL, cpu_based_exec_control); //Processor
//	vmwrite(CPU_BASED_VM_EXEC_CONTROL, 0x840069F2); //Processor
	//vmwrite(CPU_BASED_VM_EXEC_CONTROL, 0xA50061F2); //Processor


	unsigned int second_vm_exec_control;
	msr_value = rdmsr(MSR_IA32_VMX_PROCBASED_CTLS2);
	    high = msr_value >> 32;
        low = msr_value | 0x82;
	second_vm_exec_control = high & low;
	print("second_vm_exec_control:%lx", second_vm_exec_control);
	vmwrite(SECONDARY_VM_EXEC_CONTROL, second_vm_exec_control);

	/*3. vmx exit control filed */
	unsigned int vm_exit_controls;
	msr_value = rdmsr(MSR_IA32_VMX_EXIT_CTLS);	
	high = msr_value >> 32;
	/* 
 	* If such a VM exit occurs and this control is 1, 
 	* the logical processor acknowledges the
 	* interrupt controller, acquiring the interrupt’s vector. 
 	*/
        low = msr_value | 0x8200; 
	vm_exit_controls = high & low;
	vmwrite(VM_EXIT_CONTROLS, vm_exit_controls);
	print("vm_exit_controls:%lx ", vm_exit_controls);
	//vmwrite(VM_EXIT_CONTROLS, 0x36FFF);
	//	 asm volatile ("vmptrld %0; ":: "m" (vmcs));
	//vmwrite(0x681e, 0);
}

static void init_host_state()
{
/*
   HOST_CR0                        = 0x00006c00,
   HOST_CR3                        = 0x00006c02,
   HOST_CR4                        = 0x00006c04,
*/
	vmwrite(HOST_CR0, read_cr0());
	vmwrite(HOST_CR3, read_cr3());
	vmwrite(HOST_CR4, read_cr4());
/*
   HOST_ES_SELECTOR                = 0x00000c00,
   HOST_CS_SELECTOR                = 0x00000c02,
   HOST_SS_SELECTOR                = 0x00000c04,
   HOST_DS_SELECTOR                = 0x00000c06,
   HOST_FS_SELECTOR                = 0x00000c08,
   HOST_GS_SELECTOR                = 0x00000c0a,
   HOST_TR_SELECTOR                = 0x00000c0c,
*/
	vmwrite(HOST_CS_SELECTOR, 0x08);
	vmwrite(HOST_DS_SELECTOR, 0x10);
	vmwrite(HOST_ES_SELECTOR, 0);
	vmwrite(HOST_FS_SELECTOR, 0);
	vmwrite(HOST_GS_SELECTOR, 0);
	vmwrite(HOST_SS_SELECTOR, 0x10);
	vmwrite(HOST_TR_SELECTOR, 0x28);	
/*
   HOST_FS_BASE                    = 0x00006c06,
   HOST_GS_BASE                    = 0x00006c08,
   HOST_TR_BASE                    = 0x00006c0a,
   HOST_GDTR_BASE                  = 0x00006c0c,
   HOST_IDTR_BASE                  = 0x00006c0e,
*/
	struct descriptor_table gdt, idt;
	get_gdt(&gdt);
	vmwrite(HOST_GDTR_BASE, gdt.base);
	get_idt(&idt);
	vmwrite(HOST_IDTR_BASE, idt.base);
	print("idt:%lx, gdt:%lx\n", idt.base, gdt.base);
/*
   HOST_IA32_SYSENTER_ESP          = 0x00006c10,
   HOST_IA32_SYSENTER_EIP          = 0x00006c12,
   HOST_RSP                        = 0x00006c14,
   HOST_RIP                        = 0x00006c16,
*/
	unsigned long rsp;
	unsigned long tmpl;
        asm("movabs $Vmx_exit, %0" : "=r"(tmpl));
        //vmwrite(HOST_RIP, tmpl);

	vmwrite(HOST_RIP, tmpl);
	vmwrite(HOST_RSP, read_rsp()+8);
	unsigned long msr = rdmsr(0xc0000080);
	vmwrite(0x2c02, msr);
	/*
	asm volatile ("movl $0xc0000080, %%ecx\t\n"
		      "rdmsr \t\n"
			"movq $0x2c02, %%rbx \t\n"
			"vmwrite %%rbx, %%rax"
			::); 
	*/
}

static void init_guest_state()
{
/*
   GUEST_CR0                       = 0x00006800,
   GUEST_CR3                       = 0x00006802,
   GUEST_CR4                       = 0x00006804,
   GUEST_RFLAGS                    = 0x00006820,
*/
	//vmwrite(GUEST_CR0, 0x60000030); //And the NX bit must be set
	vmwrite(GUEST_CR0, 0x00000020); //And the NX bit must be set
	vmwrite(GUEST_CR3, 0);
	vmwrite(GUEST_CR4, 0x2000); //the 13th bit of CR4 must be set in VMX mode
	vmwrite(GUEST_RFLAGS, 0x2);
/*
   GUEST_CS_SELECTOR               = 0x00000802,
   GUEST_CS_LIMIT                  = 0x00004802,
   GUEST_CS_AR_BYTES               = 0x00004816,
   GUEST_CS_BASE                   = 0x00006808,
   GUEST_RIP                       = 0x0000681e,
*/
	vmwrite(GUEST_CS_SELECTOR, 0);
	vmwrite(GUEST_CS_LIMIT, 0xffff);
	vmwrite(GUEST_CS_AR_BYTES ,0x9f);
	vmwrite(GUEST_CS_BASE, 0);
	vmwrite(GUEST_RIP, 0x7C00);
	//vmwrite(GUEST_RIP, 0x0000);
	vmwrite(GUEST_RSP, 0x000);
/*
   GUEST_GDTR_BASE                 = 0x00006816,
   GUEST_IDTR_BASE                 = 0x00006818,
   GUEST_GDTR_LIMIT                = 0x00004810,
   GUEST_IDTR_LIMIT                = 0x00004812,
*/
	vmwrite(GUEST_GDTR_BASE, 0);
	vmwrite(GUEST_IDTR_BASE, 0);
	vmwrite(GUEST_GDTR_LIMIT, 0x30);
	vmwrite(GUEST_IDTR_LIMIT, 0x3ff);
/*
   GUEST_ES_AR_BYTES               = 0x00004814,
   GUEST_CS_AR_BYTES               = 0x00004816,
   GUEST_SS_AR_BYTES               = 0x00004818,
   GUEST_DS_AR_BYTES               = 0x0000481a,
   GUEST_FS_AR_BYTES               = 0x0000481c,
   GUEST_GS_AR_BYTES               = 0x0000481e,
*/
	vmwrite(GUEST_ES_AR_BYTES, 0x93);
	vmwrite(GUEST_SS_AR_BYTES, 0x93);
	vmwrite(GUEST_DS_AR_BYTES, 0x93);
	vmwrite(GUEST_FS_AR_BYTES, 0x93);
	vmwrite(GUEST_GS_AR_BYTES, 0x93);
/*
   GUEST_ES_LIMIT                  = 0x00004800,
   GUEST_CS_LIMIT                  = 0x00004802,
   GUEST_SS_LIMIT                  = 0x00004804,
   GUEST_DS_LIMIT                  = 0x00004806,
   GUEST_FS_LIMIT                  = 0x00004808,
   GUEST_GS_LIMIT                  = 0x0000480a,
*/
	vmwrite(GUEST_ES_LIMIT, 0xffff);
	vmwrite(GUEST_SS_LIMIT, 0xffff);
	vmwrite(GUEST_DS_LIMIT, 0xffff);
	vmwrite(GUEST_FS_LIMIT, 0xffff);
	vmwrite(GUEST_GS_LIMIT, 0xffff);
/*
   GUEST_ES_SELECTOR               = 0x00000800,
   GUEST_CS_SELECTOR               = 0x00000802,
   GUEST_SS_SELECTOR               = 0x00000804,
   GUEST_DS_SELECTOR               = 0x00000806,
   GUEST_FS_SELECTOR               = 0x00000808,
   GUEST_GS_SELECTOR               = 0x0000080a,
*/

	
	vmwrite(GUEST_ES_SELECTOR, 0);
	vmwrite(GUEST_SS_SELECTOR, 0);
	vmwrite(GUEST_DS_SELECTOR, 0);
	vmwrite(GUEST_FS_SELECTOR, 0);
	vmwrite(GUEST_GS_SELECTOR, 0);
/*
   GUEST_LDTR_SELECTOR             = 0x0000080c,
   GUEST_TR_SELECTOR               = 0x0000080e,
   GUEST_LDTR_LIMIT                = 0x0000480c,
   GUEST_TR_LIMIT                  = 0x0000480e,
   GUEST_LDTR_AR_BYTES             = 0x00004820,
   GUEST_TR_AR_BYTES               = 0x00004822,
   GUEST_GDTR_BASE                 = 0x00006816,
   GUEST_IDTR_BASE                 = 0x00006818,
 */
	vmwrite(GUEST_LDTR_SELECTOR, 0);
	vmwrite(GUEST_TR_SELECTOR, 0);
	vmwrite(GUEST_LDTR_LIMIT, 0xffff);
	vmwrite(GUEST_TR_LIMIT, 0xffff);
	vmwrite(GUEST_LDTR_AR_BYTES, 0x82);
	vmwrite(GUEST_TR_AR_BYTES, 0x8b);
	vmwrite(GUEST_GDTR_BASE, 0);
	vmwrite(GUEST_IDTR_BASE, 0);
	vmwrite(GUEST_DR7, 0);
	vmwrite(GUEST_ACTIVITY_STATE, 0);
	vmwrite(GUEST_INTERRUPTIBILITY_INFO, 0);
}


/*Allocate Guest memory */
int sys_alloc_vm_mem(unsigned long phys)
{
	struct page *pp = NULL;
	unsigned long gpa = 0;
	int r;
	unsigned long *pte;

        r = walk_page(ept_pgdir, 0xb8000, 1, &pte);
        if(r < 0 ) {
                print("walk_page error\n");
                return r;
        }

        *pte = 0xb8000| VMX_EPT_PTE_FULL | 0x40;
	int i;

	for(i=0; i < 0xA0000; i += 4096) {
		pp = alloc_page(1);
		r = insert_page(ept_pgdir, i, pp, VMX_EPT_PTE_FULL);
	}

	for(gpa = 0x218000; gpa < phys; gpa += PAGESIZE  ) {
		pp = alloc_page(1);
		r = insert_page(ept_pgdir, gpa, pp, VMX_EPT_PTE_FULL);
	}
	return 0;
}

/* Allocate vitrual CPU resource */
int sys_create_vcpu()
{
	print("sys_create_vcpu\n");
	memset(&allregs, 0, sizeof(unsigned long) *NR_VCPU_REGS);
	struct vmcs_region *vmcs = NULL;
	struct page *pp = NULL;
	unsigned long basic_msr;

        basic_msr = rdmsr(0x480);

	//1. The VCMS Initialization
	pp = alloc_page(1);
	vmcs = (struct vmcs_region *)PAGE2VA(pp);
	vmcs->revision = basic_msr & 0xFFFFFFFF;
	
	print("basic_msr version:%lx\n", vmcs->revision);
	vmclear((unsigned long)PAGE2PA(pp));
	vmcsptr = (unsigned long)PAGE2PA(pp);
	//vmcs->revision = basic_msr & 0xFFFFFFFF;
	vmptrld((unsigned long)PAGE2PA(pp));
#if 1
	//2. The VMX EPT Initialization
	unsigned long eptp;
	pp = alloc_page(1);
	ept_pgdir = (void *)PAGE2VA(pp);
	eptp = PAGE2PA(pp) | 0x18;
	vmwrite(EPT_POINTER, eptp);
#endif
#if 0
	extern unsigned char vmx_pgkdir[];
	unsigned long eptp;

	print("vmx_pgkdir:%lx\n", vmx_pgkdir - KBASE);

	eptp = construct_eptp(vmx_pgkdir - KBASE);

	//eptp = construct_eptp(KVA2PA(vmx_pgkdir));
	print("vmx_pgkdir:%lx\n", eptp);
	vmwrite(EPT_POINTER, eptp);
	//vmwrite(EPT_POINTER, PAGE2PA(pp) | 0x18);
#endif

	
	//3. The VMX Controls Initialization
	init_vmx_control();
	
	//4. The VMX host
	init_host_state();
	
	//5. The VMX guest
	init_guest_state();	
	
	
	vmwrite(VMCS_LINK_POINTER, 0xFFFFFFFF);
	vmwrite(VMCS_LINK_POINTER_HIGH, 0xFFFFFFFF);

	//6. Run VM
	//vmlaunch();

	//7. Exit VM
		
	return 0;
}

/* Enable intel VMX 
Enter Long Mode.
Set CR4's bit 13 to 1. This bit enables the VMX operations.
Set CR0's bit 5 to 1 (NE) - this is required for the VMXON to succeed.
Initialize a VMXON region.
Execute the VMXON instruction.
*/
static void enable_vmx()
{
	unsigned long cpuid = check_vmx_cpuid();	
//	print("cpuid :%x\n", cpuid);
	 //1. Set CR4's bit 13 to 1. This bit enables the VMX operations.
	unsigned long cr4;
	cr4 = read_cr4();
	cr4 |= 0x2000;
	write_cr4(cr4);
/*
	cr4 = read_cr4();
	print("cr4:%lx\n", cr4);
*/
/*
	unsigned long cr4;
	asm volatile( "movq %%cr4, %%rax \t\n"
	 "bts $13,%%rax \t\n"
	"movq %%rax, %%cr4"
	:
	:
	);

	cr4 = read_cr4();

*/
	//2. Set CR0's bit 5 to 1 (NE) - this is required for the VMXON to succeed
	unsigned long cr0;
	cr0 = read_cr0();
	cr0 |= 0x20;
	write_cr0(cr0);
	cr0 = read_cr0();
	//print("cr0:%lx\n", cr0);
	
	//3. Unlock
//	print("vmx feature control: %lx\n", rdmsr(0x3a));
	unsigned long feature = rdmsr(0x3a);
	feature |= 0x1;
	wrmsr(0x3a, feature);	

	check_ept();

	//4. Initialize a VMXON region.
	unsigned long basic_msr;
	basic_msr = rdmsr(0x480);

	print("vmx revision: %lx\n", basic_msr);	
	struct page * pp = NULL;
	unsigned int version = basic_msr & 0xFFFFFFFF;

	pp = alloc_page(1);
	*(unsigned int *)PAGE2VA(pp) = version;

	//5. Execute the VMXON instruction.
	vmxon((unsigned long)PAGE2PA(pp));	
}

void init_vmx()
{
	enable_vmx();
}

