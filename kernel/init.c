#include <kern/console.h>
#include <kern/arch.h>

extern void init_console();
extern void init_mem();
extern void init_smp();
extern void init_proc();
extern void init_vmx();
extern void init_trap();
extern void init_pic();
extern void init_pit();
extern void init_fs();
extern void init_files();
extern void spawn_proc();
extern void schedule();

void kernmain()
{
	//cls();
	init_console();
	print("start my os\n");
	init_mem();
#if (defined amd64) && (defined host)
	init_smp();
#endif
	init_proc();
#if (defined amd64)  && (defined host)
	init_vmx();
#endif
	init_trap();
	init_pic();
	init_pit();
	init_fs();
	init_files();
	spawn_proc();

	while(1) {
		print("PROCESS RUNNING\n");
		schedule();
		print("NO PROCESS RUNNING\n");
	}
	asm volatile ("hlt");

}
