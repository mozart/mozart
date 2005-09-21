/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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

#ifndef __ENGINE_INTERFACE_HH
#define __ENGINE_INTERFACE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "value.hh"
#include "pstContainer.hh"

extern int ThreadIdCtr;



// Interafces to the Engine 
/* Forward dec */ 
class ProxyVar;
class LazyVar;


/* Annotations */ 

void annotateEntity(TaggedRef,int);
Bool getAnnotation(TaggedRef, int &); 
void gcAddress2InfoTables();

void gcProxyRecurseImpl(ConstTerm *ct);

void initDP(int port, int ip, const char *siteId, int primKey);

#endif



