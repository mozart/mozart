#ifndef __SIGFPE_H__
#define __SIGFPE_H__

#include <fpu_control.h>
#include <signal.h>
#include <unistd.h>
#include <linux/user.h> /* for user_i387_struct */

#ifdef __cplusplus
extern "C" {
#endif

/* just for compatibility with SUN: */
struct siginfo{
        int si_code;
};
typedef struct siginfo siginfo_t;
typedef struct user_i387_struct ucontext_t;
typedef int sigfpe_code_type;
typedef void (*sigfpe_handler_type)(int, siginfo_t * , ucontext_t *);

#define SIGFPE_DEFAULT (_sigfpe_default)
#define SIGFPE_IGNORE  (_sigfpe_ignore)
#define SIGFPE_ABORT   (_sigfpe_abort)

#define FPE_FLTINV 0
#define FPE_FLTDEN 1
#define FPE_FLTDIV 2
#define FPE_FLTUND 3
#define FPE_FLTOVF 4
#define FPE_FLTRES 5

sigfpe_handler_type sigfpe( int code, sigfpe_handler_type handler);

#ifdef __cplusplus
}
#endif

#endif
