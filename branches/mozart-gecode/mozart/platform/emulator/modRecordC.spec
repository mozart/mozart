###
### Authors:
###   Christian Schulte <schulte@ps.uni-sb.de>
###
### Copyright:
###   Christian Schulte, 1999
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
     'is'	    => { in  => ['+value'],
		         out => ['+bool'],
		         bi  => BIisRecordCB},

     'tell'         => { in  => ['+literal','record'],
		         out => [],
		         BI  => BIrecordTell},

     'width'	    => { in  => ['*record','int'],
		         out => [],
		         BI  => BIwidthC},

     'hasLabel'	    => { in  => ['value'],
		         out => ['+bool'],
		         bi  => BIhasLabel},

     'monitorArity' => { in  => ['*recordC','value','[feature]'],
			out => [],
			BI  => BImonitorArity},

     'tellSize'     => { in  => ['+literal','+int','record'],
			 out => [],
		         BI  => BIsystemTellSize},

     '^'	    => { in  => ['*recordCOrChunk','+feature'],
		         out => ['value'],
		         bi  => BIofsUpArrow},

     );
1;;
