#include <kern/console.h>
#include <kern/arch.h>

void kernmain()
{
	//cls();
	init_console();
	print("start my os\n");
	init_mem();
	init_smp();
	init_proc();
//	init_vmx();
	init_trap();
	init_pic();
	init_pit();
	init_fs();
	init_files();
//	init_mp_config();
	//init_smp();
	spawn_proc();

	while(1) {
		print("PROCESS RUNNING\n");
		schedule();
		print("NO PROCESS RUNNING\n");
	}
	asm volatile ("hlt");

}
