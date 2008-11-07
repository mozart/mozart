/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *     Alejandro Arbelaez (aarbelaez@cic.puj.edu.co)
 *
 *  Contributing authors:
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
 *
 *  Copyright:
 *    Alberto Delgado, 2006-2007
 *    Alejandro Arbelaez, 2006-2007
 *    Gustavo Gutierrez, 2006-2007
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */


#ifndef __GEOZ_MISC_HH__
#define __GEOZ_MISC_HH__

/**
   Macro used for raise an exception when gecode does
*/
#define RAISE_GE_EXCEPTION(e)			\
  return OZ_raiseC((char*)e.what(),0);


#ifdef DEBUG_CHECK
#define GEOZ_DEBUG_PRINT(ARGS) printf ARGS; fflush(stdout);
#define ASSERT(Cond)					\
  if (! (Cond)) {					\
    fprintf(stderr,"%s:%d ",__FILE__,__LINE__);		\
    fprintf(stderr, " assertion '%s' failed", #Cond);	\
    abort();						\
  }
#else
#define ASSERT(Cond)
#define GEOZ_DEBUG_PRINT(ARGS) 
#endif

/**
   Macro for suspend the propagator posting if some his parametres 
   (i.e. constraint variables, domains, integers, etc) are references or ...
*/
#define SuspendPosting(Param){			\
    TaggedRef tt = (TaggedRef) (Param);		\
    DEREF(tt, t_ptr);				\
    Assert(!oz_isRef(tt));			\
    if(oz_isFree(tt)){				\
      printf("here!\n");fflush(stdout);		\
      oz_suspendOn(makeTaggedRef(t_ptr));	\
    }						\
  }


#endif
