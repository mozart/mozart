/*
 *  Authors:
 *    Andreas Sundstrom (andreas@sics.se)
 * 
 *  Contributors:
 *
 *  Copyright:
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

//---------------------------------------------------------------------
// Specializes versions of procedures unmarshaling.cc
//---------------------------------------------------------------------

#define ROBUST_UNMARSHALER

#undef buildValue_ROBUST
#undef buildValueRemember_ROBUST
#undef buildList_ROBUST  
#undef buildListRemember_ROBUST
#undef buildDictionary_ROBUST   
#undef buildDictionaryRemember_ROBUST
#undef knownChunk_ROBUST 
#undef knownClass_ROBUST 
#undef knownProcRemember_ROBUST
#undef buildValueOutline_ROBUST

#define buildValue_ROBUST                 buildValueRobust
#define buildValueRemember_ROBUST         buildValueRememberRobust
#define buildList_ROBUST                  buildListRobust
#define buildListRemember_ROBUST          buildListRememberRobust
#define buildDictionary_ROBUST            buildDictionaryRobust
#define buildDictionaryRemember_ROBUST    buildDictionaryRememberRobust
#define knownChunk_ROBUST                 knownChunkRobust
#define knownClass_ROBUST                 knownClassRobust
#define knownProcRemember_ROBUST          knownProcRememberRobust
#define buildValueOutline_ROBUST          buildValueOutlineRobust

#include "unmarshaling.cc"
