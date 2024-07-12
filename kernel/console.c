#include <kern/lib.h>

/* The number of columns.  */
#define COLUMNS                 80
/* The number of lines.  */
#define LINES                   24
#define CONSIZE 		(COLUMNS *LINES)
/* The attribute of an character.  */
#define ATTRIBUTE               7
/* The video memory address.  */
#ifndef amd64
	#define VIDEO                   0xC00B8000
#else
	#define VIDEO                   0xFFFFFFFF800B8000
#endif
/* Variables.  */
/* Save the X position.  */
static int xpos;
/* Save the Y position.  */
static int ypos;
/* Point to the video memory.  */
static volatile unsigned char *video = (unsigned char *) VIDEO;
/* Put the character C on the screen.  */

void init_kbd(void);
static void init_kbdata(void);
void
cls (void)
{
  int i;

  video = (unsigned char *) VIDEO;

  for (i = 0; i < COLUMNS * LINES * 2; i++)
    *(video + i) = 0;

  xpos = 0;
  ypos = 0;
}

/* Clear the screen and initialize VIDEO, XPOS and YPOS.  */
static void 
init_vga(void )
{
	int i;

	video = (unsigned char *) VIDEO;

	for (i = 0; i < COLUMNS * LINES * 2; i++)
		*(video + i) = 0;

	xpos = 0;
	ypos = 0;
}

void
init_console(void)
{
	init_vga();
	init_kbd();
}


void
myputchar (int c)
{
  int i;
  if (c == '\n' || c == '\r')
    {
    newline:
      xpos = 0;
      ypos++;
      if (ypos >= LINES) {
	      memcpy((void *)video, (void *)video + COLUMNS*2, (CONSIZE-COLUMNS)*2);
	      for (i = (CONSIZE-COLUMNS)*2; i < CONSIZE*2 ; i++)
		      *(video + i) = 0;
	      xpos = 0;		
	      ypos--;
      }
      return;
    }
  *(video + (xpos + ypos * COLUMNS) * 2) = c & 0xFF;
  *(video + (xpos + ypos * COLUMNS) * 2 + 1) = ATTRIBUTE;

  xpos++;
  if (xpos >= COLUMNS)
    goto newline;
}



#ifndef amd64
	typedef char *va_list1;
	#define __va_rounded_size(TYPE) \
		(((sizeof(TYPE) + sizeof(int) -1) / sizeof(int)) * sizeof(int))

	#define va_start1(AP, LASTARG) \
		(AP = ((char *) &(LASTARG) + __va_rounded_size(LASTARG)))

	#define va_arg1(AP, TYPE)	\
		(AP += __va_rounded_size(TYPE), \
	 	*((TYPE*)(AP - __va_rounded_size(TYPE))))	
	#define va_end1(AP) (AP = (va_list1) 0)
#else
	typedef __builtin_va_list  va_list1;
	#define va_start1(v,l)   __builtin_va_start(v,l)
	#define va_end1(v)       __builtin_va_end(v)
	#define va_arg1(v,l)     __builtin_va_arg(v,l)
#endif


const char *strtab = "0123456789abcdef";
int unsigned_recursion(unsigned long val, int base) {
	char t;
	int num = 0;
        if(val >=0 && val < base) {
                t = strtab[val % base];
		myputchar(t);
		num++;
                return num;
        }

        t = strtab[val % base];
        val = val / base;    
	num++;
        num += unsigned_recursion(val, base);
	myputchar(t);
	return num;
	
}
/*
int unsignedlong_recursion(unsigned long val, int base) {
	char t;
	int num = 0;
	if(val >=0 && val < base) {
		t = strtab[val % base];
		myputchar(t);
		num++;
		return num;
	}

	t = strtab[val % base];
	val = val / base;
	num++;
	num += unsigned_recursion(val, base);
	myputchar(t);
	return num;

}
*/
#if 0
int recursion(int val, int base)
{
	char t;
	int num;
	if(val >=0 && val < base) {
		t = strtab[val % base];
		num++;
		myputchar(t);
//		printf("%c",t);
		return;
	}

	if((val < 0) && (base <= 10)) {
		t = '-';
		num++;
		val = -val;
		myputchar(t);
	//	printf("%c", t);
	}

	t = strtab[val % base];
	val = val / base;	
	num++;
	num += recursion(val, base);
	myputchar(t);
	//printf("%c",t);
	return num;	

}
#endif
int print(const char *format, ...)
{
	int val, base;
	long int lval;
	unsigned int unval;
	unsigned long lunval;
	char c;

	char *strval;
	int num = 0;
	va_list1 arg;
	int lflags = 0, uflags = 0, pflags = 0;
	va_start1(arg, format);

	while(*format != '\0') {
		c = *format;

		switch(c) {
//			case '\n':
//				myputchar('\n');
//				break;
			case '%':
				pflags++;
				// format++; /* skip percent */ 
				break;
			case 'l':
				if(pflags == 0)  goto label;
				lflags++;
				break;
			case 'd':
				if(pflags == 0)  goto label;
				base = 10;
				if(lflags == 0) {
					val = va_arg1(arg, int);
					if(val < 0) { 
					val = -val;
					myputchar('-');
					num++;
					}
					num += unsigned_recursion(val, base);
				}
				else {
					lval = va_arg1(arg, long int);
					if(lval < 0) {
						lval = -lval;
						myputchar('-');
						num++;
					}
					num += unsigned_recursion(lval, base);
				}

				pflags = 0;
				break;
			case 'x':
				if(pflags == 0)  goto label;	
				base = 16;
				if(lflags == 0) {
					unval = va_arg1(arg, unsigned int);
					num += unsigned_recursion(unval, base);
				} else {
					 lunval = va_arg1(arg, unsigned long);
					 num += unsigned_recursion(lunval, base);
				}
				pflags = 0;
				break;
			case 'o':
				if(pflags == 0)  goto label;
				base = 8;
				if(lflags == 0) {
					unval = va_arg1(arg, unsigned int);
					num += unsigned_recursion(unval, base);
				} else {
					lunval = va_arg1(arg, unsigned long);
                                         num += unsigned_recursion(lunval, base);
				}
		
				pflags = 0;
				break;
			case 's':
				if(pflags == 0)  goto label;

				strval = va_arg1(arg, char *);
				while(*strval != '\0') {
					myputchar(*strval);
					strval++;
					num++;
				}
				pflags = 0; 
				break;
			
			default: /* not \n % */
label:				num++;
				myputchar(*format);
				break;
		}				
		format++;
	}

	va_end1(arg);
	return num;
}

void panic(const char *fmt)
{
	print("%s",fmt);
	asm volatile("hlt");		
}
#if 1
// note this example will always write to the top
// line of the screen
void 
write_string( const char *string )
{
//    volatile char *video = (volatile char*)0xB8000;
    while( *string != 0 )
    {
        *video++ = *string++;
        *video++ = ATTRIBUTE;
    }
}
char* 
itoa(unsigned int val, int base)
{
	
	static char buf[32] = {0};

	int i = 30;
	memset((void *)&buf,0,32);	
	if(val == 0) {
		buf[30] = '0';
		return &buf[30];
	}
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];

	return &buf[i+1];
	
}
#endif

#if 1
#include <kern/arch.h>

#define KBD_CMD_REG  0x64
#define KDB_CMD_READ  0x20
#define KDB_CMD_WRITE 0x60

#define KBD_STATUS_REG 	0x64
#define KBD_DATA_REG	0x60

/* The keyboard controller command byte */
/* Bit 0: Keyboard interrupt enable */
#define KBD_CMD_KIE	0x01

/* Keyboard controller commands */
/* ae	Enable keyboard */
#define KBD_CMD_EK	0xae

/* The keyboard controller status register */
/*Bit 5: Auxiliary output buffer full */
#define KBD_STAT_AUXB	    0x20	
/*Bit 0: Output buffer status */
#define KBD_STAT_OUTB       0x01
void init_kbd(void)
{

	int status;

	/* init keyboard buffer */
	init_kbdata();

	/* enable Keyboard */
	outb(KBD_CMD_REG, KBD_CMD_EK);

	/* get kdb status */
	outb(KBD_CMD_REG, KDB_CMD_READ);
	status = inb(KBD_DATA_REG);
	print("kdb status:%x\n", status);

	
	/* set kdb interrupt enable */
	outb(KBD_CMD_REG, KDB_CMD_WRITE);
	status |=  KBD_CMD_KIE;
	outb(KBD_DATA_REG, status);	
}
#define NR_KEYS 128
char ascii_map[] = {
' ', '!', '"', '#', '$', '%', '&', '\'', 
'(', ')', '*', '+', ',', '-', '.', '/', 
'0', '1', '2', '3', '4', '5', '6', '7', 
'8', '9', ':', ';', '<', '=', '>', '?', 
'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
'X', 'Y', 'Z', '[', '\\', ']', '^', '_', 
'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 
'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 
'x', 'y', 'z', '{', '|', '}', '~'  
};
unsigned char  plain_map[NR_KEYS] = {
 0x00, 0x1b, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
 0x37, 0x38, 0x39, 0x30, 0x2d, 0x3d, 0x7f, 0x09,
 0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69,
 0x6f, 0x70, 0x5b, 0x5d, 0x0a, 0x02, 0x61, 0x73,
 0x64, 0x66, 0x67, 0x68, 0x6a, 0x6b, 0x6c, 0x3b,
 0x27, 0x60, 0x00, 0x5c, 0x7a, 0x78, 0x63, 0x76,
 0x62, 0x6e, 0x6d, 0x2c, 0x2e, 0x2f, 0x00, 0x0c,
 0x03, 0x20, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04,
 0x05, 0x06, 0x07, 0x08, 0x09, 0x08, 0x09, 0x07,
 0x08, 0x09, 0x0b, 0x04, 0x05, 0x06, 0x0a, 0x01,
 0x02, 0x03, 0x00, 0x10, 0x06, 0x00, 0x3c, 0x0a,
 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x0e, 0x02, 0x0d, 0x1c, 0x01, 0x05, 0x14, 0x03,
 0x18, 0x01, 0x02, 0x17, 0x00, 0x19, 0x15, 0x16,
 0x1a, 0x0c, 0x0d, 0x1b, 0x1c, 0x10, 0x11, 0x1d,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 };

#define KBBUFSIZE 1024
static struct {
        unsigned char buf[KBBUFSIZE];
        unsigned int rpos;
        unsigned int wpos;
} kbs;

int
get_kbdata()
{
        unsigned char  e;
        if (kbs.rpos != kbs.wpos) {
                e = kbs.buf[kbs.rpos];
                kbs.rpos++;
		kbs.rpos = kbs.rpos % KBBUFSIZE;
		/*
                	if (kbs.rpos ==  KBBUFSIZE)
                        	kbs.rpos = 0;
		*/
                return e;
        }
//	print("keyboard buffer is empty\n");
        return -1;
}

static void
add_kbdata(unsigned char data)
{

	/* overwrite if full */
	kbs.buf[kbs.wpos] = data;

	kbs.wpos++;
	kbs.wpos = kbs.wpos % KBBUFSIZE;
	/*
	if (kbs.wpos == KBBUFSIZE)
		kbs.wpos = 0;
	*/
}

static void 
init_kbdata(void) 
{
	kbs.rpos = 0;
	kbs.wpos = 0;	
} 

void handle_scancode(unsigned char scancode)
{
	unsigned char i;
	/* translate keycode into scancode */

	/* translate scancode into asciicode */
	i = plain_map[scancode];
//	print("%d", i);
	if(i > 31 && i < 127) {
		add_kbdata(ascii_map[i-32]);	
		myputchar(ascii_map[i-32]);
	} else
		add_kbdata(i);
	//update_cursor(xpos, ypos);
	//print("%c");		
		
}
void kbd_handler(void)
{
	unsigned char status;
	unsigned char scancode;

	status = inb(KBD_STATUS_REG);
	while(status & KBD_STAT_OUTB) {

		scancode = inb(KBD_DATA_REG);
		if(status & KBD_STAT_AUXB) {
			/* FixME: kbse */
		} else {

			if(scancode < 0x7f) {
//				print("scancode:%x\n", scancode);
				handle_scancode(scancode);
			}				
		}
	
		status = inb(KBD_STATUS_REG);	
	}	
}
#endif
