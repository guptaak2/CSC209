#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s string\n", argv[0]);
		return(1);
	}
	
	char *p1 = argv[1];
	char *p2 = argv[1] + strlen(argv[1]) - 1;	
	
	while (p2 > p1) {
		if (tolower(*p1) != tolower(*p2)) {
			return(1);
		}
		else if (isalnum(*p1) == 0 || isalnum(*p2) == 0) {
			p1++;
			p2--;
		}
	return(0);
	}
}
