/*	@(#)hl_ge.c	1.3	*/
short hl_ge(a,b,la,lb)
char *a, *b;
long int la, lb;
{
return(s_cmp(a,b,la,lb) >= 0);
}
