#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

int main() {

	DIR *dp;
	int inode;
	struct stat statbuf;
	struct dirent *p;

	if (lstat(("."), &statbuf)) {
		perror(".");
		return(1);
	}
	inode = (int)statbuf.st_ino;
	while (statbuf.st_ino != 2) {
		chdir("..");
		if ((dp = opendir(".")) == NULL) {
			perror(".");
			return(1);
		} 
		while ((p = readdir(dp))) {
			if (statbuf.st_ino == p->d_ino) {
				printf("%s\n", p->d_name);
			}
		}
		inode = (int)statbuf.st_ino;
		if (lstat(("."), &statbuf)) {
			perror(".");
			return(1);
		}
		closedir(dp);
	}
	if (statbuf.st_ino == 2) {
		printf("and then we're at the root directory\n");
	} else {
		printf("Can't find %d in parent!\n", inode);
	}
	return(0);
}
