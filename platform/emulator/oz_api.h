#include "oz.h"

#define OZ_RETURN_BOOL(X) \
OZ_RETURN((X)?OZ_true():OZ_false())

#define OZ_expectDet(ARG)			\
{						\
  if (OZ_isVariable(OZ_in(ARG)))		\
    { OZ_suspendOn(OZ_in(ARG)); }		\
}

#define OZ_declareTerm(ARG,VAR)			\
OZ_Term VAR = OZ_in(ARG);

#define OZ_declareDetTerm(ARG,VAR)		\
OZ_expectDet(ARG);				\
OZ_declareTerm(ARG,VAR);

/*
 * OZ_expectType(ARG,MSG,CHECK)
 *
 * causes the builtin to suspend until argument number ARG
 * is determined. it then uses the single argument function
 * CHECK to test that the term is of the expected type.  If not,
 * a type exception is raised, using MSG as the type name.
 */

#define OZ_expectType(ARG,MSG,CHECK)		\
OZ_expectDet(ARG);				\
if (!CHECK(OZ_in(ARG))) {			\
  return OZ_typeError(ARG,MSG);			\
}

#define OZ_expectBool(ARG)			\
OZ_expectType(ARG,"Bool",OZ_isBool)

#define OZ_expectInt(ARG)			\
OZ_expectType(ARG,"Int",OZ_isInt)

#define OZ_expectFloat(ARG)			\
OZ_expectType(ARG,"Float",OZ_isFloat)

#define OZ_expectAtom(ARG)			\
OZ_expectType(ARG,"Atom",OZ_isAtom)

#define OZ_expectBitString(ARG)			\
OZ_expectType(ARG,"BitString",OZ_isBitString)

#define OZ_expectByteString(ARG)		\
OZ_expectType(ARG,"ByteString",OZ_isByteString)

#define OZ_expectForeignPointer(ARG)		\
OZ_expectType(ARG,"ForeignPointer",OZ_isForeignPointer)

/*
 * OZ_declareType(ARG,VAR,TYPE,MSG,CHECK,COERCE)
 *
 * calls OZ_expectType (see above), and, if the call succeeds,
 * declares a variable VAR of TYPE and initializes it with
 * the value that can be obtained from the ARG argument by
 * applying the conversion function COERCE.
 */

#define OZ_declareType(ARG,VAR,TYPE,MSG,CHECK,COERCE) \
OZ_expectType(ARG,MSG,CHECK);			\
TYPE VAR = COERCE(OZ_in(ARG));

#define OZ_declareBool(ARG,VAR)			\
OZ_declareType(ARG,VAR,int,"Bool",OZ_isBool,OZ_boolToC)

#define OZ_declareInt(ARG,VAR)			\
OZ_declareType(ARG,VAR,int,"Int",OZ_isInt,OZ_intToC)

#define OZ_declareFloat(ARG,VAR)		\
OZ_declareType(ARG,VAR,double,"Float",OZ_isFloat,OZ_floatToC)

#define OZ_declareAtom(ARG,VAR)			\
OZ_declareType(ARG,VAR,CONST char*,"Atom",OZ_isAtom,OZ_atomToC)

#define OZ_declareBitString(ARG,VAR)		\
OZ_declareType(ARG,VAR,BitString*,"BitString",	\
	       OZ_isBitString,tagged2BitString)

#define OZ_declareByteString(ARG,VAR)		\
OZ_declareType(ARG,VAR,ByteString*,"ByteString",\
	       OZ_isByteString,tagged2ByteString)

#define OZ_declareForeignPointer(ARG,VAR)	\
OZ_declareType(ARG,VAR,void*,"ForeignPointer",	\
	OZ_isForeignPointer,OZ_getForeignPointer)

/*
 * OZ_declareForeignType(ARG,VAR,TYPE)
 *
 * this is a specialization of foreign pointer stuff.
 * TYPE should be a pointer type. The foreign pointer is
 * coerced to that type.
 */

#define OZ_declareForeignType(ARG,VAR,TYPE)	\
OZ_expectForeignPointer(ARG);			\
TYPE VAR = (TYPE) OZ_getForeignPointer(OZ_in(ARG));

/*
 * OZ_expectRecType(ARG,MSG,CHECK)
 *
 * When a value has components, in order to check that it is
 * well formed, we may have to wait until some or all of these
 * subcomponents are determined. CHECK (the predicate that
 * verifies that the value is of the appropriate type, is here
 * assumed to take 2 arguments: the 1st argument is the term
 * to be checked, and the second argument is a pointer to an
 * OZ_Term in which the next undetermined components will be
 * stored in case we need to wait.
 */

#define OZ_expectRecType(ARG,MSG,CHECK)		\
{						\
  OZ_Term OZ__aux__;				\
  if (!CHECK(OZ_in(ARG),&OZ__aux__)) {		\
    if (OZ__aux__ == 0) {			\
      return OZ_typeError(ARG,MSG);		\
    } else {					\
      OZ_suspendOn(OZ__aux__);			\
    }						\
  }						\
}

#define OZ_expectString(ARG)			\
OZ_expectRecType(ARG,"String",OZ_isProperString);

#define OZ_expectVirtualString(ARG)		\
OZ_expectRecType(ARG,"VirtualString",OZ_isVirtualString);

/*
 * OZ_declareRecType(ARG,VAR,TYPE,MSG,CHECK,COERCE)
 *
 * just what you would expect.  Note, however, that strings
 * and virtual strings are converted to char arrays that last
 * only till the next call to the conversion function COERCE.
 */

#define OZ_declareRecType(ARG,VAR,TYPE,MSG,CHECK,COERCE) \
OZ_expectRecType(ARG,MSG,COERCE);		\
TYPE VAR = COERCE(OZ_in(ARG));

inline char* oz_str2c(OZ_Term t) { return OZ_stringToC(t,0); }
inline char* oz_vs2c(OZ_Term t) { return OZ_vsToC(t,0); }

#define OZ_declareString(ARG,VAR)		\
OZ_declareRecType(ARG,VAR,char*,"String",	\
	OZ_isProperString,oz_str2c);

#define OZ_declareVirtualString(ARG,VAR)	\
OZ_declareRecType(ARG,VAR,char*,"VirtualString",\
	OZ_isVirtualString,oz_vs2c);

/*
 * OZ_declareVS(ARG,VAR,LEN)
 *
 * like OZ_declareVirtualString, but additionally sets LEN to
 * the size of the result.
 */

#define OZ_declareVS(ARG,VAR,LEN)		\
OZ_expectVirtualString(ARG);			\
int LEN;					\
char* VAR = OZ_vsToC(OZ_in(ARG),&LEN);
