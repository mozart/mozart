
#ifndef __GEOZ_MISC_HH__
#define __GEOZ_MISC_HH__


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

#endif
