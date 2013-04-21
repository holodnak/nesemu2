int stricmp(char *s1,char *s2)
{
	char c1,c2;

	for(;*s1 && *s2;s1++,s2++) {
		c1 = tolower(*s1);
		c2 = tolower(*s2);
		if(c1 > c2)
			return(1);
		if(c1 < c2)
			return(-1);
	}
	return(0);
}
