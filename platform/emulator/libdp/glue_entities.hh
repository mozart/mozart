/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
 *    Erik Klintskog (erik@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand, 1998
 *    Erik Klintskog, 1998
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

#ifndef __ENTITIES_HH
#define __ENTITIES_HH

#ifdef INTERFACE
#pragma interface
#endif

// raph: This file was declaring classes ProxyVar, LazyVar, and
// ObjectVar.  ProxyVar was the implementation of distributed
// variables, and has been replaced by a genuine distribution support
// in OzVariable.  The two other classes were implementing delayed
// loading of values (Oz objects' classes).  We now use a completely
// different implementation.

#endif
