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
###    http://www.mozart-oz.org
###
### See the file "LICENSE" or
###    http://www.mozart-oz.org/LICENSE.html
### for information on usage and redistribution 
### of this file, and for a DISCLAIMER OF ALL 
### WARRANTIES.
###

%builtins_all =
(
    'value.toString'		=> { in  => ['+fset'],
			             out => ['+string'],
			             BI  => BIfsValueToString},

    'var.is'			=> { in  => ['value'],
			             out => ['+bool'],
			             BI  => BIfsIsVarB},

    'value.is'	        	=> { in  => ['+value'],
			             out => ['+bool'],
			             bi  => BIfsIsValueB},

    'value.make'		=> { in  => ['+value'],
			             out => ['+fset'],
			             bi  => BIfsSetValue},

    'var.bounds'	        => { in  => ['+value','+value','fset'],
			             out => [],
			             bi  => BIfsSet},

    'sup'			=> { in  => [],
			             out => ['+int'],
			             BI  => BIfsSup},

    'getKnownIn'	        => { in  => ['fset'],
			             out => ['+value'],
			             bi  => BIfsGetKnownIn},

    'getKnownNotIn'		=> { in  => ['fset'],
			             out => ['+value'],
			             bi  => BIfsGetKnownNotIn},

    'reflect.unknown'		=> { in  => ['fset'],
			     	     out => ['+value'],
			             bi  => BIfsGetUnknown},

    'reflect.lowerBound'	=> { in  => ['fset'],
			     	     out => ['+value'],
			     	     bi  => BIfsGetKnownIn},

    'reflect.upperBound'	=> { in  => ['fset'],
			     	     out => ['+value'],
			             bi  => BIfsGetLub},

    'reflect.card'		=> { in  => ['fset'],
			             out => ['+value'],
			             bi  => BIfsGetCard},

    'cardRange'			=> { in  => ['int','int','fset'],
			             out => [],
			             bi  => BIfsCardRange},

    'reflect.cardOf.lowerBound'	=> { in  => ['fset'],
			     	     out => ['+int'],
			             bi  => BIfsGetNumOfKnownIn},

    'getNumOfKnownNotIn'	=> { in  => ['fset'],
			             out => ['+int'],
			             bi  => BIfsGetNumOfKnownNotIn},
    
    'reflect.cardOf.unknown'	=> { in  => ['fset'],
			             out => ['+int'],
			             bi  => BIfsGetNumOfUnknown},

    'fsClone'			=> { in  => ['fset','fset'],
			             out => [],
			             bi  => BIfsClone},

);

