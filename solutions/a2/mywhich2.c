#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


int main(int argc, char **argv)
{
    int status = 0;
    extern int search(char *command, char *prepend);

    if (argc < 2) {
	fprintf(stderr, "usage: %s commandname ...\n", argv[0]);
	return(1);
    }
    for (argc--, argv++; argc > 0; argc--, argv++) {
	if (search(*argv, "/bin/") && search(*argv, "/usr/bin/")
		&& search(*argv, "")) {
	    fprintf(stderr, "%s: Command not found.\n", *argv);
	    status = 1;
	}
    }
    return(status);
}


int search(char *command, char *prepend)
{
    static char *buf = NULL;
    static int bufsize = 0;
    int sizeneeded = strlen(prepend) + strlen(command) + 1;
    struct stat statbuf;

    if (sizeneeded > bufsize) {
	if (buf == NULL) {
	    bufsize = 500;
	} else {
	    bufsize = sizeneeded + 100;
	    free(buf);
	}
	if ((buf = malloc(bufsize)) == NULL) {
	    fprintf(stderr, "out of memory!\n");  /* very very unlikely */
	    exit(1);
	}
    }
    strcpy(buf, prepend);
    strcat(buf, command);
    if (stat(buf, &statbuf) || (statbuf.st_mode & 0100) == 0)
	return(-1);
    printf("%s\n", buf);
    return(0);
}
