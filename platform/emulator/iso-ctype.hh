/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Programming System Lab, DFKI GmbH
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

#ifndef __ISO_C_TYPE__
#define __ISO_C_TYPE__

extern const unsigned char iso_ic_tab[];
extern const unsigned char iso_conv_tab[];

#define IC_C  1
#define IC_S  2
#define IC_D  4
#define IC_X  8
#define IC_P  16
#define IC_L  32
#define IC_U  64

#define Test(FUN, IC_FLGS) \
inline int FUN(const unsigned char c) { \
   return (iso_ic_tab[c] & (IC_FLGS));  \
}

Test(iso_islower,  IC_L)
Test(iso_isupper,  IC_U)
Test(iso_isdigit,  IC_D)
Test(iso_isxdigit, IC_X)
Test(iso_ispunct,  IC_P)
Test(iso_isspace,  IC_S)
Test(iso_iscntrl,  IC_C)
Test(iso_isalpha,  (IC_L | IC_U))
Test(iso_isalnum,  (IC_L | IC_U | IC_D))
Test(iso_isgraph,  (IC_L | IC_U | IC_D | IC_P))

inline int iso_isprint(const unsigned char c) {
  return (iso_isgraph(c) || (c == 32) || (c == 160));
}

inline unsigned char iso_tolower(const unsigned char c) {
  return (iso_isupper(c) ? iso_conv_tab[c] : c);
}

inline unsigned char iso_toupper(const unsigned char c) {
  return (iso_islower(c) ? iso_conv_tab[c] : c);
}

#endif


