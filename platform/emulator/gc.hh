/*
 *  Authors:
 *    Author's name (Author's email address)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */
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

#ifdef CS_PROFILE
extern int32 * cs_copy_start;
extern int32 * cs_orig_start;
extern int     cs_copy_size;
#endif

extern Bool gc_is_running;
Bool gcStaticProtect(TaggedRef *);
Bool gcProtect(TaggedRef *);
Bool gcUnprotect(TaggedRef *);
void gcTagged(TaggedRef&,TaggedRef&);

void protectInlineCache(InlineCache *);

OZ_C_proc_proto(BIdumpThreads)
OZ_C_proc_proto(BIlistThreads)

#endif //__GC_H__
