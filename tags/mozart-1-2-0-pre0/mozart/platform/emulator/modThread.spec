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
     'is'	  => { in  => ['+value'],
		       out => ['+bool'],
		       BI  => BIthreadIs},

     'this'	  => { in  => [],
		       out => ['+thread'],
		       BI  => BIthreadThis},

     'suspend'	  => { in  => ['+thread'],
		       out => [],
		       BI  => BIthreadSuspend},

     'resume'	  => { in  => ['+thread'],
		       out => [],
		       BI  => BIthreadResume},

     'injectException' => { in  => ['+thread','+value'],
			    out => [],
			    BI  => BIthreadRaise},

     'preempt'	  => { in  => ['+thread'],
		       out => [],
		       BI  => BIthreadPreempt},

     'setPriority'=> { in  => ['+thread','+atom'],
		       out => [],
		       BI  => BIthreadSetPriority},

     'getPriority'=> { in  => ['+thread'],
		       out => ['+atom'],
		       BI  => BIthreadGetPriority},

     'isSuspended'=> { in  => ['+thread'],
		       out => ['+bool'],
		       BI  => BIthreadIsSuspended},

     'state'	  => { in  => ['+thread'],
		       out => ['+atom'],
		       BI  => BIthreadState},

     'create'     => { in  => ['+procedure/0'],
		       out => [],
		       BI  => BIthreadCreate},
     );
1;;
