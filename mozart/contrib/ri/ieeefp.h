#ifndef __IEEEFP_H__
#define __IEEEFP_H__

#include <fpu_control.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned short fp_rnd;
typedef unsigned short fp_except;

fp_rnd fpgetround(void);
fp_rnd fpsetround(fp_rnd);
fp_except fpgetmask(void);
fp_except fpsetmask(fp_except);
fp_except fpgetsticky(void);
fp_except fpsetsticky(fp_except);

/* "Official" macros, compatible with Sun. Apparently no counterpart
	 of DM (denormal) */
#define FP_X_INV _FPU_MASK_IM /* Invalid op. */
#define FP_X_OFL _FPU_MASK_OM /* Overflow */
#define FP_X_UFL _FPU_MASK_UM /* Underflow */
#define FP_X_DZ  _FPU_MASK_ZM /* Divide by zero */
#define FP_X_IMP _FPU_MASK_PM /* Precision loss */

/* Same thing for rounding */
#define FP_RN _FPU_RC_NEAREST	/* To nearest */
#define FP_RM _FPU_RC_DOWN	/* Towards minus infinity */
#define FP_RP _FPU_RC_UP	/* Towards plus infinity */
#define FP_RZ _FPU_RC_ZERO	/* Towards zero */

#ifdef __cplusplus
}
#endif

#endif 
