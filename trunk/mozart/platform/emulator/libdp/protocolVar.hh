/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Michael Mehl (1997)
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __protocolVar__hh__
#define __protocolVar__hh__

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"
#include "dpBase.hh"

OZ_Return sendRedirect(DSite* sd,int OTI,TaggedRef val);
OZ_Return sendRedirectToProxies(OldPerdioVar *pv, OZ_Term val,
				DSite* ackSite, int OTI);
OZ_Return sendSurrender(BorrowEntry *be,OZ_Term val);
void sendRegister(BorrowEntry*);
void sendAcknowledge(DSite* sd,int OTI);

#endif
