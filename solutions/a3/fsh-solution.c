/*
 * fsh.c - the Feeble SHell.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "fsh.h"
#include "parse.h"
#include "builtin.h"
#include "error.h"

int showprompt = 1;
int laststatus = 0;  /* set by anything which runs a command */


int main()
{
    char buf[1000];
    struct parsed_line *p;
    extern void execute(struct parsed_line *p);

    while (1) {
	if (showprompt)
	    printf("$ ");
	if (fgets(buf, sizeof buf, stdin) == NULL)
	    break;
	if ((p = parse(buf))) {
	    execute(p);
	    freeparse(p);
        }
    }

    return(laststatus);
}


void execute(struct parsed_line *p)
{
    int status;
    extern void execute_one_subcommand(struct parsed_line *p);

    for (; p; p = p->next) {
	if (p->conntype == CONN_OR && laststatus == 0) {
	    /* last command succeeded, so don't do this one */
	} else if (p->conntype == CONN_AND && laststatus) {
	    /* last command failed, so don't do this one */
	} else {
	    /*
	     * "exit" and "cd" are handled specially to avoid the fork().
	     * Ideally the check should be later to make i/o redirection and
	     * piping work, but we'd have to fudge the forking that way.
	     */
	    if (p->pl && strcmp(p->pl->argv[0], "exit") == 0) {
		laststatus = builtin_exit(p->pl->argv);
	    } else if (p->pl && strcmp(p->pl->argv[0], "cd") == 0) {
		laststatus = builtin_cd(p->pl->argv);
	    } else {
		fflush(stdout);
		switch (fork()) {
		case -1:
		    perror("fork");
		    laststatus = 127;
		    break;
		case 0:
		    /* child */
		    execute_one_subcommand(p);
		    break;
		default:
		    /* parent */
		    wait(&status);
		    laststatus = status >> 8;
		}
	    }
	}
    }
}


/*
 * execute_one_subcommand():
 * Do file redirections if applicable, then call execute_pipeline()
 * which recursively executes the entire pipeline.
 * Does not return, so you want to fork() before calling me.
 */
void execute_one_subcommand(struct parsed_line *p)
{
    extern void execute_pipeline(struct pipeline *pl);
    if (p->inputfile) {
	close(0);
	if (open(p->inputfile, O_RDONLY, 0) < 0) {
	    perror(p->inputfile);
	    exit(126);
	}
    }
    if (p->outputfile) {
	close(1);
	if (open(p->outputfile, O_WRONLY|O_CREAT|O_TRUNC, 0666) < 0) {
	    perror(p->outputfile);
	    exit(126);
	}
    }
    if (p->pl)
	execute_pipeline(p->pl);
    /* otherwise, we have a null command, which should just exit successfully */
    exit(0);
}


/*
 * execute_pipeline(): recursively execute a (struct pipeline *).
 * pl must be non-null.
 * Does not return.
 */
void execute_pipeline(struct pipeline *pl)
{
    char *filepath;
    struct stat statbuf;
    static char *path[] = { "/bin", "/usr/bin", "." };
    int i;
    extern char **environ;

    /*
     * first, if this is an actual pipeline (i.e. of length > 1),
     * make a pipe and handle all but one entry recursively.
     * The recursion is kinda wacky because the parse of a|b|c is
     * like (a|b)|c, but we recurse in the other direction.  So b|c
     * is the recursive invocation but the top-level process has to be the
     * one which ends up execing c.  Hence the exec is in the child process.
     */
    if (pl->next) {
	int pipefd[2];
	if (pipe(pipefd)) {
	    perror("pipe");
	    exit(127);
	}
	switch (fork()) {
	case -1:
	    perror("fork");
	    exit(127);
	case 0:
	    /* child */
	    close(pipefd[0]);
	    dup2(pipefd[1], 1);
	    close(pipefd[1]);
	    /* ... and proceed with command execution below. */
	    break;
	default:
	    /* parent */
	    close(pipefd[1]);
	    dup2(pipefd[0], 0);
	    close(pipefd[0]);
	    execute_pipeline(pl->next);
	    return;  /* won't be reached because e_pl() does not return */
	}
    }
    /*
     * at this point we are either the child process which is the
     * non-base-case for the recursion and we did the "break" from case 0
     * above, OR we are doing a pipeline of length one, which IS the base
     * case for the recursion.
     */
    if (strchr(pl->argv[0], '/')) {
	/* if command contains a '/', don't apply the search path. */
	filepath = pl->argv[0];
    } else {
	/* find first executable item by that name in the path. */
	for (i = 0; i < sizeof path / sizeof path[0]; i++) {
	    filepath = efilenamecons(path[i], pl->argv[0]);
	    if (stat(filepath, &statbuf) == 0)
		break;
	}
	if (i == sizeof path / sizeof path[0]) {
	    fprintf(stderr, "%s: Command not found\n", pl->argv[0]);
	    exit(127);
	}
    }
    (void)execve(filepath, pl->argv, environ);
    perror(filepath);
    exit(127);
}
