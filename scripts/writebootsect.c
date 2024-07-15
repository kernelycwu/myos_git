#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
	int ret;
	unsigned char buf[512];

	int fd = open("bootloader.bin", O_RDONLY);
	if (fd < 0 ) {
		perror("bootloader");
		return -1;
	}

	int fd1 = open("kernel.img", O_WRONLY); 
	 if (fd1 < 0 ) {
                perror("open");
                return -1;
        }

	int fd2 = open("kernel.elf", O_RDONLY);
	if (fd2 < 0 ) {
		perror("kernel");
		return -1;
	}

	
	if(read(fd, buf, 512) < 0) {
		perror("read bootloader");
		return -1;
	}

	buf[510] = 0x55;
	buf[511] = 0xaa;

	/* write bootloader */
	if(write(fd1, buf, 512) < 0) {
		perror("write bootloader");
		return -1;
	}

	/* write kernel */
	while( read(fd2, buf, 512) > 0){
		write(fd1, buf, 512);
	}

	close(fd);
	close(fd1);
	close(fd2);
	return 0;
}
