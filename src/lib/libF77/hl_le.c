/*	@(#)hl_le.c	1.2	*/
short l_le(a,b,la,lb)
char *a, *b;
long int la, lb;
{
return(s_cmp(a,b,la,lb) <= 0);
}
