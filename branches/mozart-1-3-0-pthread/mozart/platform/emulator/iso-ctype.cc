/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 1997
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

#include "builtins.hh"
#include "iso-ctype.hh"


const unsigned char iso_ic_tab[] = 
     { IC_C,        IC_C,        IC_C,        IC_C, 
       IC_C,        IC_C,        IC_C,        IC_C,        
       IC_C,        IC_C | IC_S, IC_C | IC_S, IC_C | IC_S, 
       IC_C | IC_S, IC_C | IC_S, IC_C,        IC_C,        
       IC_C,        IC_C,        IC_C,        IC_C,        
       IC_C,        IC_C,        IC_C,        IC_C,        
       IC_C,        IC_C,        IC_C,        IC_C,        
       IC_C,        IC_C,        IC_C,        IC_C,        
       IC_S,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,
       IC_D | IC_X, IC_D | IC_X, IC_D | IC_X, IC_D | IC_X, 
       IC_D | IC_X, IC_D | IC_X, IC_D | IC_X, IC_D | IC_X, 
       IC_D | IC_X, IC_D | IC_X, IC_P,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_U | IC_X, IC_U | IC_X, IC_U | IC_X, 
       IC_U | IC_X, IC_U | IC_X, IC_U | IC_X, IC_U,        
       IC_U,        IC_U,        IC_U,        IC_U,        
       IC_U,        IC_U,        IC_U,        IC_U,        
       IC_U,        IC_U,        IC_U,        IC_U,        
       IC_U,        IC_U,        IC_U,        IC_U,        
       IC_U,        IC_U,        IC_U,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_L | IC_X, IC_L | IC_X, IC_L | IC_X, 
       IC_L | IC_X, IC_L | IC_X, IC_L | IC_X, IC_L,        
       IC_L,        IC_L,        IC_L,        IC_L,        
       IC_L,        IC_L,        IC_L,        IC_L,        
       IC_L,        IC_L,        IC_L,        IC_L,        
       IC_L,        IC_L,        IC_L,        IC_L,        
       IC_L,        IC_L,        IC_L,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_C,        
       0,           0,           0,           0, 
       0,           0,           0,           0, 
       0,           0,           0,           0, 
       0,           0,           0,           0, 
       0,           0,           0,           0, 
       0,           0,           0,           0, 
       0,           0,           0,           0, 
       0,           0,           0,           0, 
       IC_S,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,        
       IC_P,        IC_P,        IC_P,        IC_P,        
       IC_U,        IC_U,        IC_U,        IC_U,        
       IC_U,        IC_U,        IC_U,        IC_U,        
       IC_U,        IC_U,        IC_U,        IC_U,        
       IC_U,        IC_U,        IC_U,        IC_U,        
       IC_U,        IC_U,        IC_U,        IC_U,        
       IC_U,        IC_U,        IC_U,        IC_P,        
       IC_U,        IC_U,        IC_U,        IC_U,        
       IC_U,        IC_U,        IC_U,        IC_L,        
       IC_L,        IC_L,        IC_L,        IC_L,        
       IC_L,        IC_L,        IC_L,        IC_L,        
       IC_L,        IC_L,        IC_L,        IC_L,        
       IC_L,        IC_L,        IC_L,        IC_L,        
       IC_L,        IC_L,        IC_L,        IC_L,        
       IC_L,        IC_L,        IC_L,        IC_P,        
       IC_L,        IC_L,        IC_L,        IC_L,        
       IC_L,        IC_L,        IC_L,        IC_L};


const unsigned char iso_conv_tab[]
   = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
      16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
      32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 
      48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 
      64, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
      112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 91, 92, 93, 94, 95, 
      96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 
      80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 123, 124, 125, 126, 127, 
      128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 
      144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 
      160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 
      176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 
      224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 
      240, 241, 242, 243, 244, 245, 246, 215, 248, 249, 250, 251, 252, 253, 254, 223, 
      192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203,204, 205, 206, 207, 
      208, 209, 210, 211, 212, 213, 214, 247, 216, 217, 218, 219, 220, 221, 222, 255};


// ---------------------------------------------------------------------
// Builtins
// ---------------------------------------------------------------------

#define OZ_FirstCharArg \
 TaggedRef tc = OZ_in(0);	        \
 int i;				        \
 { DEREF(tc, tc_ptr);                   \
 Assert(!oz_isRef(tc));			\
 if (oz_isVarOrRef(tc)) {               \
   return oz_addSuspendVarList(tc_ptr); \
 }                                      \
 if (!oz_isSmallInt(tc)) {              \
   oz_typeError(0,"Char");	        \
 } else {			        \
   i = tagged2SmallInt(tc);	        \
   if ((i < 0) || (i > 255)) {	        \
     oz_typeError(0,"Char");	        \
   }				        \
 } }

#define OZ_TestChar(TEST)                                            \
  OZ_FirstCharArg;                                                   \
  OZ_RETURN(oz_bool(TEST ((unsigned char) i)));

OZ_BI_define(BIcharIs,1,1) {
 oz_declareNonvarIN(0,c);
 c = oz_deref(c);
 if (!oz_isSmallInt(c)) OZ_RETURN(oz_false());
 int i = tagged2SmallInt(c);
 OZ_RETURN(oz_bool(i >=0 && i <= 255));
} OZ_BI_end

#define BI_TESTCHAR(Name,Arg) \
OZ_BI_define(Name,1,1) { OZ_TestChar(Arg); } OZ_BI_end

BI_TESTCHAR(BIcharIsAlNum,iso_isalnum)
BI_TESTCHAR(BIcharIsAlpha,iso_isalpha)
BI_TESTCHAR(BIcharIsCntrl,iso_iscntrl)
BI_TESTCHAR(BIcharIsDigit,iso_isdigit)
BI_TESTCHAR(BIcharIsGraph,iso_isgraph)
BI_TESTCHAR(BIcharIsLower,iso_islower)
BI_TESTCHAR(BIcharIsPrint,iso_isprint)
BI_TESTCHAR(BIcharIsPunct,iso_ispunct)
BI_TESTCHAR(BIcharIsSpace,iso_isspace)
BI_TESTCHAR(BIcharIsUpper,iso_isupper)
BI_TESTCHAR(BIcharIsXDigit,iso_isxdigit)


OZ_BI_define(BIcharToLower,1,1) {
  OZ_FirstCharArg;
  OZ_RETURN_INT(iso_tolower((unsigned char) i));
} OZ_BI_end

OZ_BI_define(BIcharToUpper,1,1) {
  OZ_FirstCharArg;
  OZ_RETURN_INT(iso_toupper((unsigned char) i));
} OZ_BI_end

OZ_BI_define(BIcharToAtom,1,1) {
  OZ_FirstCharArg;
  if (i) {
     char s[2]; s[0]= (char) i; s[1]='\0';
     OZ_RETURN(oz_atom(s));
  }
  OZ_RETURN(AtomEmpty);
} OZ_BI_end

OZ_BI_define(BIcharType,1,1) {
  OZ_FirstCharArg;
  TaggedRef type;
  if (iso_isupper(i))      type = AtomUpper; 
  else if (iso_islower(i)) type = AtomLower;
  else if (iso_isdigit(i)) type = AtomDigit;
  else if (iso_isspace(i)) type = AtomCharSpace;
  else if (iso_ispunct(i)) type = AtomPunct;
  else                     type = AtomOther;
  OZ_RETURN(type);
} OZ_BI_end

