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
// Specializes methods to class Builder defined general_builder_methods.hh
//---------------------------------------------------------------------

#undef ROBUST_UNMARSHALER

#undef buildValue_ROBUST
#undef buildValueRemember_ROBUST
#undef buildRepetition_ROBUST
#undef buildList_ROBUST
#undef buildListRemember_ROBUST
#undef buildDictionary_ROBUST
#undef buildDictionaryRemember_ROBUST
#undef knownChunk_ROBUST
#undef knownClass_ROBUST
#undef knownProcRemember_ROBUST
#undef buildValueOutline_ROBUST

#define buildValue_ROBUST                 buildValue
#define buildValueRemember_ROBUST         buildValueRemember
#define buildRepetition_ROBUST            buildRepetition
#define buildList_ROBUST                  buildList
#define buildListRemember_ROBUST          buildListRemember
#define buildDictionary_ROBUST            buildDictionary
#define buildDictionaryRemember_ROBUST    buildDictionaryRemember
#define knownChunk_ROBUST                 knownChunk
#define knownClass_ROBUST                 knownClass
#define knownProcRemember_ROBUST          knownProcRemember
#define buildValueOutline_ROBUST          buildValueOutline

#include "general_builder_methods.hh"
