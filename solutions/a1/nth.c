#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int n;
    if (argc != 2 || (n = atoi(argv[1])) < 0) {
	fprintf(stderr, "usage: nth fieldnum\n");
	return(1);
    }

    int c, state = 0, i = 1;
    while ((c = getchar()) != EOF) {
	if (c == '\n') {
	    /* this logic is common to all states, so put it here */
	    putchar('\n');
	    i = 1;
	    state = 0;
	} else {
	    switch (state) {
	    case 0:
		if (c == ' ' || c == '\t')
		    break;
		if (i < n) {
		    state = 1;
		    break;
		}
		putchar(c);
		state = 2;
		break;
	    case 1:
		if (c == ' ' || c == '\t') {
		    i += 1;
		    state = 0;
		    break;
		}
		break;
	    case 2:
		if (c == ' ' || c == '\t') {
		    state = 3;
		    break;
		}
		putchar(c);
		break;
	    case 3:
		break;
	    }
	}
    }

    return(0);
}
