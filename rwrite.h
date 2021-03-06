/*  -*- c -*-
 *
 * $RCSfile: rwrite.h,v $
 * ----------------------------------------------------------------------
 * Header file of the rwrite and rwrited programs that implement
 * the RWP protocol.
 * ----------------------------------------------------------------------
 * Created      : Tue Sep 13 15:27:58 1994 tri
 * Last modified: Sat Apr 18 23:35:04 1998 tri
 * ----------------------------------------------------------------------
 * Copyright � 1994-1998
 * Timo J. Rinne <tri@iki.fi>
 * All rights reserved.  See file COPYRIGHT for details.
 *
 * Address: Cirion oy, PO-BOX 250, 00121 Helsinki, Finland
 * ----------------------------------------------------------------------
 * Any express or implied warranties are disclaimed.  In no event
 * shall the author be liable for any damages caused (directly or
 * otherwise) by the use of this software.
 *
 * Please, send your patches to <tri@iki.fi>.
 * ----------------------------------------------------------------------
 * $Revision: 1.39 $
 * $State: Exp $
 * $Date: 1998/04/18 20:52:28 $
 * $Author: tri $
 * ----------------------------------------------------------------------
 * $Log: rwrite.h,v $
 * Revision 1.39  1998/04/18 20:52:28  tri
 * New copyright in COPYRIGHT.
 *
 * Revision 1.38  1996/05/17 12:31:28  tri
 * SOCKS support from kivinen@iki.fi.
 *
 * Revision 1.37  1995/11/14 06:35:54  tri
 * Readline library can be disabled in rc-file.
 *
 * Revision 1.36  1995/10/24 21:54:56  tri
 * Added support for gnu libreadline.
 *
 * Revision 1.35  1994/12/14  22:06:07  tri
 * Cleanup.
 *
 * Revision 1.34  1994/12/14  19:12:36  tri
 * Hacked udp connection type a bit, but it
 * does not seem to work.
 *
 * Revision 1.33  1994/12/13  20:29:53  tri
 * Preparation for autoconfig and tcp-port change.
 *
 * Revision 1.30  1994/12/12  21:17:55  tri
 * Closed files more pedantically.
 * Fix by toka & tri.
 *
 * Revision 1.29  1994/12/12  19:50:16  tri
 * Fixed a small but potentially harmful fclose(NULL) -bug.
 *
 * Revision 1.26  1994/12/12  11:08:14  tri
 * Moved the name of the file containing last
 * message into rwrite.h
 *
 * Revision 1.25  1994/12/11  21:25:30  tri
 * Cleaned up some warnings.  No functional changes.
 *
 * Revision 1.23  1994/12/11  18:16:28  tri
 * Some portability fixes and configuration stuff
 * moved to Makefile.
 *
 * Revision 1.21  1994/12/11  13:29:29  tri
 * Background message sending can be defaulted in
 * rwriterc.  Explicit -b or -B flag overrides the
 * default.
 *
 * Revision 1.20  1994/12/11  12:58:17  tri
 * Fixed the allow-deny -heuristics to be
 * more powerful.
 * Also added the cleardefs command to the rc-file syntax.
 *
 * Revision 1.19  1994/12/10  11:28:38  tri
 * Last known method to send terminal control codes
 * through correctly configured rwrite is now diabled.
 *
 * Revision 1.18  1994/12/09  23:57:49  tri
 * Added a outbond message logging.
 *
 * Revision 1.16  1994/12/09  10:28:56  tri
 * Fixed a return value of dequote_and_send().
 *
 * Revision 1.14  1994/12/08  22:56:45  tri
 * Fixed the quotation system on message
 * delivery.  Same message can now be quoted
 * differently for the each receiver.
 * Also the autoreplies are now quoted right.
 *
 * Revision 1.13  1994/12/07  12:34:32  tri
 * Removed read_message() and dropped in Camillo's GetMsg()
 * instead.
 *
 * Revision 1.12  1994/11/22  20:49:13  tri
 * Added configurable parameter to limit the number
 * of lines in the incoming message.
 *
 * Revision 1.11  1994/11/20  11:08:12  tri
 * Fixed minor quotation bug in backround mode.
 *
 * Revision 1.10  1994/11/20  00:47:18  tri
 * Completed autoreply and quotation stuff.
 * We are almost there now.
 *
 * Revision 1.9  1994/10/06  18:32:37  tri
 * Hacked multitty option.
 *
 * Revision 1.8  1994/10/04  20:50:22  tri
 * Conforms now the current RWP protocol.
 *
 * Revision 1.7  1994/09/20  19:08:57  tri
 * Added a configuration option for rwrited ran
 * without tty setgid.
 *
 * Revision 1.6  1994/09/20  08:24:13  tri
 * Support for .rwrite-allow and .rwrite-deny files.
 *
 * Revision 1.5  1994/09/19  22:40:37  tri
 * TOOK replaced by VRFY and made some considerable
 * cleanup.
 *
 * Revision 1.4  1994/09/15  20:14:42  tri
 * Completed the support of RWP version 1.0.
 *
 * Revision 1.1  1994/09/13  12:32:13  tri
 * Initial revision
 *
 * ----------------------------------------------------------------------
 */
/* Headers are included only once. */
#ifndef __RWRITE_H__
#define __RWRITE_H__ 1

#define RWP_VERSION_NUMBER	"1.0"		/* Protocol version */

#define RWRITE_1_1_COMPAT		   /* Enable rwrite 1.1.1 compat */

/*
 * User definitions are in the following files.
 */
#define RWRITE_CONFIG_FILE	".rwriterc"
#define RWRITE_GLOBAL_CONFIG	"/etc/rwrite.conf"
#define RWRITE_AUTOREPLY_FILE	".rwrite-autoreply"
#define RWRITE_LAST_SENT_MSG    ".rwrite-last"

#define PATH_SEPARATOR          ((int)'!')  /* Separator char in delivery path */
#define ADDRESS_TTY_SEPARATOR	((int)':')

/*
 * Allocation step in line buffer allocation. 
 * Has to be at least 4.  No need to modify this anyway.
 */
#define BUF_ALLOC_STEP	128

/*
 * Daemon sends only first MAX_AUTOREPLY_LINES lines of autoreply.
 * -1 means unlimited.
 */
#define MAX_AUTOREPLY_LINES 64

/*
 * Maximum number of lines in the incoming message.  
 * This can be overridden in user configuration file.
 */
#define DEFAULT_MAX_LINES_IN	1024
#define DEFAULT_MAX_CHARS_IN	(DEFAULT_MAX_LINES_IN * 64)

/*
 * Maximum length of the packet sent via udp.
 */
#define UDP_DIALOG_LEN_MAX 512

/*
 * Maximum number of lines that DATA command can get.
 */
#define DATA_MAXLINES	DEFAULT_MAX_LINES_IN
#define DATA_MAXCHARS	(DATA_MAXLINES * 64)

/*************************************************/
/*************************************************/
/********* END OF THE USER CONFIGURATION *********/
/*************************************************/
/*************************************************/

/*
 * Prototypes of the resource functions
 */
int rc_read_p(void);
int ring_bell(void);
int max_lines_in(void);
int max_chars_in(void);
int default_bg(void);
int add_to_list(char ***list, int *list_sz, char *str);
int add_list_to_list(char ***tgt, int *tgt_sz, char **list);
int is_in_list(char **list, char *str);
void reset_rc(void);
void read_rc(char *fn);
int is_allowed(char *name, char *host);
int use_readline(void);
int deliver_all_ttys(void);
int no_tty_delivery(void);
char *quote_str(char *s);
char *dequote_str(char *s, int maxlen, int *len);
int dequote_and_write(FILE *f, char **msg, int maxlines, int maxchars, 
		      int is_f_tty);
#ifndef __RWRITERC_C__
extern char **rc_tty_list;
extern char **rc_outlog;
extern char *rc_prompt;
#endif

#define RWRITE_PROMPT (rc_prompt ? rc_prompt : "")

/*
 * #
 * # Entry to enable rwrite service in /etc/services.
 * # The port number is about to change in near future.
 * #
 * rwrite		18/tcp			# rwrite
 */
#define RWRITE_DEFAULT_PORT	18

#define RWRITE_FWD_LIMIT	32
/*
 * These response codes follow the RWP version 1.0 RFC.
 */
/*
 * Success codes.
 */
#define RWRITE_READY		100
#define RWRITE_BYE		101
#define RWRITE_DELIVERY_OK	103
#define RWRITE_DELIVERY_FORWARDED	104
#define RWRITE_SENDER_OK	105
#define RWRITE_RCPT_OK		106
#define RWRITE_MSG_OK		107
#define RWRITE_RCPT_OK_TO_SEND	108
#define RWRITE_RSET_OK		109
#define RWRITE_RCPT_OK_TO_FWD	110
#define RWRITE_FHST_OK		111
#define RWRITE_QUOTE_OK		112
/*
 * Server readu to receive message body.
 */
#define RWRITE_GETMSG		200
/*
 * Autoreply
 */
#define RWRITE_AUTOREPLY	555 /* To be 300 */
/* Older RWP clients barf with 300 but ignore 555 so let it be 555 for now. */
#define RWRITE_AUTOREPLY_AS_COMMENT	556
/*
 * Informational responses.
 */
#define RWRITE_HELO		500
#define RWRITE_VER		501
#define RWRITE_PROT		502
#define RWRITE_HELP		510
#define RWRITE_INFO		511 /* Stuff for client to ignore. */
#define RWRITE_DEBUG		512 /* Stuff for client to ignore. */
/*
 * Error codes.
 */
#define RWRITE_ERR_FATAL	666
#define RWRITE_ERR_SYNTAX	668
#define RWRITE_ERR_PERMISSION_DENIED	669
#define RWRITE_ERR_USER_NOT_IN	670
#define RWRITE_ERR_NO_SUCH_USER	671
#define RWRITE_ERR_NO_MESSAGE	672
#define RWRITE_ERR_NO_SENDER	673
#define RWRITE_ERR_NO_ADDRESS	674
#define RWRITE_ERR_NO_DATA	675
#define RWRITE_ERR_FWD_LIMIT_EXCEEDED	676
#define RWRITE_ERR_FWD_FAILED	677
#define RWRITE_ERR_QUOTE_CMD_FAILED	678
#define RWRITE_ERR_QUOTE_CMD_UNKNOWN	679
#define RWRITE_ERR_INTERNAL	698
#define RWRITE_ERR_UNKNOWN	699

#ifdef SOCKS
#  define connect Rconnect
#  define getsockname Rgetsockname
#  define bind Rbind
#  define accept Raccept
#  define listen Rlisten
#  define select Rselect
#define SEND_FHST 1
#endif

#endif /* not __RWRITE_H__ */
/* EOF (rwrite.h) */
