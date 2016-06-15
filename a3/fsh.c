/*
 * fsh.c - the Feeble SHell.
 */

#include <stdio.h>
#include "fsh.h"
#include "parse.h"
#include "error.h"
#include "builtin.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

int showprompt = 1;
int laststatus = 0;  /* set by anything which runs a command */
extern char **environ;

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
    char bin[1000];
    char usrbin[1000];
    char current[1000];
    struct parsed_line *cmd;
    int status, pid;
    extern int docommand(char *bin, char *usrbin, char *current, char **argp);
    extern void docommand2(char *bin, char *usrbin, char *current, char **argp, char **argpnext);

    for (; p; p = p->next) {
    if (p->pl == NULL) {
        continue;
    }

    if (strchr(p->pl->argv[0], '/')) {
        strcpy(bin, p->pl->argv[0]);
        strcpy(usrbin, p->pl->argv[0]);
        strcpy(current, p->pl->argv[0]);
    } else {
        strcpy(bin, efilenamecons("/bin", p->pl->argv[0]));
        strcpy(usrbin, efilenamecons("/usr/bin", p->pl->argv[0]));
        strcpy(current, efilenamecons(".", p->pl->argv[0]));
    }

    if (strcmp(p->pl->argv[0], "exit") == 0) {
        laststatus = builtin_exit(p->pl->argv);
    }

    int x = fork();
    if (x == -1) {
        perror("fork");
        laststatus = 1;
    } else if (x == 0) {
        // Input Redirection
        if (p->inputfile) {
            close(0);
            if (open(p->inputfile, O_RDONLY, 0666) < 0) {
                perror(p->inputfile);
                exit(1);
            }
            laststatus = docommand(bin, usrbin, current, p->pl->argv);
        }
        // Pipelines of length two
        if (p->pl->next != NULL) { 
            int temp, wait2;
            fflush(stdout);
            switch (temp = fork()) {
                case -1:
                    perror("fork");
                    exit(1);
                case 0:
                    docommand2(bin, usrbin, current, p->pl->argv, p->pl->next->argv);
                    break;
                default:
                    temp = wait(&wait2);
                    laststatus = wait2 >> 8;
                }
            }
        // Output Redirection
        else if (p->outputfile) {
            close(1);
            if (open(p->outputfile, O_WRONLY|O_CREAT|O_TRUNC, 0666) < 0) {
                perror(p->outputfile);
                exit(1);
            }
            laststatus = docommand(bin, usrbin, current, p->pl->argv);
        }
        // Connectives
        else if (p->conntype) {
            for (cmd = p; cmd; cmd = cmd->next) {            
                if (strchr(cmd->pl->argv[0], '/')) {
                    strcpy(bin, cmd->pl->argv[0]);
                    strcpy(usrbin, cmd->pl->argv[0]);
                    strcpy(current, cmd->pl->argv[0]);
                } else {
                    strcpy(bin, efilenamecons("/bin", cmd->pl->argv[0]));
                    strcpy(usrbin, efilenamecons("/usr/bin", cmd->pl->argv[0]));
                    strcpy(current, efilenamecons(".", cmd->pl->argv[0]));
                }
                if (cmd->conntype == CONN_AND) {
                    if (laststatus == 0) {
                        laststatus = docommand(bin, usrbin, current, cmd->pl->argv);
                    }
                }
                else if (cmd->conntype == CONN_OR) {
                    if (laststatus != 0) {
                        laststatus = docommand(bin, usrbin, current, cmd->pl->argv);          
                    }
                } else {
                    docommand(bin, usrbin, current, cmd->pl->argv);
                } 
            }
        }
        else if (strcmp(p->pl->argv[0], "cd") == 0) {
            laststatus = builtin_cd(p->pl->argv);
        }
        // Execute Simple Commands
        else {
            laststatus = docommand(bin, usrbin, current, p->pl->argv);
        }
    }
        // Parent
    else {
            pid = wait(&status);
            laststatus = status >> 8;
        }
    }
}

int docommand(char *bin, char *usrbin, char *current, char **argp)  
{
    struct stat statbuf;
    if (stat(bin, &statbuf) == 0) {
        execve(bin, argp, environ);
        perror(bin);
        return(1);
    }
    else if (stat(usrbin, &statbuf) == 0) {
        execve(usrbin, argp, environ);
        perror(usrbin);
        return(1);
    }
    else if (stat(current, &statbuf) == 0) {
        execve(current, argp, environ);
        perror(current);
        return(1);
    } else {
        fprintf(stderr, "%s: Command not found.\n", argp[0]);
        return(1);
    }
}

void docommand2(char *bin, char *usrbin, char *current, char **argp, char **argpnext)  
{
    int status;
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
        close(pipefd[0]);  
        dup2(pipefd[1], 1);  
        close(pipefd[1]);  
        status = docommand(bin, usrbin, current, argp);
        if (status == 1) {
            exit(125);
        }
    default:
        if (strchr(argpnext[0], '/')) {
            strcpy(bin, argpnext[0]);
            strcpy(usrbin, argpnext[0]);
            strcpy(current, argpnext[0]);
        } else {
            strcpy(bin, efilenamecons("/bin", argpnext[0]));
            strcpy(usrbin, efilenamecons("/usr/bin", argpnext[0]));
            strcpy(current, efilenamecons(".", argpnext[0]));
        }
        close(pipefd[1]); 
        dup2(pipefd[0], 0);  
        close(pipefd[0]);  
        status = docommand(bin, usrbin, current, argpnext);
        if (status == 1) {
            exit(125);
        }
    }
}