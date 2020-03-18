/*
 *	$Id: HZutil.c,v 1.2 2003/05/06 07:40:19 htl10 Exp $
 */

/***********************************************************
Copyright 1994,1995 by Yongguang Zhang.  All Rights Reserved

Permission to retain, use, modify, copy, and distribute CXTERM 5.0
in source or binary and its documentation (hereafter, the Software)
for non-commercial purpose is hereby granted to you without a fee,
provided that this entire copyright and permission notice appear in
all such copies, that no charge be associated with such copies,
that distribution of derivative works (including value-added
distributions such as with additional input dictionaries or fonts)
include clarification that such added or derived parts are not from
the original Software, and that the names of the author(s) not be
used to endorse or promote such works.

THE AUTHOR(S) DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/*
 * All utilities routines and miscellaneous routines.
 */

#include "HZtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static struct codetbl {
	char *codestr;
	int   codeint;
} codeTbl[] = {

	{"GB",		GB_ENCODE},
	{"Gb",		GB_ENCODE},
	{"gb",		GB_ENCODE},
	{"GUOBIAO",	GB_ENCODE},
	{"GuoBiao",	GB_ENCODE},
	{"guobiao",	GB_ENCODE},

	{"BIG5",		BIG5_ENCODE},
	{"Big5",		BIG5_ENCODE},
	{"big5",		BIG5_ENCODE},
	{"B5",		BIG5_ENCODE},
	{"b5",		BIG5_ENCODE},

	{"JIS",		JIS_ENCODE},
	{"jis",		JIS_ENCODE},
	{"J",		JIS_ENCODE},
	{"j",		JIS_ENCODE},

	{"KS",		KS_ENCODE},
	{"ks",		KS_ENCODE},
	{"KSC",		KS_ENCODE},
	{"ksc",		KS_ENCODE},

	{(char *)0,	UNKNOWN_ENCODE}
};


extern int access (const char *, int);
extern void free (void *);
extern void *malloc (size_t);

int HZencode (char *name)
{
    struct codetbl *pct = codeTbl;

    while (pct->codestr) {
	if (strcmp (name, pct->codestr) == 0)
	    return (pct->codeint);
	pct++;
    }
    return (pct->codeint);
}

char *HZencodeName (int encode)
{
    struct codetbl *pct = codeTbl;

    while (pct->codestr) {
	if (encode == pct->codeint)
	    return (pct->codestr);
	pct++;
    }
    return (pct->codestr);
}

/*
 * find a file of given "name" from the list of "inputdir",
 * the expanded pathname is written in "filename".
 */
int HZfindfile(char *name, char *inputdir, char *filename)
{
#ifndef R_OK
#define	R_OK 4
#endif

    if (name[0] == '/') {
	/* try root directory first */
	strcpy (filename, name);
	if (access (filename, R_OK) == 0)
	    return(1);
    }

    /* search name in different dirs */
    if (inputdir) {
	register char *dir = inputdir;
	register char *pfilename;

	while (*dir) {
	    /* copy from (dir) to (filename), till ':' or end of string */
	    pfilename = filename;
	    while ((*dir != '\0') && (*dir != ':'))
		*pfilename++ = *dir++ ;
	    *pfilename = '\0';

	    strcat (filename, "/");
	    strcat (filename, name);
	    if (access (filename, R_OK) == 0)
		return(1);

	    if (*dir == ':')
		dir++ ;	/* skip this ':', ready for next component dir */
	}
    }

    if (name[0] != '/') {
	/* try current directory */
	strcpy (filename, name);
	if (access (filename, R_OK) == 0)
	    return(1);
    }

    return(0);	/* found no file */
}

/*
 * Return all files of given "suffix" from the list of "inputdir",
 * the basename is written in "basename".  To start a new search,
 * give non-empty "name" and "inputdir"; to get the next match,
 * give NULL as "name" and "inputdir".  When no more match for this
 * search, the function returns 0 (otherwise, 1 means successful).
 */
int HZfindsuffix(char *new_suffix, char *new_inputdir, char *basename)
{

#if !defined(X_NOT_POSIX) || defined(SYSV) || defined(USG)
# include <dirent.h>
#else
# include <sys/dir.h>
# ifndef dirent
#  define dirent direct
# endif 
#endif      

  /* persistent variables for the search continuation */
  static char *inputdir = NULL;
  static char *suffix = NULL;
  static int suffix_len = 0;
  static char *next_dirp;
  static DIR *pdir = NULL;

  struct dirent *entry;

    if (new_suffix && new_inputdir) {
	/* clean up the previous search thread, if any */
	if (pdir)
	    closedir(pdir);
	pdir = NULL;
	if (inputdir)
	    free(inputdir);

	inputdir = (char *)malloc( strlen(new_inputdir) + 1 );
	strcpy(inputdir, new_inputdir);
	suffix = new_suffix;
	suffix_len = strlen(new_suffix);

	next_dirp = inputdir;
    }

    if (!inputdir || !suffix)
	return 0;	/* nothing to search for */

    while (1) {

	/* need to open a new directory? */
	while (! pdir) {
	    char *dirname = next_dirp;

	    if (! dirname) {
		/* we've tried everything and we are done */
		free(inputdir);
		inputdir = NULL;
		return(0);
	    }

	    /* get to the next directory name delimited by ':' */
	    next_dirp = (char *)strchr(dirname, ':');
	    if (next_dirp)
	        *next_dirp++ = '\0';	/* null-terminate the dirname */

	    pdir = opendir(dirname);
	}

	/* read next dir */
	while ((entry = readdir(pdir)) != NULL) {
	    char *filename = entry->d_name;
	    int baselen = strlen(filename) - suffix_len;

	    if ((baselen <= 0) || (strcmp(filename + baselen, suffix) != 0))
		continue;	/* not this entry */

	    /* this is what we are looking for */
	    strncpy(basename, filename, baselen);
	    basename[baselen] = '\0';
	    return 1;
	}

	/* done with this directory */
	closedir(pdir);
	pdir = NULL;
    }
}

int HZgetprompt (char *buffer, char *strbuf)
{
#define	is7space(c)	(isspace(c) && (!((c) & 0x80)))

    while (isspace (*buffer))  buffer++;
    while (*buffer && (! isspace(*buffer)))  buffer++;	/* skip 1st field */
    while (is7space (*buffer))  buffer++;
    if (! *buffer) {
	*strbuf = '\0';
	return (0);
    }
    while (*buffer && (! is7space(*buffer)))
	*strbuf++ = *buffer++;
    *strbuf = '\0';
    return(1);

#undef	is7space
}

