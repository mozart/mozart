/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $_Date$ by $_Author$
 *    $_Revision$
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



#include "dictionary.hh"

ProgramCounter copyCode(ProgramCounter start, OzDictionary *dict, Bool makecopy);
