#include "ieeefp.h"
#include "sigfpe.h"

#define ROUND_BITS 0xC00
#define MASK_BITS  0x3f

fp_rnd fpgetround(void)
{
	return __fpu_control & ROUND_BITS;
}

fp_except fpgetmask(void)
{
	return __fpu_control & MASK_BITS;
}

fp_rnd fpsetround(fp_rnd code) 
{
        fp_rnd r = __fpu_control & ROUND_BITS;  

	__fpu_control &= ~ROUND_BITS;
	__fpu_control |= code & ROUND_BITS;
	__setfpucw(__fpu_control);

	return r;
}

fp_except fpsetmask(fp_except code) 
{
        fp_except r = __fpu_control & MASK_BITS;  

	__fpu_control &= ~MASK_BITS;
	__fpu_control |= code & MASK_BITS;
	__setfpucw(__fpu_control);

	return r;
}

/* The proper implementation of the following two routines is a bit
   - er - sticky, because Linux and SunOS differ in their handling of
   FPU traps. A "sticky bit" is something that is set on FPU error,
   masked or not, so it corresponds to the x87 status word. But on Sun,
   the sticky bits can be seen at all times, also during exception 
   handling and they must be explicitly reset by the handler, whereas
   Linux zaps the status word before passing control back to the SIGFPE
   handler. Things get a bit complicated if we insist on perfect
   compatibility, so I didn't do it (yet).

   (The current kernel handling of FPU traps is a bit kludgy. Rather
   than mangling other registers to save the status word, the
   exception bits could be cleared and the original status word put
   into the signal context structure. This would require the sticky
   routines to keep track of whether they are called from a signal
   handler or not, but that is no big deal.)
 */
fp_except fpgetsticky(void)
{
	fp_except r;

	__asm__ ("fnstsw %0":"=m" (r));

	return r & MASK_BITS;
}

fp_except fpsetsticky(fp_except code)
{
	fp_except r;
	static ucontext_mozart_t sigfpe_fpu_state;


	/* There seems to be no such thing as an fldsw instruction,
	   so we have no other option but to dump the entire FPU,
	   hack the status word, and restore the image. Note that 
	   setting bits for unmasked exceptions generates a trap. */
	__asm__ ("fnsave %0":"=m" (sigfpe_fpu_state));
        r = sigfpe_fpu_state.swd & MASK_BITS;
	sigfpe_fpu_state.swd &= ~MASK_BITS;
	sigfpe_fpu_state.swd |= code & MASK_BITS;
        __asm__ ("frstor %0":"=m" (sigfpe_fpu_state));

	return r;
}


