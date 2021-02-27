#ifndef MSGNO_H
#define MSGNO_H

/* msgno - managing error codes and associated messages across
 * separate C libraries
 */

#include <stdio.h>

#ifndef MSGNO

#if WIN32

#define MSG     
#define MNO     
#define MNF     
#define PMSG    
#define PMNO    
#define PMNF    
#define AMSG    
#define AMNO    
#define AMNF    

#else

#define MSG(fmt, args...)
#define MNO(msgno)
#define MNF(msgno, fmt, args...)
#define PMSG(fmt, args...)
#define PMNO(msgno)
#define PMNF(msgno, fmt, args...)
#define AMSG(fmt, args...)
#define AMNO(msgno)
#define AMNF(msgno, fmt, args...)

#endif
/*
*/

#else

#if defined(__GNUC__) && (__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 96))

#define MSG(fmt, args...) _msgno_printf("%s:%u:%s: " fmt "\n", \
					__FILE__, __LINE__, __FUNCTION__, ## args)
#define MNO(msgno) _msgno_printf("%s:%u:%s: %s\n", \
					__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno))
#define MNF(msgno, fmt, args...) _msgno_printf("%s:%u:%s: %s" fmt "\n", \
					__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno), ## args)

#define PMSG(fmt, args...) (_msgno_buf_idx = sprintf(_msgno_buf, \
					"%s:%u:%s: " fmt "\n", \
					__FILE__, __LINE__, __FUNCTION__, ## args))
#define PMNO(msgno) (_msgno_buf_idx = sprintf(_msgno_buf, \
					"%s:%u:%s: %s\n", \
					__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno)))
#define PMNF(msgno, fmt, args...) (_msgno_buf_idx = sprintf(_msgno_buf, \
					"%s:%u:%s: %s" fmt "\n", \
					__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno), ## args))

#define AMSG(fmt, args...) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
					"  %s:%u:%s: " fmt "\n", \
					__FILE__, __LINE__, __FUNCTION__, ## args))
#define AMNO(msgno) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
					"  %s:%u:%s: %s\n", \
					__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno)))
#define AMNF(msgno, fmt, args...) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
					"  %s:%u:%s: %s" fmt "\n", \
					__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno), ## args))

#else

#if WIN32

#define MSG             
#define MNO(msgno)      printf("%s:%u: %s\n", __FILE__, __LINE__, msgno_msg(msgno))
#define MNF             
#define PMSG            
#define PMNO(msgno)     printf("%s:%u: %s\n", __FILE__, __LINE__, msgno_msg(msgno))
#define PMNF            
#define AMSG            
#define AMNO(msgno)     printf("%s:%u: %s\n", __FILE__, __LINE__, msgno_msg(msgno))
#define AMNF            

#else

#define MSG(fmt, args...) _msgno_printf("%s:%u: " fmt "\n", \
					__FILE__, __LINE__ , ## args)
#define MNO(msgno) _msgno_printf("%s:%u: %s\n", \
					__FILE__, __LINE__, msgno_msg(msgno))
#define MNF(msgno, fmt, args...) _msgno_printf("%s:%u: %s" fmt "\n", \
					__FILE__, __LINE__, msgno_msg(msgno) , ## args)

#define PMSG(fmt, args...) (_msgno_buf_idx = sprintf(_msgno_buf, \
					"%s:%u: " fmt "\n", __FILE__, __LINE__ , ## args))
#define PMNO(msgno) (_msgno_buf_idx = sprintf(_msgno_buf, \
					"%s:%u: %s\n", __FILE__, __LINE__, msgno_msg(msgno)))
#define PMNF(msgno, fmt, args...) (_msgno_buf_idx = sprintf(_msgno_buf, \
					"%s:%u: %s" fmt "\n", __FILE__, __LINE__, msgno_msg(msgno) , ## args))

#define AMSG(fmt, args...) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
					"  %s:%u: "fmt"\n", __FILE__, __LINE__ , ## args))
#define AMNO(msgno) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
					"  %s:%u: %s\n", __FILE__, __LINE__, msgno_msg(msgno)))
#define AMNF(msgno, fmt, args...) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
					"  %s:%u: %s" fmt "\n", __FILE__, __LINE__, msgno_msg(msgno) , ## args))
#endif
#endif
#endif

#define NULL_POINTER_ERR _builtin_codes[0].msgno

struct msgno_entry {
	unsigned int msgno;
	const char *msg;
};

extern struct msgno_entry _builtin_codes[];

int msgno_add_codes(struct msgno_entry *list);
const char *msgno_msg(unsigned int errnum);

int msgno_hdlr_stderr(const char *fmt, ...);
extern int (*msgno_hdlr)(const char *fmt, ...);

extern char _msgno_buf[];
extern unsigned int _msgno_buf_idx;
void _msgno_printf(const char *fmt, ...);

#endif /* MSGNO_H */

