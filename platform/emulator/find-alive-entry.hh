/*
 *  Authors:
 *    Christian Schulte (schulte@dfki.de)
 *
 *  Copyright:
 *    Christian Schulte, 1998
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

#ifndef __ALIVE_ENTRY_H__
#define __ALIVE_ENTRY_H__

inline
TaggedRef findAliveEntry(TaggedRef group) {
  group = oz_deref(group);

  while (oz_isCons(group)) {
      TaggedRef ahead = oz_deref(head(group));

      if (!(oz_isLiteral(ahead) && literalEq(ahead,NameGroupVoid)))
        return group;

      group = oz_deref(tail(group));
  }

  return group;
}

#endif
