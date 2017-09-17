#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpp.h"
#ifdef WIN32	/*@@@*/
#include <ctype.h>
#endif

Includelist	includelist[NINCLUDE];

extern char	*objname;

#if 1 /*@@@*/
static Includelist	userinclist[NINCLUDE];
static int			userincOfs = NINCLUDE;
#endif

#if 1 /*@@@*/
static int add_dir_file(char* path, unsigned size, const char* dir, const char* fname);
#endif

void
doinclude(Tokenrow *trp)
{
  #if 1 /*@@@*/
	char fname[PATH_BUF_SIZE], iname[PATH_BUF_SIZE];
  #else
	char fname[256], iname[256];
  #endif
	Includelist *ip;
	int angled, len, i;
	FILE *fd;

	trp->tp += 1;
	if (trp->tp>=trp->lp)
		goto syntax;
	if (trp->tp->type!=STRING && trp->tp->type!=LT) {
		len = trp->tp - trp->bp;
		expandrow(trp, "<include>");
		trp->tp = trp->bp+len;
	}
	if (trp->tp->type==STRING) {
		len = trp->tp->len-2;
		if (len > sizeof(fname) - 1)
			len = sizeof(fname) - 1;
		strncpy(fname, (char*)trp->tp->t+1, len);
		angled = 0;
	} else if (trp->tp->type==LT) {
		len = 0;
		trp->tp++;
		while (trp->tp->type!=GT) {
			if (trp->tp>trp->lp || len+trp->tp->len+2 >= sizeof(fname))
				goto syntax;
			strncpy(fname+len, (char*)trp->tp->t, trp->tp->len);
			len += trp->tp->len;
			trp->tp++;
		}
		angled = 1;
	} else
		goto syntax;
	trp->tp += 2;
	if (trp->tp < trp->lp || len==0)
		goto syntax;
	fname[len] = '\0';
	if (fname[0]=='/'
	 #ifdef WIN32	/*@@@*/
		|| fname[0] == '\\' || (isalpha(fname[0]) && fname[1] == ':')
	 #endif
	) {
		fd = fopen(fname, "r");
		strcpy(iname, fname);
	} else {
	 #if 1	/*@@@*/
		fd = NULL;
		if (!angled) {
			for (i = userincOfs; i < NINCLUDE; ++i) {
				ip = &userinclist[i];
				if (ip->file) {
					if (add_dir_file(iname, PATH_BUF_SIZE, ip->file, fname))
						fd = fopen(iname, "r");
					break;
				}
			}
		}
		if (!fd)
	 #endif
		{
			for (fd=NULL,i=NINCLUDE-1; i>=0; i--) {
				ip = &includelist[i];
				if (ip->file==NULL || ip->deleted || (angled && ip->always==0))
					continue;
			  #if 1	/*@@@*/
				if (add_dir_file(iname, PATH_BUF_SIZE, ip->file, fname) == 0)
					continue;
			  #else
				if (strlen(fname)+strlen(ip->file)+2 > sizeof(iname))
					continue;
				strcpy(iname, ip->file);
				strcat(iname, "/");
				strcat(iname, fname);
			  #endif
				if ((fd = fopen(iname, "r")) != NULL) {
					break;
				}
			}
		}
	}
	if ( Mflag>1 || !angled&&Mflag==1 ) {
		fwrite(objname,1,strlen(objname),stdout);
		fwrite(iname,1,strlen(iname),stdout);
		fwrite("\n",1,1,stdout);
	}
	if (fd != NULL) {
		if (++incdepth > 10)
			error(FATAL, "#include too deeply nested");
		setsource((char*)newstring((uchar*)iname, strlen(iname), 0), fd, NULL);
		genline();
	} else {
		trp->tp = trp->bp+2;
		error(ERROR, "Could not find include file %r", trp);
	}
	return;
syntax:
	error(ERROR, "Syntax error in #include");
	return;
}

/*
 * Generate a line directive for cursource
 */
void
genline(void)
{
	static Token ta = { UNCLASS };
	static Tokenrow tr = { &ta, &ta, &ta+1, 1 };
	uchar *p;

	ta.t = p = (uchar*)out_p;
	strcpy((char*)p, "#line ");
	p += sizeof("#line ")-1;
	p = (uchar*)outnum((char*)p, cursource->line);
	*p++ = ' '; *p++ = '"';
	strcpy((char*)p, cursource->filename);
	p += strlen((char*)p);
	*p++ = '"'; *p++ = '\n';
	ta.len = (char*)p-out_p;
	out_p = (char*)p;
	tr.tp = tr.bp;
	puttokens(&tr);
}

void
setobjname(char *f)
{
	int n = strlen(f);
	objname = (char*)domalloc(n+5);
	strcpy(objname,f);
	if(objname[n-2]=='.'){
		strcpy(objname+n-1,"$O: ");
	}else{
		strcpy(objname+n,"$O: ");
	}
}

#if 1	/*@@@*/

static char* get_basename(const char* name)
{
	const char *p = name;
	while (*p != '\0') {
	 #ifdef _WIN32
		if (*p == '/' || *p == '\\' || *p == ':')
	 #else
		if (*p == '/')
	 #endif
			name = p + 1;
		++p;
	}
	return (char*)name;
}

void push_userinclist(const char* fname)
{
	Includelist *u;
	unsigned	l;
	if (userincOfs <= 0)
		error(FATAL, "#include too deeply nested");
	--userincOfs;
	u = &userinclist[userincOfs];
	u->always = 0;
	u->file   = NULL;
	if (fname && fname[0] && fname[0] != '<') {
		char *b;
		l = strlen(fname);
		if ((b = get_basename(fname)) != fname) {
			l = b - fname;
		}
		u->file = (char*)newstring((uchar*)fname, l, 0);
	}
}

void pop_userinclist(void)
{
	Includelist *u;
	if (userincOfs < NINCLUDE) {
		u = &userinclist[userincOfs];
		if (u->file)
			dofree(u->file);
		u->always = 0;
		u->file = NULL;
		++userincOfs;
	}
}

#endif

#if 1	/*@@@*/

static int
add_dir_file(char* path, unsigned size, const char* dir, const char* fname)
{
 #ifdef WIN32
	char   buf[PATH_BUF_SIZE+1];
	size_t l = strlen(dir);
	size_t n = strlen(fname);
	if (l+n+2 >= PATH_BUF_SIZE)
		return 0;
	memcpy(buf, dir, l);
	buf[l++] = '/';
	buf[l]	 = 0;
	memcpy(buf+l, fname, n+1);
	_fullpath(path, buf, size);
	path[size-1] = 0;
	return 1;
 #elif defined __linux__ || defined __GLIBC__
	char	buf[PATH_BUF_SIZE+1];
	char	*p;
	size_t l = strlen(dir);
	size_t n = strlen(fname);
	if (l+n+2 >= PATH_BUF_SIZE)
		return 0;
	memcpy(buf, dir, l);
	buf[l++] = '/';
	memcpy(buf+l, fname, n+1);
	p = realpath(NULL, buf);
	strncpy(path, p, size);
	free(p);
	path[size-1] = '\0';
	return 1;
 #else	/* tenuki */
	const char	*s = fname;
	char		*t;
	char		*p;
	size_t		n;
	size_t		l = strlen(dir);
	if (l > 0 && dir[l-1] == '/')
		--l;
	t = memcpy(path, dir, l);
	t[l] = 0;
	for (;;) {
		if (strncmp(t, "./", 2) == 0)
			t+= 2;
		/*else if (strncmp(t, "../", 3) == 0)
			t+= 3;*/
		else
			break;
	}
	for (;;) {
		if (strncmp(s, "./", 2) == 0) {
			s += 2;
			continue;
		}
		if (strncmp(s, "../", 3) == 0) {
			s += 3;
			p = strrchr(t, '/');
			if (p <= t)
				goto ERR;
			*p = '\0';
			continue;
		}
		break;
	}
	l = strlen(path);
	if (l + 1 >= size)
		goto ERR;
	path[l++] = '/';
	path[l]   = 0;
	n = strlen(s);
	if (l+n >= size)
		goto ERR;
	memcpy(path+l, s, n+1);
	return 1;

  ERR:
	path[0] = 0;
	return 0;
 #endif
}

#endif
