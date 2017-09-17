#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cpp.h"

extern	int getopt(int, char *const *, const char *);
extern	char	*optarg, rcsid[];
extern	int	optind;
int	verbose;
int	Mflag;	/* only print active include files */
char	*objname; /* "src.$O: " */
int	Cplusplus = 1;

#if 1 /*@@@*/
#define NFORCEINC_FILE	32
static	char		*forceinc_file[NFORCEINC_FILE+1];
static	unsigned	forceinc_file_size;
static	unsigned	forceinc_file_cur;
#endif

void
setup(int argc, char **argv)
{
	int c, i;
	FILE *fd;
	char *fp, *dp;
	Tokenrow tr;
	extern void setup_kwtab(void);

	setup_kwtab();
  #if 1 /*@@@*/
	while ((c = getopt(argc, argv, "MNOVv+I:i:D:U:F:lg")) != -1)
  #else
	while ((c = getopt(argc, argv, "MNOVv+I:D:U:F:lg")) != -1)
  #endif
		switch (c) {
		case 'N':
			for (i=0; i<NINCLUDE; i++)
				if (includelist[i].always==1)
					includelist[i].deleted = 1;
			break;
		case 'I':
			for (i=NINCLUDE-2; i>=0; i--) {
				if (includelist[i].file==NULL) {
					includelist[i].always = 1;
					includelist[i].file = optarg;
					break;
				}
			}
			if (i<0)
				error(FATAL, "Too many -I directives");
			break;
	  #if 1 /*@@@*/
		case 'i':
			if (forceinc_file_size < NFORCEINC_FILE)
				forceinc_file[forceinc_file_size++] = (char*)newstring((uchar*)optarg, strlen(optarg), 0);
			break;
	  #endif
		case 'D':
		case 'U':
			setsource("<cmdarg>", NULL, optarg);
			maketokenrow(3, &tr);
			gettokens(&tr, 1);
			doadefine(&tr, c);
			unsetsource();
			break;
		case 'M':
			Mflag++;
			break;
		case 'v':
			fprintf(stderr, "%s %s\n", argv[0], rcsid);
			break;
		case 'V':
			verbose++;
			break;
		case '+':
			Cplusplus++;
			break;
		default:
			break;
		}
	dp = ".";
	fp = "<stdin>";
	fd = stdin;
	if (optind<argc) {
	 #if 0	/*@@@*/ /* change to push_userinclist(name), pop_userinclist() */
		if ((fp = strrchr(argv[optind], '/')) != NULL) {
			int len = fp - argv[optind];
			dp = (char*)newstring((uchar*)argv[optind], len+1, 0);
			dp[len] = '\0';
		}
	 #endif
		fp = (char*)newstring((uchar*)argv[optind], strlen(argv[optind]), 0);
		if ((fd = fopen(fp, "r")) == NULL)
			error(FATAL, "Can't open input file %s", fp);
	}
	if (optind+1<argc) {
		FILE *fdo = freopen(argv[optind+1], "w", stdout);
		if (fdo == NULL)
			error(FATAL, "Can't open output file %s", argv[optind+1]);
	}
	if(Mflag)
		setobjname(fp);
	includelist[NINCLUDE-1].always = 0;
	includelist[NINCLUDE-1].file = dp;
	setsource(fp, fd, NULL);
}


#if 1 /*@@@*/
int
forceinc_file_begin(void)
{
	char* fp = forceinc_file[ forceinc_file_cur ];
	if (fp) {
		FILE *fd;
		if ((fd = fopen(fp, "r")) == NULL) {
			error(FATAL, "Can't open -i file %s", fp);
		}
		setsource(fp, fd, NULL);
		++forceinc_file_cur;
	}
	return fp != 0;
}

void
forceinc_file_end(void)
{
	unsetsource();
}
#endif


#ifndef _WIN32	/*#ifndef _MSC_VER		//@@@ */
/* memmove is defined here because some vendors don't provide it at
   all and others do a terrible job (like calling malloc) */
void *
memmove(void *dp, const void *sp, size_t n)
{
	unsigned char *cdp, *csp;

	if (n==0)
		return 0;
	cdp = dp;
	csp = (unsigned char *)sp;
	if (cdp < csp) {
		do {
			*cdp++ = *csp++;
		} while (--n);
	} else {
		cdp += n;
		csp += n;
		do {
			*--cdp = *--csp;
		} while (--n);
	}
	return 0;
}
#endif

