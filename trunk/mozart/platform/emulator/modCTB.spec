###
### Authors:
###   Christian Schulte <schulte@ps.uni-sb.de>
###
### Copyright:
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


# -*-perl-*-

%builtins_all = 
    (
     #* Constraint variables
     
     'isB'	          => { in  => ['value'],
			       out => ['+bool'],
			       BI  => BIIsGenCtVarB},
     
    'getConstraintAsAtom' => { in  => ['value','atom'],
			       out => [],
			       BI  => BIGetCtVarConstraintAsAtom},
     
     'getNameAsAtom'      => { in  => ['value','atom'],
			       out => [],
			       BI  => BIGetCtVarNameAsAtom},
     
     );

