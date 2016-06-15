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
    	int letter = c+n;
	    while ((c > 64 && c < 91) && letter > 90) {
    		letter = letter - 26;
    	}
    	while ((c > 96 && c < 123) && letter > 122) {
    		letter = letter - 26;
    	}
    	if ((c < 65) || (c > 90 && c < 97) || (c > 122)) {
    		letter = c;
    	}
    	putchar(letter);
	}
    return(0);
}
