#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

static int sflag = 0, hflag = 0;
static int status = 0;  /* i.e. anyone can cause us to return error status */

static long du(char *dirname);
static void show(char *dirname, long space);


int main(int argc, char **argv)
{
    int c;

    while ((c = getopt(argc, argv, "sh")) != EOF) {
	switch (c) {
	case 's':
	    sflag = 1;
	    break;
	case 'h':
	    hflag = 1;
	    break;
	case '?':
	default:
	    status = 1;
	    break;
	}
    }

    if (status || optind >= argc) {
	fprintf(stderr, "usage: %s [-sh] dir ...\n", argv[0]);
	return(1);
    }

    for (; optind < argc; optind++)
	show(argv[optind], du(argv[optind]));

    return(status);
}


static long du(char *dirname)
{
    DIR *dp;
    struct dirent *r;
    char subpath[2000];
    long subvalue, sum = 0;
    struct stat statbuf;

    if ((dp = opendir(dirname)) == NULL) {
	perror(dirname);
	status = 1;
	return(-1);
    }
    while ((r = readdir(dp))) {
	if (strcmp(r->d_name, ".") == 0
		|| strcmp(r->d_name, "..") == 0) {
	    continue;
	}
	if (strlen(dirname) + 1 + strlen(r->d_name) + 1 > sizeof subpath) {
	    fprintf(stderr, "%s/%s: path name too long\n", dirname, r->d_name);
	    status = 1;
	    continue;
	}
	sprintf(subpath, "%s/%s", dirname, r->d_name);
	if (lstat(subpath, &statbuf)) {
	    perror(subpath);
	    status = 1;
	} else if (S_ISDIR(statbuf.st_mode)) {
	    subvalue = du(subpath);
	    if (subvalue >= 0) {
		if (!sflag)
		    show(subpath, subvalue);
		sum += subvalue;
	    }
	} else {
	    sum += statbuf.st_blocks;
	}
    }
    closedir(dp);
    return(sum);
}


static void show(char *dirname, long space)
{
    if (space >= 0) {  /* -1 means error */
	if (!hflag)
	    printf("%ld", (space + 1) / 2);
	else if (space >= 2048000)
	    printf("%ldG", (space + 1024000) / 2048000);
	else if (space >= 2048)
	    printf("%ldM", (space + 1024) / 2048);
	else
	    printf("%ldK", (space + 1) / 2);
	printf(" %s\n", dirname);
    }
}
