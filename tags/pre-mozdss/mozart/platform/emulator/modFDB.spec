###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Christian Schulte <schulte@ps.uni-sb.de>
###   Tobias Mueller <tmueller@ps.uni-sb.de>
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
    #* Finite Domain Base

    'is'		 => { in  => ['*value'],
			      out => ['+bool'],
			      bi  => BIfdIs},

    'isVar' 		 => { in  => ['value'],
			      out => [],
			      BI  => BIisFdVar},

    'isVarB'		 => { in  => ['value'],
			      out => ['+bool'],
			      BI  => BIisFdVarB},

    'getLimits'		 => { in  => [],
			      out => ['+int','+int'],
			      BI  => BIgetFDLimits},

    'reflect.min'        => { in  => ['*int'],
			      out => ['+int'],
			      bi  => BIfdMin},

    'reflect.mid'	 => { in  => ['*int'],
			      out => ['+int'],
			      bi  => BIfdMid},

    'reflect.max'	 => { in  => ['*int'],
			      out => ['+int'],
			      bi  => BIfdMax},

    'reflect.dom'	 => { in  => ['*int'],
			      out => ['+[value]'],
			      bi  => BIfdGetAsList},

    'reflect.size'	 => { in  => ['*int'],
			      out => ['+int'],
			      bi  => BIfdGetCardinality},

    'reflect.width'	 => { in  => ['*int'],
			      out => ['+int'],
			      bi  => BIfdWidth},

    'reflect.nextSmaller'=> { in  => ['+int','*int'],
			      out => ['+int'],
			      bi  => BIfdNextSmaller},

    'reflect.nextLarger' => { in  => ['+int','*int'],
			      out => ['+int'],
			      bi  => BIfdNextLarger},

    'int'		 => { in  => ['+value', 'int'],
			      out => [],
			      bi  => BIfdTellConstraint},

    'bool'		 => { in  => ['int'],
			      out => [],
			      bi  => BIfdBoolTellConstraint},

    'decl'		 => { in  => ['int'],
			      out => [],
			      bi  => BIfdDeclTellConstraint},

    'watch.size'	 => { in  => ['*int','+int','bool'],
			      out => [],
			      bi  => BIfdWatchSize},

    'watch.min'		 => { in  => ['*int','+int','bool'],
			      out => [],
			      bi  => BIfdWatchMin},

    'watch.max'		 => { in  => ['*int','+int','bool'],
			      out => [],
			      bi  => BIfdWatchMax},

 );
