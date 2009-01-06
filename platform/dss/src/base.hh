/*
 *  Authors:
 *    Erik Klintskog(erik@sics.se)
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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

// This file contain globals nessesary for both the dss and the msl. Thus,
// the data structures and definitions should not be specific to neither of the systems.
// general data structures that should be globally accesicble should be located in dssBase.hh
// or in mslBase.hh



// ***************** NAMING CONVENTIONS FOR THE DSS ********************
//
//  Classes:
//    Attribute = a_
//    Static    = s_
//    Methods   = m_
//    Special Attributes (Fast access to the environment) = e_
//    - If they are private they should start with _ i.e (_a_, _s_, _m_, _e_)
//
//  Class Definitions:
//    if the class is purely internal to a specific other class its name
//    should start with _
//    This is typical for "Container" classes, holding an pointer or something
//
//  Variables:
//    Global    = g_
//    - No globally static variables are used anymore since we are OO
//
//  Functions: (stand alone)
//    - These are functions which may be used anywhere in the DSS but the
//      naming limits their usage
//    Global    = gf_
//    Local     = sf_
//    Local inline = if_
//
//
//

#ifndef __BASE_HH
#define __BASE_HH

#ifdef INTERFACE
#pragma interface
#endif


#include <stdio.h> //NULL
#include "dss_enums.hh"


// Create an integer2char, taking int and BYTE*
inline void gf_integer2char(BYTE* const buf, const u32& e){
#ifdef BIG_ENDIAN_HOST
  buf[0] = e, buf[1] = (e >>  8), buf[2] = (e >> 16), buf[3] = (e >> 24);
#else
  *reinterpret_cast<u32*>(buf) = e;
#endif
}

inline u32 gf_char2integer(BYTE* const buf){
#ifdef BIG_ENDIAN_HOST
  return (buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0]);
#else
  return *reinterpret_cast<u32*>(buf);
#endif
}

void  dssError(const char* const format, ...);
void  dssAssert(const char* const  file, const int& line, const char* const condition);

#ifdef DEBUG_CHECK
#define Assert(Cond) if(!(Cond)){ dssAssert(__FILE__,__LINE__,#Cond); }
#define DebugCode(Code) Code
#else
#define Assert(Cond)
#define DebugCode(Code)
#endif // DEBUG_CHECK

// The inlining idea of dssLog was good but it seems that the
// compiler CANNOT inline variable argument functions (va needs a
// stackframe and none is created with inline).
//
// Ok, solved it by using variadic macro expansion (nice name :) and
// it's a part of the C99 standard) i.e. sacrificed "purity" (a
// macro vs inline) for speed.
//
// Note that this is a gnu 2.9X solution, if moved to later versions
// the ... would be sufficent (also applicablefor other compiler types)
// args... <==> args
// ... <==> __VA_ARGS__


extern DSS_LOG_LEVEL  g_dssLogLevel;

#ifdef DSS_LOG
void dssLog(DSS_LOG_LEVEL level, const char* const format, ...);
#else
// We want to optimize away this statement
#ifdef _MSC_VER
#define dssLog(level, __VA_ARGS__)
#else
#define dssLog(level, args...)
#endif // DSS_LOG / _MSC_VER

#endif // DSS_LOG

#endif
