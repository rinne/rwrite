/*  -*- c -*-
 *
 * $RCSfile: rwrite.c,v $
 * ----------------------------------------------------------------------
 * Simple client to RWP-protocol
 * ----------------------------------------------------------------------
 * Created      : Tue Sep 13 15:28:07 1994 tri
 * Last modified: Wed Sep 14 17:51:56 1994 tri
 * ----------------------------------------------------------------------
 * $Revision: 1.1 $
 * $State: Exp $
 * $Date: 1994/09/14 14:58:53 $
 * $Author: tri $
 * ----------------------------------------------------------------------
 * $Log: rwrite.c,v $
 * Revision 1.1  1994/09/14 14:58:53  tri
 * Initial revision
 *
 * ----------------------------------------------------------------------
 * Copyright 1994, Timo Rinne <tri@cirion.fi> and Cirion oy.
 * 
 * Address: Cirion oy, PO-BOX 250, 00121 HELSINKI, Finland
 * 
 * Even though this code is copyrighted property of the author, it can
 * still be used for any purpose under following conditions:
 * 
 *     1) This copyright notice is not removed.
 *     2) Source code follows any distribution of the software
 *        if possible.
 *     3) Copyright notice above is found in the documentation
 *        of the distributed software.
 * 
 * Any express or implied warranties are disclaimed.  In no event
 * shall the author be liable for any damages caused (directly or
 * otherwise) by the use of this software.
 * ----------------------------------------------------------------------
 */
#define __RWRITE_C__ 1
#ifndef lint
static char *RCS_id = "$Id: rwrite.c,v 1.1 1994/09/14 14:58:53 tri Exp $";
#endif /* not lint */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pwd.h>

#include "rwrite.h"

/*
 * Allocation step in line buffer allocation. 
 * Has to be at least 2.  No need to modify this anyway.
 */
#define BUF_ALLOC_STEP	128

int verbose = 0;

int read_char(int fd)
{
    unsigned char buf[8];
    int ret;

    ret = read(fd, buf, 1);

    if(ret != 1)
	return -1;

    return((int)buf[0]);
}

char *read_line_fd(int f)
{
    char *buf;
    int buflen;
    int c, i;

    if(!(buf = ((char *)malloc(BUF_ALLOC_STEP))))
	return NULL;
    buflen = BUF_ALLOC_STEP;
    for(i = 0; (-1 != (c = (read_char(f)))); i++) {
	if((c == '\000') ||
	   (c == '\015')) {
	    i--;
	    continue;
	}
	if(c == '\012')
	    break;
	if((i + 1) >= buflen) {
	    char *newbuf;
	    buflen += BUF_ALLOC_STEP;
	    if(!(newbuf = (char *)malloc(buflen))) {
		free(buf);
		return NULL;
	    }
	    memcpy(newbuf, buf, i);
	    free(buf);
	    buf = newbuf;
	}
	buf[i] = c;
    }
    buf[i] = '\000';
    if((c == -1) && (i == 0)) {
	free(buf);
	return NULL;
    }
    buf[i] = '\000';
    return(buf);
}

char *read_line(FILE *f)
{
    char *buf;
    int buflen;
    int c, i;

    if(!(buf = ((char *)malloc(BUF_ALLOC_STEP))))
	return NULL;
    buflen = BUF_ALLOC_STEP;
    for(i = 0; (EOF != (c = (fgetc(f)))); i++) {
	if((c == '\000') ||
	   (c == '\015')) {
	    i--;
	    continue;
	}
	if(c == '\012')
	    break;
	if((i + 1) >= buflen) {
	    char *newbuf;
	    buflen += BUF_ALLOC_STEP;
	    if(!(newbuf = (char *)malloc(buflen))) {
		free(buf);
		return NULL;
	    }
	    memcpy(newbuf, buf, i);
	    free(buf);
	    buf = newbuf;
	}
	buf[i] = c;
    }
    buf[i] = '\000';
    if((c == EOF) && (i == 0)) {
	free(buf);
	return NULL;
    }

    buf[i] = '\000';
    return(buf);
}

char *read_rwp_resp(int s, int *code)
{
    char *ret;
    int c;

    if(!(ret = read_line_fd(s))) {
	return NULL;
    }
    c = atoi(ret);
    if(code)
	*code = c;
    return ret;
}

int write_string(int fd, char *s)
{
    int n, len, r;

    len = strlen(s);
    n = 0;
    while(len) {
	r = write(fd, &(s[n]), len);
	if(r < 0)
	    return 0;
	n += r;
	len -= r;
    }
    return 1;
}

#define LEGAL_CODE(c) (((c)>=100)&&((c)<=999))
#define IGNORABLE_CODE(c) (((c)>=500)&&((c)<=599))

#define WRITE_STRING(f, str) {                                            \
       if(!(write_string(s, str))) {                                      \
	   fprintf(stderr, "rwrite: Remote server closed connection.\n"); \
	   return 0; } }

#define DIALOG_BEGIN 1
#define DIALOG_TO    2
#define DIALOG_FROM  3
#define DIALOG_TOOK  4
#define DIALOG_DATA  5
#define DIALOG_SEND  6

int rwp_dialog(int s, char *to, char *from)
{
    int code;
    char *resp, *line;
    int mode, modeattr;

    mode = DIALOG_BEGIN;
    modeattr = 0;
    while(1) {
    redo_dialog_loop:;
	if(!(resp = read_rwp_resp(s, &code))) {
	    fprintf(stderr, "rwrite: Remote server closed connection.\n");
	    return 0;
	}
#ifdef DEBUG	
	fprintf(stderr, "Got >%03d< %s\n", code, resp); 
#endif
	if(!(LEGAL_CODE(code))) {
	    fprintf(stderr, "rwrite: Illegal RWP response code (%03d).\n", code);
	    return 0;
	}
	if(IGNORABLE_CODE(code)) {
	    goto redo_dialog_loop;
	}
	switch(mode) {
	case DIALOG_BEGIN:
	    switch(code) {
	    case RWRITE_READY:
		if(modeattr != 0) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		modeattr = 1;
		WRITE_STRING(s, "TO ");
		WRITE_STRING(s, to);
		WRITE_STRING(s, "\012");
		goto redo_dialog_loop;
	    case RWRITE_RCPT_OK:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		mode = DIALOG_TO;
		modeattr = 0;
		goto redo_dialog_loop;
	    case RWRITE_ERR_PERMISSION_DENIED:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		fprintf(stderr, "rwrite: Permission denied.\n");
		return 0;
	    case RWRITE_ERR_USER_NOT_IN:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		fprintf(stderr, "rwrite: User not in.\n");
		return 0;
	    case RWRITE_ERR_NO_SUCH_USER:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		fprintf(stderr, "rwrite: No such user.\n");
		return 0;
	    default:
		fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		return 0;
	    }
	case DIALOG_TO:
	    switch(code) {
	    case RWRITE_READY:
		if(modeattr != 0) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		modeattr = 1;
		WRITE_STRING(s, "FROM ");
		WRITE_STRING(s, from);
		WRITE_STRING(s, "\012");
		goto redo_dialog_loop;
	    case RWRITE_SENDER_OK:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		mode = DIALOG_FROM;
		modeattr = 0;
		goto redo_dialog_loop;
	    /* here maybe some error handling XXX */
	    default:
		fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		return 0;
	    }
	case DIALOG_FROM:
	    switch(code) {
	    case RWRITE_READY:
		if(modeattr != 0) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		modeattr = 1;
		WRITE_STRING(s, "TOOK\012");
		goto redo_dialog_loop;
	    case RWRITE_RCPT_OK_TO_SEND:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		mode = DIALOG_TOOK;
		modeattr = 0;
		goto redo_dialog_loop;
	    case RWRITE_ERR_PERMISSION_DENIED:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		fprintf(stderr, "rwrite: Permission denied.\n");
		return 0;
	    case RWRITE_ERR_USER_NOT_IN:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		fprintf(stderr, "rwrite: User not in.\n");
		return 0;
	    case RWRITE_ERR_NO_SUCH_USER:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		fprintf(stderr, "rwrite: No such user.\n");
		return 0;
	    default:
		fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		return 0;
	    }
	case DIALOG_TOOK:
	    switch(code) {
	    case RWRITE_READY:
		if(modeattr != 0) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		modeattr = 1;
		WRITE_STRING(s, "DATA\012");
		goto redo_dialog_loop;
	    case RWRITE_GETMSG:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		while(line = read_line(stdin)) {
		    WRITE_STRING(s, line);
		    WRITE_STRING(s, "\012");
		}
		WRITE_STRING(s, ".\012");
		modeattr = 2;
		goto redo_dialog_loop;
	    case RWRITE_MSG_OK:
		if(modeattr != 2) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		mode = DIALOG_DATA;
		modeattr = 0;
		goto redo_dialog_loop;
	    default:
		fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		return 0;
	    }
	case DIALOG_DATA:
	    switch(code) {
	    case RWRITE_READY:
		if(modeattr != 0) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		modeattr = 1;
		WRITE_STRING(s, "SEND\012");
		goto redo_dialog_loop;
	    case RWRITE_DELIVERY_OK:
	    case RWRITE_DELIVERY_FORWARDED:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		mode = DIALOG_SEND;
		modeattr = 0;
		goto redo_dialog_loop;
	    case RWRITE_ERR_PERMISSION_DENIED:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		fprintf(stderr, "rwrite: Permission denied.\n");
		return 0;
	    case RWRITE_ERR_USER_NOT_IN:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		fprintf(stderr, "rwrite: User not in.\n");
		return 0;
	    case RWRITE_ERR_NO_SUCH_USER:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 0;
		}
		fprintf(stderr, "rwrite: No such user.\n");
		return 0;
	    default:
		fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		return 0;
	    }
	case DIALOG_SEND:
	    switch(code) {
	    case RWRITE_READY:
		if(modeattr != 0) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 1;
		}
		modeattr = 1;
		WRITE_STRING(s, "BYE\012");
		goto redo_dialog_loop;
	    case RWRITE_BYE:
		if(modeattr != 1) {
		    fprintf(stderr, "rwrite: Unexpected RWP response code (%03d).\n", code);
		    return 1;
		}
		return 1;
	    default:
		fprintf(stderr, "rwrite: Internal error.\n");
	    }
	}
    }
    return 1;
}

/**************** From Berkeley Unix's finger(1) *****************/
/********************* Modifications by tri *********************/
int open_to(char *name)
{
    extern int lflag;
    FILE *fp;
    int c, lastc;
    struct in_addr defaddr;
    struct hostent *hp, def;
    struct servent *sp;
    struct sockaddr_in sin;
    int s;
    char *alist[1], *host, *rindex();
    u_long inet_addr();

    if(!(host = rindex(name, '@'))) {
	host = "localhost";
    } else {
	*host++ = '\000';
    }
    if(!(hp = gethostbyname(host))) {
	defaddr.s_addr = inet_addr(host);
	if(defaddr.s_addr == -1) {
	    (void)fprintf(stderr,
			  "rwrite: unknown host: %s\n", host);
	    return -1;
	}
	def.h_name = host;
	def.h_addr_list = alist;
	def.h_addr = (char *)&defaddr;
	def.h_length = sizeof(struct in_addr);
	def.h_addrtype = AF_INET;
	def.h_aliases = 0;
	hp = &def;
    }
    if(!(sp = getservbyname("rwrite", "tcp"))) {
	(void)fprintf(stderr, "rwrite: tcp/rwrite: unknown service\n");
	return -1;
    }
    sin.sin_family = hp->h_addrtype;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = sp->s_port;
    if((s = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
	perror("rwrite: socket");
	return -1;
    }
    /* have network connection; identify the host connected with */
    if(verbose)
	printf("[%s]\n", hp->h_name);
    if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
	perror("rwrite: connect");
	(void)close(s);
	return -1;
    }
    return s;
}

/********* END OF STUFF FROM Berkeley Unix's finger(1) **********/

int main(int argc, char **argv)
{
    int s, ret;
    char *to, *from;
    
    if(argc == 2) {
	{
	    struct passwd *pwd;
	    char *tmp;

	    if(!(tmp = getlogin())) {
		pwd = getpwuid(getuid());
		tmp = pwd ? (pwd->pw_name) : "UNKNOWN";
	    }
	    if(!(from = (char *)malloc(strlen(tmp) + 1))) {
		exit(1);
	    }
	    strcpy(from, tmp);
	}	
	if(!(to = (char *)malloc(strlen(argv[1]) + 1))) {
	    exit(1);
	}
	strcpy(to, argv[1]);
	if(0 > (s = open_to(to))) {
	    exit(2);
	}
	ret = rwp_dialog(s, to, from);
	close(s);
	exit(ret ? 0 : (-1));
    } else {
	fprintf(stderr, "USAGE: rwrite user[@host]\n");
	exit(3);
    }
    /*NOTREACHED*/
    return 0;
}

/* EOF (rwrite.c) */
