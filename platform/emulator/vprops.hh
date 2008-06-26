/*
 *  Authors:
 *    Denys Duchier <duchier@ps.uni-sb.de>
 *
 *  Copyright:
 *    Denys Duchier, 1997
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

#ifndef __VPROPS__HH__
#define __VPROPS__HH__

#include "base.hh"

// Bogus OZ_Return values to indicate that the property is
// resp. not readable or not writable, and that the attempted
// operation should raise an error.

#define PROP__NOT__FOUND        666
#define PROP__NOT__READABLE     667
#define PROP__NOT__WRITABLE     668
#define PROP__NOT__GLOBAL       669

extern void initVirtualProperties();

class VirtualProperty {
public:
  virtual OZ_Term   get();
  virtual OZ_Return set(OZ_Term);
  void              add(const char*);
private:
  static void       add(const char*,const int);
  friend void initVirtualProperties();
};

extern OZ_Term registry_get(OZ_Term);
extern void registry_put(OZ_Term,OZ_Term);

#endif
