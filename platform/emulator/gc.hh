/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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

void gcSiteTable();

#endif //__GC_H__
