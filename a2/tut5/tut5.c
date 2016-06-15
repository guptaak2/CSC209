/*
 * Illustrating various stuff about calling stat().
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char **argv)
{
    struct stat path1, path2;
    int i;

    if (argc != 3) {
		fprintf(stderr, "usage: %s file_path_1 file_path_2\n", argv[0]);
		return(1);
    }
	
	for (i = 1; i < argc; i++) {
	if (stat(argv[i], &path1)) {
	    perror(argv[i]);
	    return(1);    
	}
	else if (stat(argv[i], &path2)) {
		perror(argv[i]);
		return(1);
	}
	}

	if (difftime(path1.st_mtime, path2.st_mtime) >= 0) {
		printf("%s\n", argv[1]);
	}
	else {
		printf("%s\n", argv[2]);
	}
	return(0);
}


