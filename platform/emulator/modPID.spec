###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Christian Schulte <schulte@dfki.de>
###
### Copyright:
###   Denys Duchier, 1998
###   Christian Schulte, 1998
###
### Last change:
###   $Date$ by $Author$
###   $Revision$
###
### This file is part of Mozart, an implementation 
### of Oz 3:
###    http://mozart.ps.uni-sb.de
###
### See the file "LICENSE" or
###    http://mozart.ps.uni-sb.de/LICENSE.html
### for information on usage and redistribution 
### of this file, and for a DISCLAIMER OF ALL 
### WARRANTIES.
###

%builtins_all =
(
    #* Connection

    'get'	=> { in  => [],
	             out => ['+record'],
		     BI  => BIGetPID},

    'received'	=> { in  => ['value'],
		     out => [],
		     BI  => BIReceivedPID},

    'toPort'	=> { in  => ['+virtualString','+int','+int','+int'],
		     out => ['+port'],
		     BI  => BITicket2Port},

    'getCRC'	=> { in  => ['+virtualString'],
		     out => ['+int','+int'],
		     BI  => BIgetCRC},

 );


