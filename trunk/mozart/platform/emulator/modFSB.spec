###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Tobias Mueller <tmueller@ps.uni-sb.de>
###   Christian Schulte <schulte@ps.uni-sb.de>
###
### Copyright:
###   Denys Duchier, 1998
###   Tobias Mueller, 1998
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
    'valueToString'	=> { in  => ['+fset'],
			     out => ['+string'],
			     BI  => BIfsValueToString},

    'isVarB'		=> { in  => ['value'],
			     out => ['+bool'],
			     BI  => BIfsIsVarB},

    'isValueB'	=> { in  => ['+value','bool'],
			     out => [],
			     bi  => BIfsIsValueB},

    'setValue'	=> { in  => ['+value','fset'],
			     out => [],
			     bi  => BIfsSetValue},

    'set'		=> { in  => ['+value','+value','fset'],
			     out => [],
			     bi  => BIfsSet},

    'sup'		=> { in  => [],
			     out => ['+int'],
			     BI  => BIfsSup},

    'getKnownIn'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownIn},

    'getKnownNotIn'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownNotIn},

    'getUnknown'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetUnknown},

    'getGlb'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownIn},

    'getLub'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetLub},

    'getCard'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetCard},

    'cardRange'	=> { in  => ['int','int','fset'],
			     out => [],
			     bi  => BIfsCardRange},

    'getNumOfKnownIn'	=> { in  => ['fset','int'],
			     out => [],
			     bi  => BIfsGetNumOfKnownIn},

    'getNumOfKnownNotIn'=> { in  => ['fset','int'],
			       out => [],
			       bi  => BIfsGetNumOfKnownNotIn},
    
    'getNumOfUnknown'	=> { in  => ['fset','int'],
			     out => [],
			     bi  => BIfsGetNumOfUnknown},

    'clone'		=> { in  => ['fset','fset'],
			     out => [],
			     bi  => BIfsClone},

);

