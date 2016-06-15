#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int n;
    if (argc != 2 || (n = atoi(argv[1])) < 0) {
        fprintf(stderr, "usage: nth n\n");
        return(1);
    }
	
	int state = 0;
	int i = 1;
	int c;
	
	while ((c = getchar()) != EOF && state <= 3)
	{
    	switch (state)
    	{
    		case 0:
    			if (c == '\t' || c == 32) {
    				state = 0;
    			}
    			else if (c == '\n') {
    				putchar(c);
    				i = 1;
    				state = 0;
    			}
    			else if (i < n) {
    				state++;
    			}
    			else {
    				putchar(c);
    				state += 2;
    			}
    			break;
	    	case 1:  
    			if (c == '\t' || c == 32) {
    				i++;
    				state--;
	    		}    
    			else if (c == '\n') {
    				putchar(c);
    				i = 1;
    				state--;
	    		}
	    		else {
	    			state = 1;
	    		}
    			break;
    		case 2:  
	    		if (c == '\t' || c == 32) {
    				state++;
    			}    
	    		else if (c == '\n') {
	    			putchar(c);
	    			i = 1;
    				state -= 2;
    			}
    			else {
    				putchar(c);
    				state = 2;
    			}
	    		break;
	    	case 3:
	    		if (c == '\n') {
	    			putchar(c);
	    			i = 1;
	    			state -= 3;
	    		}
	    		else {
	    			state = 3;
	    		}
    	default:
    			break;
    		}
    	}
    	return(0);
    }
    	
