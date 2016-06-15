/*
 * Produce all permutations of argv.
 */

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    extern void permute(char *prefix, int count, char **words);
    if (argc > 1)
	permute("", argc - 1, argv + 1);
    return(0);
}

void permute(char *prefix, int count, char **words)  /* precond: count >= 1 */
{
    if (count == 1) {
	printf("%s%s\n", prefix, *words);
    } else {
	int i, size, pos;
	char *newwords[count];
	for (size = i = 0; i < count; size += strlen(words[i]) + 1, i++)
	    newwords[i] = words[i];
	pos = strlen(prefix);
	char newprefix[pos + size + 1];
	strcpy(newprefix, prefix);
	for (i = 0; i < count; i++) {
	    strcpy(newprefix + pos, words[i]);
	    strcat(newprefix, " ");
	    newwords[i] = words[0];
	    permute(newprefix, count - 1, newwords + 1);
	    newwords[i] = words[i];
	}
    }
}
