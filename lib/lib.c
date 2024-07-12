
void *memset(void *s, int c, unsigned int n)
{
	void *p = s;
	while(n != 0) {
		*(unsigned char *)s++ = (unsigned char)c;
		n--;
	}	
	return p;

}
void *memcpy(void *dest, const void *src, unsigned int n)
{
	void *p = dest;
	while(n != 0) {
		*(unsigned char *)dest++ =  *(unsigned char *)src++;
		n--;
	}
	return p;
}
int strcmp(const char *s1, const char *s2)
{
	int r;
	while(*s1 != '\0' || *s2 != '\0' ) {
		if(*s1 == *s2) {
			s1++;
			s2++;
			continue;
		}
		return (*s1 - *s2) ? 1 : -1;
	}
	if(*s1 == '\0' && *s2 == '\0')  
		return 0;
	if(*s1 == '\0')
		return  -1;
	else 
		return 1;
}
