include $(TOP)/kernel/${ARCH}/module.mk
KERN_SRC += 	\
	kernel/init.c \
	kernel/lib.c  \
	kernel/console.c \
	kernel/mm.c \
	kernel/proc.c \
	kernel/syscall.c \
	kernel/pic.c \
	kernel/pit.c \
	kernel/ide.c \
	kernel/fs.c \
	kernel/cache.c \
	kernel/file.c \
	kernel/exec.c \
	kernel/lapic.c
