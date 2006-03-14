###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Christian Schulte <schulte@ps.uni-sb.de>
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
###    http://www.mozart-oz.org
###
### See the file "LICENSE" or
###    http://www.mozart-oz.org/LICENSE.html
### for information on usage and redistribution 
### of this file, and for a DISCLAIMER OF ALL 
### WARRANTIES.
###

# -*-perl-*-

%builtins_all =
    (
     'is'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIisPort},

     'new'	=> { in  => ['list'],
		     out => ['+port'],
		     BI  => BInewPort},

     'send'	=> { in  => ['+port','value'],
		     out => [],
		     BI  => BIsendPort},

     'sendRecv'	=> { in  => ['+port','value'],
		     out => ['value'],
		     BI  => BIsendRecvPort},

     );
1;;
