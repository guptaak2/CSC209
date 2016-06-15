/*
 * fsh.c - the Feeble SHell.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fsh.h"
#include "builtin.h"


int builtin_cd(char **argv)
{
    if (argv[1] && argv[2]) /* i.e. argc >= 2 */ {
	fprintf(stderr, "usage: cd [dir]\n");
	fflush(stderr);
	return(1);
    } else if (argv[1]) {
	/* "cd dir" */
	if (chdir(argv[1])) {
	    perror(argv[1]);
	    fflush(stderr);
	    return(1);
	}
	return(0);
    } else {
	/* "cd" with no argument */
	char *p = getenv("HOME");
	if (p == NULL) {
	    fprintf(stderr, "cd: HOME environment variable not set\n");
	    fflush(stderr);
	    return(1);
	}
	if (chdir(p)) {
	    perror(p);
	    fflush(stderr);
	    return(1);
	}
	return(0);
    }
}


int builtin_exit(char **argv)
{
    if (argv[1] && argv[2]) /* i.e. argc >= 2 */ {
	fprintf(stderr, "usage: exit [status]\n");
	fflush(stderr);
	return(1);
    } else if (argv[1]) {
	/* "exit ###" */
	exit(atoi(argv[1]));
    } else {
	/* "exit" with no argument */
	exit(laststatus);
    }
}
