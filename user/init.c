#include <kern/lib.h>
#include <elf.h>


int getline(unsigned char *buf, int n, int fd)
{

        char ch = 0;
        int i = 0;
        int r;
        unsigned char *cur = buf;

        while(ch != '\n' &&  i < n) {
                r = read(fd, &ch, 1);
                if(r == 0)
                        yield();
                else {
                        *cur++ = ch;
                        i++;
                }
        }
        return i;

}


void user_main()
{
        char buf[512];
        int pid;
        char ch;
        int r, i = 0;
        int fd;

        fd = opencons();
        while(1) {
                write(fd, ">", 1);

                getline((unsigned char *)&buf[0], 512, fd);
#if 1
                pid = fork();
                if(pid == 0) {
//			tprintf();
			//while(1) yield();	
                        execve("world", 0, 0);
                } 
#endif
                waitpid();
        }
        while(1) ;
}
