.global _start
_start:
	nop
	nop
	callq user_main
	callq user_exit
	ret
