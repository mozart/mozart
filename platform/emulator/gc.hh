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

//*****************************************************************************
//    headers of functions to make external references into heap known to gc
//*****************************************************************************

// function 'gcIntroExtTerm':
//    introduce new external term reference into heap
//    returns NO if already present else OK

Bool gcProtect(TaggedRef *);


// function 'gcDismissExtTerm':
//    dismiss external term reference into heap
//    return NO if not present else OK

Bool gcUnprotect(TaggedRef *);


// for really external terms
TaggedRef gcProtectCopy(TaggedRef r);
void gcUnprotectCopy(TaggedRef *r);


#ifdef DEBUG_CHECK
void checkInToSpace(TaggedRef term);
void regsInToSpace(TaggedRef *regs, int size);
#else
#define checkInToSpace(term)
#define regsInToSpace(regs,size)
#endif

#endif //__GC_H__
