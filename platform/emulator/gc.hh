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

#include "types.hh"
#include "tagged.hh"

/*****************************************************************************
 * make external references into heap known to gc
 *****************************************************************************/

Bool gcProtect(TaggedRef *);
Bool gcUnprotect(TaggedRef *);


#ifdef DEBUG_CHECK
void checkInToSpace(TaggedRef term);
void regsInToSpace(TaggedRef *regs, int size);
#else
#define checkInToSpace(term)
#define regsInToSpace(regs,size)
#endif

#endif //__GC_H__
