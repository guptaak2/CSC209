/* not part of fsh; here to help with part 8 of the assignment */

#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
	fprintf(stderr, "usage: %s dir\n", argv[0]);
	return(1);
    }
    if (chdir(argv[1])) {
	perror(argv[1]);
	return(1);
    }
    return(0);
}
