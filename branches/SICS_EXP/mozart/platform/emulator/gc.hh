/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
*/

#ifndef __GC_H__
#define __GC_H__

/*****************************************************************************
 * make external references into heap known to gc
 *****************************************************************************/

extern Bool gc_is_running;
Bool gcStaticProtect(TaggedRef *);
Bool gcProtect(TaggedRef *);
Bool gcUnprotect(TaggedRef *);
void gcTagged(TaggedRef&,TaggedRef&);

void protectInlineCache(InlineCache *);

OZ_C_proc_proto(BIdumpThreads)
OZ_C_proc_proto(BIlistThreads)

#endif //__GC_H__




