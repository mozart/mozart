#ifndef __SIGFPE_H__
#define __SIGFPE_H__

//#define __have_siginfo_t

#include <fpu_control.h>
#include <signal.h>
#include <unistd.h>
#include <linux/user.h> /* for user_i387_struct */

#ifdef __cplusplus
extern "C" {
#endif

/* just for compatibility with SUN: */
  /*
struct siginfo a{
	int si_code;
};
typedef struct siginfo siginfo_t;
  */
typedef struct user_i387_struct ucontext_t;
typedef int sigfpe_code_type;
typedef void (*sigfpe_handler_type)(int, siginfo_t * , ucontext_t *);

#define SIGFPE_DEFAULT (_sigfpe_default)
#define SIGFPE_IGNORE  (_sigfpe_ignore)
#define SIGFPE_ABORT   (_sigfpe_abort)

#ifndef FPE_FLTINV
#define FPE_FLTINV 0
#endif

#ifndef FPE_FLTDEN
#define FPE_FLTDEN 1
#endif

#ifndef FPE_FLTDIV
#define FPE_FLTDIV 2
#endif

#ifndef FPE_FLTUND
#define FPE_FLTUND 3
#endif

#ifndef FPE_FLTOVF
#define FPE_FLTOVF 4
#endif

#ifndef FPE_FLTRES
#define FPE_FLTRES 5
#endif

sigfpe_handler_type sigfpe( int code, sigfpe_handler_type handler);

#ifdef _FPU_SETCW
#define __setfpucw(CW) _FPU_SETCW(CW)
#endif

#ifdef _FPU_GETCW
#define __getfpucw(CW) _FPU_GETCW(CW)
#endif

#ifdef __cplusplus
}
#endif

#endif
