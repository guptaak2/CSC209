#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int n;
    if (argc != 2 || (n = atoi(argv[1])) < 0) {
	fprintf(stderr, "usage: rot n\n");
	return(1);
    }

    int c;

    while ((c = getchar()) != EOF) {
	if (c >= 'a' && c <= 'z') {
	    c += n;
	    while (c > 'z')
		c -= 26;
	} else if (c >= 'A' && c <= 'Z') {
	    c += n;
	    while (c > 'Z')
		c -= 26;
	}
	putchar(c);
    }

    return(0);
}
