#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int n;
	if (argc != 2 || atoi(argv[1]) > 46) {
		fprintf(stderr, "usage: fib n\n");
		return(1);
	}
    
    n = atoi(argv[1]);
    int fib[47];
    fib[0] = 0;
    fib[1] = 1;
    
    for (int i=2; i<=47; i++) {
    	fib[i] = fib[i-1] + fib[i-2];
    }
    
    printf("%d\n",fib[n]);
    return(0);
}
