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
		     bi  => BIisObject},

     '@'	=> { in  => ['value'],
		     out => ['value'],
		     bi  => BIat},

     '<-'	=> { in  => ['value','value'],
		     out => [],
		     bi  => BIassign},

     'ooExch'	=> { in  => ['value','value'],
		     out => ['value'],
		     bi  => BIexchange},

     'copyRecord'=>{ in  => ['+record'],
		     out => ['+record'],
		     BI  => BIcopyRecord},

     'getClass'	=> { in  => ['+object'],
	             out => ['+class'],
	             bi  => BIgetClass},

     ','	=> { in  => ['+class','+record'],
		     out => [],
		     bi  => BIcomma},

     'send'	=> { in  => ['+record','+class','+object'],
		     out => [],
		     bi  => BIsend},

     'ooGetLock'=> { in  => ['lock'],
		     out => [],
		     bi  => BIooGetLock},

     'newObject'=> { in  => ['+class'],
		     out => ['+object'],
		     bi  => BInewObject},

     'new'	=> { in  => ['+class','+record','value'],
		     out => [],
		     bi  => BINew},
     );
1;;
