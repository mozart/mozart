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

    'is'		=> { in  => ['*value','bool'],
			     out => [],
			     bi  => BIfdIs},

    'isVar'		=> { in  => ['value'],
			     out => [],
			     BI  => BIisFdVar},

    'isVarB'		=> { in  => ['value'],
			     out => ['+bool'],
			     BI  => BIisFdVarB},

    'getLimits'	=> { in  => [],
			     out => ['+int','+int'],
			     BI  => BIgetFDLimits},

    'reflect.min'		=> { in  => ['*int','int'],
			     out => [],
			     bi  => BIfdMin},

    'reflect.mid'		=> { in  => ['*int','int'],
			     out => [],
			     bi  => BIfdMid},

    'reflect.max'		=> { in  => ['*int','int'],
			     out => [],
			     bi  => BIfdMax},

    'reflect.dom'		=> { in  => ['*int','+[value]'],
			     out => [],
			     bi  => BIfdGetAsList},

    'reflect.size'		=> { in  => ['*int','int'],
			     out => [],
			     bi  => BIfdGetCardinality},

    'reflect.nextSmaller'	=> { in  => ['+int','*int','int'],
			     out => [],
			     bi  => BIfdNextSmaller},

    'reflect.nextLarger'	=> { in  => ['+int','*int','int'],
			     out => [],
			     bi  => BIfdNextLarger},

    'int'	=> { in  => ['+value', 'int'],
			     out => [],
			     bi  => BIfdTellConstraint},

    'bool'=> { in  => ['int'],
			     out => [],
			     bi  => BIfdBoolTellConstraint},

    'decl'=> { in  => ['int'],
			     out => [],
			     bi  => BIfdDeclTellConstraint},

    'watch.size'	=> { in  => ['*int','+int','bool'],
			     out => [],
			     bi  => BIfdWatchSize},

    'watch.min'	=> { in  => ['*int','+int','bool'],
			     out => [],
			     bi  => BIfdWatchMin},

    'watch.max'	=> { in  => ['*int','+int','bool'],
			     out => [],
			     bi  => BIfdWatchMax},

    'constrDisjSetUp'	=> { in  => ['+value','+value','+value','+value'],
			     out => [],
			     ifdef => FDCD,
			     bi  => BIfdConstrDisjSetUp},

    'constrDisj'	=> { in  => ['+value','+value','+value'],
			     out => [],
			     ifdef => FDCD,
			     bi  => BIfdConstrDisj},

    'tellConstraintCD' => { in  => ['value','value','value'],
			     out => [],
			     ifdef => FDCD,
			     bi  => BIfdTellConstraintCD},

 );
