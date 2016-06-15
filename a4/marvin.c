#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include "parse.h"
#include "util.h"
#include "chatsvr.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <ctype.h>

int portnumber = 1234;
int main(int argc, char **argv) {
	int fd, maxfd;
	struct sockaddr_in r;
	struct hostent *hp;
	fd_set fdlist;
	char *nextpos;
	int bytes_in_buf = 0;
	char *name_token;
	char *expr_token;
	int nbytes;
	char msg[MAXMESSAGE+16] = "";
	char msgcopy[MAXMESSAGE+16] = "";
	static char greeting[] = CHATSVR_ID_STRING"\r\n";
	static char marvin[] = "Marvin\r\n";
	extern void process(int fd, char *name_token, char *expr_token);
	extern char *mystrstr(char *haystack, char *needle);

	if (argc < 2 || argc > 3) {
		fprintf(stderr, "usage: %s hostname [port]\n", argv[0]);
		return(1);
	}

	if ((hp = gethostbyname(argv[1])) == NULL) {
		fprintf(stderr, "%s: no such host\n", argv[1]);
		return(1);
	}
	if (hp->h_addr_list[0] == NULL || hp->h_addrtype != AF_INET) {
		fprintf(stderr, "%s: not an internet protocol host name\n", argv[1]);
		return(1);
	}

	if (argv[2]) {
		if (atoi(argv[2]) == 0) {
			fprintf(stderr, "%s: port number must be a positive integer\n", argv[2]);
		}
		portnumber = atoi(argv[2]);
	}
	
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return(1);
	}
		
	memcpy(&r.sin_addr, hp->h_addr_list[0], hp->h_length);
	r.sin_family = AF_INET;
	r.sin_port = htons(portnumber);
	
	if (connect(fd, (struct sockaddr *)&r, sizeof r) < 0) {
		perror("connect");
		return(1);
	}

		for(;;) {
		FD_ZERO(&fdlist);
		FD_SET(fd, &fdlist);
		maxfd = fd;
		FD_SET(0, &fdlist);
		if (0 > maxfd)
			maxfd = 0;
		if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
			perror("select");
		} else {
			if (FD_ISSET(fd, &fdlist)) {
			// move the leftover data to the beginning of buf 
		    	if (bytes_in_buf && nextpos) {
		    		memmove(msg, nextpos, bytes_in_buf);
		    	}
		    	// if we've already got another whole line, return it without a read() 
				if ((nextpos = extractline(msg, bytes_in_buf))) {
				 	bytes_in_buf -= (nextpos - msg);
				 	printf("%s\n", msg);
				}

			 	// try a read
				nbytes = read(fd, msg + bytes_in_buf, sizeof msg - bytes_in_buf - 1);
				if (nbytes <= 0) {
					if (nbytes < 0) {
						perror("read()");
				    }
					else if (nbytes == 0) {
						printf("Server shut down\n");
						exit(1);
					}
				} else {
		    		if (strcmp(greeting, msg) == 0) {
   						if (write(fd, marvin, sizeof marvin - 1) != sizeof marvin - 1) {
            		    	perror("write()");
            	      		return(1);
    	   				}
		    		}
			    	bytes_in_buf += nbytes;

			    	// so, now do we have a whole line?
			    	if ((nextpos = extractline(msg, bytes_in_buf))) {
			    		bytes_in_buf -= (nextpos - msg);
			    		if (mystrstr(greeting, msg)) {
			    			continue;
			    		} else {
                        	printf("%s\n", msg);
                        }
                    }

			    	if (mystrstr(msg, "hey marvin,")) {
			    		strcpy(msgcopy, msg);
			    		name_token = strtok(msg, ":");
			    		expr_token = strtok(msgcopy, ",");
			    		expr_token = strtok(NULL, ", ");
			    		if (expr_token == NULL) {
			    			process(fd, name_token, " ");
			    		} else {
			    			process(fd, name_token, expr_token);
			    		}
			    	}

			    	// if max message, call it a line
					if (bytes_in_buf >= MAXMESSAGE) {
						msg[bytes_in_buf] = '\0';
						bytes_in_buf = 0;
						nextpos = NULL;
						printf("%s\n", msg);
					}
				}
				} 
				else if (FD_ISSET(0, &fdlist)) {
					char buf[MAXMESSAGE+16];
					char send[MAXMESSAGE+16];
					if (fgets(buf, sizeof buf, stdin) == NULL) {
						printf("Server shut down\n");
					    	exit(1);
					}
					strcpy(send, buf);
					if (write(fd, send, strlen(send)) != strlen(send)) {
						perror("write()");
						exit(1);
					}
					memset((void *) buf, 0, sizeof buf); 
				} 
			}
		}
	return(0);
}

void process(int fd, char *name_token, char *expr_token) {
	struct expr *e = parse(expr_token);
	char msgtosend[MAXMESSAGE+16];
	if (e) {
		sprintf(msgtosend, "Hey %s, %d\r\n", name_token, evalexpr(e));
		    if (write(fd, msgtosend, strlen(msgtosend)) != strlen(msgtosend)) {
	    		perror("write()");
	    	}
	    freeexpr(e);
	} else {
		sprintf(msgtosend, "Hey %s, I don't like that.\r\n", name_token);
			if (write(fd, msgtosend, strlen(msgtosend)) != strlen(msgtosend)) {
	    		perror("write()");
	    	}
	    printf("[%s]\n", errorstatus);
	}
}

char *mystrstr(char *msg, char *marvin)
{
   if (!*marvin) {
      return msg;
   }
   for (; *msg; ++msg) {
      if (toupper(*msg) == toupper(*marvin)) {
         char *message, *marv;
         for (message = msg, marv = marvin; *message && *marv; ++message, ++marv) {
            if (toupper(*message) != toupper(*marv)) {
               break;
            }
         }
         if (!*marv) {
            return marvin; 
         }
      }
   }
   return 0;
}
