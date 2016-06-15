#include <stdio.h>
#include <unistd.h>
#include <string.h>


static int cflag = 0;

static int count = 0;
static char buf[2][500];
static int which;
extern void show();  /* output the line we're working on because either we
			have a non-matching one or we are exiting */


int main(int argc, char **argv)
{
    int c, status = 0;
    FILE *fp;
    extern void process(FILE *fp);

    while ((c = getopt(argc, argv, "c")) != EOF) {
	switch (c) {
	case 'c':
	    cflag = 1;
	    break;
	case '?':
	default:
	    status = 1;
	    break;
	}
    }

    if (status) {
	fprintf(stderr, "usage: %s [-c] [file ...]\n", argv[0]);
	return(1);
    }

    if (optind >= argc) {
	process(stdin);
    } else {
	for (; optind < argc; optind++) {
	    if (strcmp(argv[optind], "-") == 0) {
		process(stdin);
	    } else {
		if ((fp = fopen(argv[optind], "r")) == NULL) {
		    perror(argv[optind]);
		    status = 1;
		} else {
		    process(fp);
		    fclose(fp);
		}
	    }
	}
    }
    if (count)
	show();
    return(status);
}


void process(FILE *fp)
{
    /* If we haven't read any lines at all yet, that's a special case. */
    if (count == 0) {
	if (fgets(buf[0], sizeof buf[0], fp) == NULL)
	    return;
	count = 1;
	which = 0;
    }

    while (fgets(buf[!which], sizeof buf[0], fp)) {
	if (strcmp(buf[0], buf[1]) == 0) {
	    count++;
	} else {
	    show();
	    which = !which;
	    count = 1;
	}
    }
}


/*
 * Output the line we're working on because either we have a non-matching one
 * or we are exiting
 */
void show()
{
    if (cflag)
	printf("%d ", count);
    printf("%s", buf[which]);
}
