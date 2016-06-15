#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "chatsvr.h"
#include "util.h"
#include "parse.h"

static int serverfd;
char msg[MAXMESSAGE+2];  /* must be greater than MAXHANDLE + 3 */

/* data from server */
static char buf[MAXMESSAGE+MAXHANDLE+16];
static int bytes_in_buf = 0;
static char *nextpos;

static void checkmarvin(char *p);


int main(int argc, char **argv)
{
    int port = 1234;  /* default port number */
    int len;
    char *p;
    static char marvinhandle[] = "Marvin\r\n";
    extern void connect_to_server(char *host, int port);
    extern char *read_from_server();  /* read from server, remove network newline */

    if (argc == 3) {
	if ((port = atoi(argv[2])) == 0) {
	    fprintf(stderr, "%s: port number must be a positive integer\n", argv[0]);
	    return(1);
	}
    } else if (argc != 2) {
	fprintf(stderr, "usage: %s host [port]\n", argv[0]);
	return(1);
    }

    connect_to_server(argv[1], port);
    if (write(serverfd, marvinhandle, sizeof marvinhandle - 1) != sizeof marvinhandle - 1) {
	perror("write");
	exit(1);
    }

    while (1) {
	fd_set fdlist;
	FD_ZERO(&fdlist);
	FD_SET(serverfd, &fdlist);
	FD_SET(0, &fdlist);
	if (select(serverfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
	    perror("select");
	    exit(1);
	}
	if (FD_ISSET(serverfd, &fdlist)) {
	    if ((p = read_from_server())) {
		printf("%s\n", p);
		checkmarvin(p);
	    }
	} else if (fgets(msg, sizeof msg - 2, stdin) == NULL) {
	    exit(0);
	} else {
	    if ((p = strchr(msg, '\n')))
		*p = '\0';
	    strcat(msg, "\r\n");
	    len = strlen(msg);
	    if (write(serverfd, msg, len) != len)
		perror("write");
	}
    }
}


void connect_to_server(char *host, int port)
{
    struct hostent *hp;
    struct sockaddr_in r;
    char *p;
    extern char *read_from_server();

    if ((hp = gethostbyname(host)) == NULL) {
	fprintf(stderr, "%s: no such host\n", host);
	exit(1);
    }
    if (hp->h_addr_list[0] == NULL || hp->h_addrtype != AF_INET) {
	fprintf(stderr, "%s: not an internet protocol host name\n", host);
	exit(1);
    }

    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&r, '\0', sizeof r);
    r.sin_family = AF_INET;
    memcpy(&r.sin_addr, hp->h_addr_list[0], hp->h_length);
    r.sin_port = htons(port);

    if (connect(serverfd, (struct sockaddr *)&r, sizeof r) < 0) {
        perror("connect");
        exit(1);
    }

    while ((p = read_from_server()) == NULL)
	;
    if (strcmp(p, CHATSVR_ID_STRING)) {
	fprintf(stderr, "That is not a 'chatsvr'\n");
	exit(1);
    }
}


char *read_from_server()
{
    int nbytes;

    /* move the leftover data to the beginning of buf */
    if (bytes_in_buf && nextpos)
        memmove(buf, nextpos, bytes_in_buf);

    /* If we've already got another whole line, return it without a read() */
    if ((nextpos = extractline(buf, bytes_in_buf))) {
        bytes_in_buf -= (nextpos - buf);
        return(buf);
    }

    /*
     * Ok, try a read().  Note that we _never_ fill the buffer, so that there's
     * always room for a \0.
     */
    nbytes = read(serverfd, buf + bytes_in_buf, sizeof buf - bytes_in_buf - 1);
    if (nbytes <= 0) {
	int status = 0;
        if (nbytes < 0) {
            perror("read()");
	    status = 1;  /* corrected for posted solution */
	}
	printf("Server shut down\n");
	exit(status);
    }

    bytes_in_buf += nbytes;

    /* So, _now_ do we have a whole line? */
    if ((nextpos = extractline(buf, bytes_in_buf))) {
	bytes_in_buf -= (nextpos - buf);
	return(buf);
    }
    /*
     * Don't do another read(), to avoid the possibility of blocking.
     * However, if we've hit the maximum message size, we should call
     * it all a line.
     */
    if (bytes_in_buf >= MAXMESSAGE) {
	buf[bytes_in_buf] = '\0';
	bytes_in_buf = 0;
	nextpos = NULL;
	return(buf);
    }

    /* If we got to here, we don't have a full input line yet. */
    return(NULL);
}


static void checkmarvin(char *p)
{
    char *q, *r;
    struct expr *e;
    int len;

    /* We return as soon as we determine that this is not a Marvin request. */

    /* skip up to and including colon and any subsequent whitespace */
    if ((q = strchr(p, ':')) == NULL)
	return;
    *q++ = '\0';
    while (*q && isspace(*q))
	q++;

    /* separate by the comma */
    if ((r = strchr(q, ',')) == NULL)
	return;
    *r++ = '\0';
    /* text between colon and comma must be "Hey Marvin", case-insensitive */
    if (strcasecmp(q, "Hey Marvin"))
	return;

    /* parse and evaluate expression and speak */
    if ((e = parse(r))) {
	snprintf(msg, sizeof msg, "Hey %s, %d\r\n", p, evalexpr(e));
	freeexpr(e);
    } else {
	printf("[%s]\n", errorstatus);
	snprintf(msg, sizeof msg, "Hey %s, I don't like that.\r\n", p);
    }
    len = strlen(msg);
    if (write(serverfd, msg, len) != len)
	perror("write");
}
