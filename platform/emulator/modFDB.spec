###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Christian Schulte <schulte@dfki.de>
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
    #* Finite Domain Base

    'is'                => { in  => ['*value','bool'],
                             out => [],
                             bi  => BIfdIs},

    'isVar'             => { in  => ['value'],
                             out => [],
                             BI  => BIisFdVar},

    'isVarB'            => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIisFdVarB},

    'getLimits' => { in  => [],
                             out => ['+int','+int'],
                             BI  => BIgetFDLimits},

    'getMin'            => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMin},

    'getMid'            => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMid},

    'getMax'            => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMax},

    'getDom'            => { in  => ['*int','+[value]'],
                             out => [],
                             bi  => BIfdGetAsList},

    'getCard'           => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdGetCardinality},

    'getNextSmaller'    => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextSmaller},

    'getNextLarger'     => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextLarger},

    'tellConstraint'    => { in  => ['+value', 'int'],
                             out => [],
                             bi  => BIfdTellConstraint},

    'tellBoolConstraint'=> { in  => ['int'],
                             out => [],
                             bi  => BIfdBoolTellConstraint},

    'tellDeclConstraint'=> { in  => ['int'],
                             out => [],
                             bi  => BIfdDeclTellConstraint},

    'watchSize' => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchSize},

    'watchMin'  => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMin},

    'watchMax'  => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMax},

    'constrDisjSetUp'   => { in  => ['+value','+value','+value','+value'],
                             out => [],
                             ifdef => FDCD,
                             bi  => BIfdConstrDisjSetUp},

    'constrDisj'        => { in  => ['+value','+value','+value'],
                             out => [],
                             ifdef => FDCD,
                             bi  => BIfdConstrDisj},

    'tellConstraintCD'=> { in  => ['value','value','value'],
                             out => [],
                             ifdef => FDCD,
                             bi  => BIfdTellConstraintCD},

    'debugStable'       => { in  => [],
                             out => [],
                             bi  => debugStable,
                             ifdef =>DEBUG_STABLE},

    'resetStable'       => { in  => [],
                             out => [],
                             bi  => resetStable,
                             ifdef =>DEBUG_STABLE},

 );
