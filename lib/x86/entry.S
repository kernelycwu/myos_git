.global _start
_start:
	nop
	nop
	call user_main
	call user_exit
	ret
