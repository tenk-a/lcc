/* x86s running MS Windows NT 4.0 */

#if 0	/*@@@ old */
#include <string.h>

static char rcsid[] = "$Id$";

#ifndef LCCDIR
#define LCCDIR "\\progra~1\\lcc\\4.1\\bin\\"
#endif

char *suffixes[] = { ".c;.C", ".i;.I", ".asm;.ASM;.s;.S", ".obj;.OBJ", ".exe", 0 };
char inputs[256] = "";
char *cpp[] = { LCCDIR "cpp", "-D__STDC__=1", "-Dwin32", "-D_WIN32", "-D_M_IX86",
	"$1", "$2", "$3", 0 };
char *include[] = { "-I" LCCDIR "include", 0 };
char *com[] = { LCCDIR "rcc", "-target=x86/win32", "$1", "$2", "$3", 0 };
char *as[] = { "ml", "-nologo", "-c", "-Cp", "-coff", "-Fo$3", "$1", "$2", 0 };
char *ld[] = { "link", "-nologo", 
	"", "-subsystem:console", "-entry:mainCRTStartup",
	"$2", "-OUT:$3", "$1", LCCDIR "liblcc.lib", "oldnames.lib", "libc.lib", "kernel32.lib", 0 };

extern char *concat(char *, char *);
extern char *replace(const char *, int, int);

int option(char *arg) {
	if (strncmp(arg, "-lccdir=", 8) == 0) {
		arg = replace(arg + 8, '/', '\\');
		if (arg[strlen(arg)-1] == '\\')
			arg[strlen(arg)-1] = '\0';
		cpp[0] = concat(arg, "\\cpp.exe");
		include[0] = concat("-I", concat(arg, "\\include"));
		com[0] = concat(arg, "\\rcc.exe");
		ld[8] = concat(arg, "\\liblcc.lib");
	} else if (strcmp(arg, "-b") == 0)
		;
	else if (strncmp(arg, "-ld=", 4) == 0)
		ld[0] = &arg[4];
	else
		return 0;
	return 1;
}

#else	/*@@@ read lcc.cfg file. */

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define CFGARY_SIZE		32

char *suffixes[] = { ".c;.C", ".i;.I", ".asm;.ASM;.s;.S", ".obj;.OBJ", ".exe", 0 };
char inputs[256] = "";
char *cpp[CFGARY_SIZE+1];
char *include[CFGARY_SIZE+1];
char *com[CFGARY_SIZE+1];
char *as[CFGARY_SIZE+1];
char *ld[CFGARY_SIZE+1];

#define LCCINSTDIR_SIZE		1024
char lccinstdir[LCCINSTDIR_SIZE];		/* lcc install directory. */
unsigned lccinst_parent_len;

void *alloc(int);
extern char *replace_direct(const char *, int, int);

static char* get_basename(const char* name) {
	const char *p = name;
	while (*p != '\0') {
		if (*p == ':' || *p == '/' || *p == '\\')
			name = p + 1;
		++p;
	}
	return (char*)name;
}

static char* skip_spc(char* s) {
	while ((0 < *s && *s <= 0x20) || *s == 0x7f)
		++s;
	return s;
}

static char *skip_arg1(char *s) {
	unsigned f;
	int		 c;
	if (s == 0)
		return NULL;

	if (*s == '\0')
		return s;

	f = 0;
	do {
		c = *s++;
		if (c == '"') {
			f ^= 1;
			if (*s == '"' && f == 0)
				++s;
			else
				continue;
		}
	} while (c >= 0x20 && (c != ' ' || f != 0));
	--s;
	return s;
}

static char* repl_instDir(char* str)
{
	char* s = strchr(str, '$');
	if (s && s[1] == '0') {
	 #if 1 /* has _fullpath() */
		size_t	l		= strlen(lccinstdir)+strlen(s+2)+4;
		char	*buf	= (char*)malloc(l * 2);
		char	*buf2	= buf + l;
		strcpy(buf2, lccinstdir);
		strcat(buf2, s+2);
		_fullpath(buf, buf2, l);
		buf2 = (char*) alloc(s-str + strlen(buf)+1);
		memcpy(buf2, str, s-str);
		strcpy(buf2+(s-str), buf);
		str = buf2;
		free(buf);
	 #else /* tiny */
		char  *a;
		size_t srcLen;
		size_t instLen;
		size_t len  = s - str;
		/* $0\..\ => lccinstdir parent. */
		if ((s[2] == '/' || s[2] == '\\') && s[3] == '.' && s[4] == '.'
		 && (s[5] == '/' || s[5] == '\\') )
		{
			instLen = lccinst_parent_len;
			s += 6;
		} else {
			instLen = strlen(lccinstdir);
			s += 2;
		}
		srcLen = strlen(s);
		a = (char*)alloc(len + instLen + srcLen + 1);
		if (len) {
			memcpy(a, str, len);
		}
		if (instLen) {
			memcpy(a+len, lccinstdir, instLen);
			len += instLen;
		}
		if (srcLen) {
			memcpy(a+len, s, srcLen);
			len += srcLen;
		}
		a[len] = 0;
		str  = a;
	 #endif
	}
	return str;
}

static void get_cfg_line(char* lst[], unsigned lst_num, const char* str) {
	unsigned l;
	unsigned n;
	char*	 s;
	char*	 e;

	l = strlen(str) + 1;
	s = (char*)alloc( l );
	if (s == NULL) {
		return;
	}
	memcpy(s, str, l);
	for (n = 0; n < lst_num-1; ++n) {
		s = skip_spc(s);
		e = skip_arg1(s);
		if (e > s) {
			*e = '\0';
			lst[n] = repl_instDir(s);
		} else {
			break;
		}
		s = e + 1;
	}
	lst[n] = NULL;
}

static int get_lccinstdir(const char* prgname, int mode)
{
	size_t l;
	char   *p;
	lccinstdir[0] = 0;
	if (prgname == 0)
		return 0;
	if (mode > 0) {	/* has exe name ? */
		p = get_basename(prgname);
		l = p - prgname;
	} else {
		l = strlen(prgname);
	}
	if (l > 0 && (prgname[l-1] == '\\' || prgname[l-1] == '/'))
		--l;
	if (l >= LCCINSTDIR_SIZE)
		l  = LCCINSTDIR_SIZE - 1;
	if (l > 0)
		memcpy(lccinstdir, prgname, l);
	lccinstdir[l] = 0;
	replace_direct(lccinstdir, '/', '\\');
	lccinst_parent_len = strlen(lccinstdir);
	p = get_basename(lccinstdir);
	if (p > lccinstdir) {
		lccinst_parent_len = p - lccinstdir;
	}
	return 0;
}

void read_cfgfile(const char* progname) {
	#define 	CFGNAME_SIZE	1024
	char 		cfgname[ CFGNAME_SIZE + 5 ];
	char 		buf[4096];
	char 		*p;
	unsigned	line;
	FILE*		fp;

	assert(progname != NULL);
  #if defined _MSC_VER || defined __MINGW32__ || defined __LCC_WITH_VC__
	progname = _pgmptr;
  #endif
	get_lccinstdir(progname, 1);
	strncpy(cfgname, progname, CFGNAME_SIZE);
	cfgname[CFGNAME_SIZE-1] = 0;
	if ((p = strrchr(get_basename(cfgname), '.')) == NULL) {
		p = cfgname + strlen(cfgname);
	}
	strcpy(p, ".cfg");
	fp = fopen(cfgname, "r");
	if (fp == NULL) {
		fprintf(stderr, "file '%s' not found.\n", cfgname);
		return;
	}
	line = 0;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		++line;
		p = skip_spc(buf);
		if (strncmp(p, "cpp=", 4) == 0) {
			get_cfg_line(cpp, CFGARY_SIZE, p+4);
		} else if (strncmp(p, "inc=", 4) == 0) {
			get_cfg_line(include, CFGARY_SIZE, p+4);
		} else if (strncmp(p, "com=", 4) == 0) {
			get_cfg_line(com, CFGARY_SIZE, p+4);
		} else if (strncmp(p, "asm=", 4) == 0) {
			get_cfg_line(as , CFGARY_SIZE, p+4);
		} else if (strncmp(p, "lnk=", 4) == 0) {
			get_cfg_line(ld , CFGARY_SIZE, p+4);
		} else if (p[0] == ';' || p[0] == '#' || p[0] == 0) {
			;
		} else {
			fprintf(stderr, "%s (%d):unkown line.\n", cfgname, line);
			exit(1);
		}
	}
	fclose(fp);
	#undef CFGNAME_SIZE
}

int option(char *arg) {
	if (strncmp(arg, "-lccinst=", 9) == 0) {
		get_lccinstdir(arg+9, -1);
	} else if (strncmp(arg, "-lccdir=", 8) == 0) {
		if (get_lccinstdir(arg+8, 0) == 0)
			com[0] = "$0\rcc.exe";
	} else if (strcmp(arg, "-b") == 0) {
		if (get_lccinstdir(arg+2, 0) == 0)
			com[0] = "$0\rcc.exe";
	} else if (strncmp(arg, "-ld=", 4) == 0) {
		ld[0] = &arg[4];
	} else {
		return 0;
	}
	return 1;
}

#endif
