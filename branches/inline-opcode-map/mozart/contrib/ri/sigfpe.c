#include "sigfpe.h"
#include "ieeefp.h"
#include <stdio.h>

static void sigfpe_abort(int i, siginfo_t *info, ucontext_t *fpu_state)
{
	fprintf(stderr, "Floating point exception\n");
	abort();
}

static void sigfpe_ignore(int i, siginfo_t *info, ucontext_t *fpu_state)
{}

sigfpe_handler_type _sigfpe_abort   = (sigfpe_handler_type) &sigfpe_abort;
sigfpe_handler_type _sigfpe_default = (sigfpe_handler_type) &sigfpe_abort;
sigfpe_handler_type _sigfpe_ignore  = (sigfpe_handler_type) &sigfpe_ignore;

#define FPE_DEFAULT (sigfpe_handler_type) &sigfpe_abort

static sigfpe_handler_type  sigfpe_array[6] = {
	FPE_DEFAULT,
	FPE_DEFAULT,
	FPE_DEFAULT,
	FPE_DEFAULT,
	FPE_DEFAULT,
	FPE_DEFAULT};

static ucontext_t sigfpe_fpu_state;
static siginfo_t sigfpe_info;
static int dispatcher_installed = 0;

static void fpe_dispatcher(int i)
{
	long status, control, raised_exceptions, k, bit;

	/* A lovely mess-up here: we want to get at the status and
	   control registers to see what caused the exception. But
	   the status word is not there, because if it were, the kernel
	   would generate a new FPU trap when it tries to give control
	   back to us. So the kernel puts the status word in the low
	   bits of the code segment selector. However, to access the
	   SW now requires a full save/restore of the FPU, because 
	   all the instructions that store the CS also do an implied 
	   FINIT (reset). FSTENV/FLDENV won't do since we want to keep
	   the FP registers intact.
	*/
        __asm__ ("fnsave %0":"=m" (sigfpe_fpu_state));
	status = sigfpe_fpu_state.fcs & 0x0000ffff;
	control = sigfpe_fpu_state.cwd;
        __asm__ ("frstor %0":"=m" (sigfpe_fpu_state));
	raised_exceptions = status & 0x3f & ~control;

	/* Should we process all raised exceptions or just the first 
	   one? Not too sure, but the former option is chosen: */

	for ( k = 0, bit = 1 ; k < 6 ; k++, bit <<= 1 )
	{
		if ( raised_exceptions & bit ) 
		{
			sigfpe_info.si_code = k;
		 	sigfpe_array[k](i, &sigfpe_info, &sigfpe_fpu_state);
		}
	}
}

sigfpe_handler_type sigfpe( int code, sigfpe_handler_type handler)
{
	sigfpe_handler_type old_handler;

	/* This is a bit inelegant. Arguably, the dispatcher should get
	 * installed in crt0.c, but I don't like messing with things I
	 * don't understand....
	 */
	if ( !dispatcher_installed )
	{ /* install it */
		signal(SIGFPE, fpe_dispatcher);
		dispatcher_installed = 1;
	}
	
	old_handler = sigfpe_array[code];
	sigfpe_array[code] = handler;
	return old_handler;
}
