/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bdiff:bdiff.c	5.12"

#include	<fatal.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<unistd.h>
#include	<stdio.h>
#include	<ctype.h>

#define ONSIG	16

char *satoi();
char	*Mesg[ONSIG]={
	0,
	0,	/* Hangup */
	0,	/* Interrupt */
	0,	/* Quit */
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad system call",
	"Broken pipe",
	"Alarm clock"
};

/*
	This program segments two files into pieces of <= seglim lines
	(which is passed as a third argument or defaulted to some number)
	and then executes diff upon the pieces. The output of
	'diff' is then processed to make it look as if 'diff' had
	processed the files whole. The reason for all this is that seglim
	is a reasonable upper limit on the size of files that diff can
	process.
	NOTE -- by segmenting the files in this manner, it cannot be
	guaranteed that the 'diffing' of the segments will generate
	a minimal set of differences.
	This process is most definitely not equivalent to 'diffing'
	the files whole, assuming 'diff' could handle such large files.

	'diff' is executed by a child process, generated by forking,
	and communicates with this program through pipes.
*/

char Error[128];

int seglim;	/* limit of size of file segment to be generated */

FILE *fdopen();
char diff[]  =  "diff";
char tempskel[] = "/tmp/bdXXXXX";	/* used to generate temp file names */
char tempfile[32];
char otmp[32], ntmp[32];
char	*Ffile;
int linenum, Fflags;

char *prognam;

main(argc,argv)
int argc;
char *argv[];
{
	FILE *poldfile, *pnewfile, *maket();
	char oline[BUFSIZ], nline[BUFSIZ], diffline[BUFSIZ];
	char *olp, *nlp, *dp;
	int otcnt, ntcnt;
	pid_t i;
	int pfd[2];
	FILE *poldtemp, *pnewtemp, *pipeinp;
	int status;

	prognam = argv[0];
	/*
	Set flags for 'fatal' so that it will clean up,
	produce a message, and terminate.
	*/
	Fflags = FTLMSG | FTLCLN | FTLEXIT;

	setsig();

	if (argc < 3 || argc > 5)
		fatal("arg count");

	if (strcmp(argv[1],"-") == 0 && strcmp(argv[2],"-") == 0) 
		fatal("both files standard input");
	if (strcmp(argv[1],"-") == 0)
		poldfile = stdin;
	else
		if((poldfile = fopen(argv[1],"r")) == NULL) {
			sprintf(Error, "Can not open '%s'", argv[1]);
			fatal(Error);
		}
	if (strcmp(argv[2],"-") == 0)
		pnewfile = stdin;
	else
		if((pnewfile = fopen(argv[2], "r")) == NULL) {
			sprintf(Error, "Can not open '%s'", argv[2]);
			fatal(Error);
		}

	seglim = 3500;

	if (argc > 3) {
		if (argv[3][0] == '-' && argv[3][1] == 's')
			Fflags &= ~FTLMSG;
		else {
			if ((seglim = atoi(argv[3])) == 0)
				fatal("non-numeric limit");
			if (argc == 5 && argv[4][0] == '-' &&
					argv[4][1] == 's')
				Fflags &= ~FTLMSG;
		}
	}

	linenum = 0;

	/*
	The following while-loop will prevent any lines
	common to the beginning of both files from being
	sent to 'diff'. Since the running time of 'diff' is
	non-linear, this will help improve performance.
	If, during this process, both files reach EOF, then
	the files are equal and the program will terminate.
	If either file reaches EOF before the other, the
	program will generate the appropriate 'diff' output
	itself, since this can be easily determined and will
	avoid executing 'diff' completely.
	*/
	while (1) {
		olp = fgets(oline,BUFSIZ,poldfile);
		nlp = fgets(nline,BUFSIZ,pnewfile);

		if (!olp && !nlp)	/* files are e == 0qual */
			exit(0);

		if (!olp) {
			/*
			The entire old file is a prefix of the
			new file. Generate the appropriate "append"
			'diff'-like output, which is of the form:
					nan,n
			where 'n' represents a line-number.
			*/
			addgen(nline,pnewfile);
		}

		if (!nlp) {
			/*
			The entire new file is a prefix of the
			old file. Generate the appropriate "delete"
			'diff'-like output, which is of the form:
					n,ndn
			where 'n' represents a line-number.
			*/
			delgen(oline,poldfile);
		}

		if (strcmp(olp,nlp) == 0)
			linenum++;
		else
			break;
	}

	/*
	Here, first 'linenum' lines are equal.
	The following while-loop segments both files into
	seglim segments, forks and executes 'diff' on the
	segments, and processes the resulting output of
	'diff', which is read from a pipe.
	*/
	while (1) {
		/*
		If both files are at EOF, everything is done.
		*/
		if (!olp && !nlp)	/* finished */
			exit(0);

		if (!olp) {
			/*
			Generate appropriate "append"
			output without executing 'diff'.
			*/
			addgen(nline,pnewfile);
		}

		if (!nlp) {
			/*
			Generate appropriate "delete"
			output without executing 'diff'.
			*/
			delgen(oline,poldfile);
		}

		/*
		Create a temporary file to hold a segment
		from the old file, and write it.
		*/
		poldtemp = maket(otmp);
		otcnt = 0;
		while(olp && otcnt < seglim) {
			fputs(oline,poldtemp);
			if (ferror(poldtemp) != 0) {
				Fflags |= FTLMSG;
				fatal("Can not write to temporary file");
			}
			olp = fgets(oline,BUFSIZ,poldfile);
			otcnt++;
		}
		fclose(poldtemp);

		/*
		Create a temporary file to hold a segment
		from the new file, and write it.
		*/
		pnewtemp = maket(ntmp);
		ntcnt = 0;
		while(nlp && ntcnt < seglim) {
			fputs(nline,pnewtemp);
			if (ferror(pnewtemp) != 0) {
				Fflags |= FTLMSG;
				fatal("Can not write to temporary file");
			}
			nlp = fgets(nline,BUFSIZ,pnewfile);
			ntcnt++;
		}
		fclose(pnewtemp);

		/*
		Create pipes and fork.
		*/
		if((pipe(pfd)) == -1)
			fatal("Can not create pipe");
		if ((i = fork()) < (pid_t)0) {
			close(pfd[0]);
			close(pfd[1]);
			fatal("Can not fork, try again");
		}
		else if (i == (pid_t)0) {	/* child process */
			close(pfd[0]);
			close(1);
			dup(pfd[1]);
			close(pfd[1]);

			/*
			Execute 'diff' on the segment files.
			*/
			execlp(diff,diff,otmp,ntmp,0);
			close(1);
			sprintf(Error,"Can not execute '%s'",diff);
			fatal(Error);
		}
		else {			/* parent process */
			close(pfd[1]);
			pipeinp = fdopen(pfd[0], "r");

			/*
			Process 'diff' output.
			*/
			while ((dp = fgets(diffline,BUFSIZ,pipeinp))) {
				if (isdigit(*dp))
					fixnum(diffline);
				else
					printf("%s",diffline);
			}

			fclose(pipeinp);

			/*
			EOF on pipe.
			*/
			wait(&status);
 			if (status&~0x100) {
				sprintf(Error,"'%s' failed",diff);
				fatal(Error);
			}
		}
		linenum += seglim;

		/*
		Remove temporary files.
		*/
		unlink(otmp);
		unlink(ntmp);
	}
}

/*
	Routine to save remainder of a file.
*/
saverest(line,iptr)
char *line;
FILE *iptr;
{
	register char *lp;
	FILE *temptr, *maket();

	temptr = maket(tempfile);

	lp = line;

	while (lp) {
		fputs(line,temptr);
		linenum++;
		lp = fgets(line,BUFSIZ,iptr);
	}
	fclose(temptr);
}

/*
	Routine to write out data saved by
	'saverest' routine and to remove the file.
*/
putsave(line,type)
char *line;
char type;
{
	FILE *temptr;

	if((temptr = fopen(tempfile, "r")) == NULL) {
		sprintf(Error, "Can not open tempfile ('%s')", tempfile);
		fatal(Error);
	}

	while (fgets(line,BUFSIZ,temptr))
		printf("%c %s",type,line);

	fclose(temptr);

	unlink(tempfile);
}

fixnum(lp)
char *lp;
{
	int num;

	while (*lp) {
		switch (*lp) {

		case 'a':
		case 'c':
		case 'd':
		case ',':
		case '\n':
			printf("%c",*lp);
			lp++;
			break;

		default:
			lp = satoi(lp,&num);
			num += linenum;
			printf("%d",num);
		}
	}
}

addgen(lp,fp)
char *lp;
FILE *fp;
{
	printf("%da%d,",linenum,linenum+1);

	/*
	Save lines of new file.
	*/
	saverest(lp,fp);

	printf("%d\n",linenum);

	/*
	Output saved lines, as 'diff' would.
	*/
	putsave(lp,'>');

	exit(0);
}

delgen(lp,fp)
char *lp;
FILE *fp;
{
	int savenum;

	printf("%d,",linenum+1);
	savenum = linenum;

	/*
	Save lines of old file.
	*/
	saverest(lp,fp);

	printf("%dd%d\n",linenum,savenum);

	/*
	Output saved lines, as 'diff' would.
	*/
	putsave(lp,'<');

	exit(0);
}

clean_up()
{
	unlink(tempfile);
	unlink(otmp);
	unlink(ntmp);
}

FILE *
maket(file)
char *file;
{
	FILE *iop;
	register char *cp;
	char *mktemp();

	strcpy(file, tempskel);
	cp = mktemp(file);
	if ((iop = fopen(cp, "w+")) == NULL) {
		sprintf(Error, "Can not open/create temp file ('%s')", cp);
		fatal(Error);
	}
	return(iop);
}

fatal(msg)
char *msg;
/*
	General purpose error handler.
 
	The argument to fatal is a pointer to an error message string.
	The action of this routine is driven completely from
	the "Fflags" global word (see <fatal.h>).
 
	The FTLMSG bit controls the writing of the error
	message on file descriptor 2.  The message is preceded
	by the string "ERROR: ", unless the global character pointer
	"Ffile" is non-zero, in which case the message is preceded
	by the string "ERROR [<Ffile>]: ".  A newline is written
	after the user supplied message.
 
	If the FTLCLN bit is on, clean_up is called with an
	argument of 0.
*/
{
	if (Fflags & FTLMSG) 
		fprintf(stderr, "%s: %s\n", prognam, msg);
	if (Fflags & FTLCLN)
		clean_up(0);
	if (Fflags & FTLEXIT)
		exit(1);
}

setsig()
/*
	General-purpose signal setting routine.
	All non-ignored, non-caught signals are caught.
	If a signal other than hangup, interrupt, or quit is caught,
	a "user-oriented" message is printed on file descriptor 2.
	If hangup, interrupt or quit is caught, that signal	
	is set to ignore.
	Termination is like that of "fatal",
	via "clean_up(sig)" (sig is the signal number).
*/
{
	void setsig1();
	register int j, n;

	for (j=1; j < ONSIG; j++) {
		n = (int) signal(j, setsig1);
		if ((void(*)())n == SIG_ERR)
			continue;
		if ((void(*)())n == SIG_DFL)
			continue;
		signal(j, (void(*)())n);
	}
}

void
setsig1(sig)
int sig;
{

	signal(sig, SIG_IGN);
	clean_up();
	exit(1);
}

char *satoi(p,ip)
register char *p;
register int *ip;
{
	register int sum;

	sum = 0;
	while (isdigit(*p))
		sum = sum * 10 + (*p++ - '0');
	*ip = sum;
	return(p);
}
