###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Christian Schulte <schulte@dfki.de>
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
     'wait'	        => { in  => ['+value'],
			     out => [],
			     bi  => BIisValue},

     'waitOr'		=> { in  => ['value','value'],
			     out => [],
			     BI  => BIwaitOr},

     'isFree'		=> { in  => ['value'],
			     out => ['+bool'],
			     bi  => BIisFree},

     'isKinded'		=> { in  => ['value'],
			     out => ['+bool'],
			     bi  => BIisKinded},

     'isDet'		=> { in  => ['value'],
			     out => ['+bool'],
			     bi  => BIisDet},

     'max'	        => { in  => ['+comparable','+comparable'],
			     out => ['+comparable'],
			     bi  => BImax},

     'min'	        => { in  => ['+comparable','+comparable'],
			     out => ['+comparable'],
			     bi  => BImin},

     'hasFeature'	=> { in  => ['*recordCOrChunk','+feature'],
			     out => ['+bool'],
			     bi  => BIhasFeatureB},

     'condSelect'	=> { in  => ['*recordCOrChunk','+feature','value'],
			     out => ['value'],
			     bi  => BImatchDefault},

     'byNeed'		=> { in  => ['value'],
			     out => ['value'],
			     BI  => BIbyNeed},

     'future'		=> { in  => ['value'],
			     out => ['value'],
			     BI  => BIfuture},

     '=='		=> { in  => ['*value','*value'],
			     out => ['+bool'],
			     bi  => BIeqB,
			     negated => '\\\\='},

     '\\\\='		=> { in  => ['*value','*value'],
			     out => ['+bool'],
			     bi  => BIneqB,
			     negated => '=='},

     '<'		=> { in  => ['+comparable','+comparable'],
			     out => ['+bool'],
			     bi  => BIlessFun,
			     negated => '>='},

     '=<'	        => { in  => ['+comparable','+comparable'],
			     out => ['+bool'],
			     bi  => BIleFun,
			     negated => '>'},

     '>'		=> { in  => ['+comparable','+comparable'],
			     out => ['+bool'],
			     bi  => BIgreatFun,
			     negated => '=<'},

     '>='	        => { in  => ['+comparable','+comparable'],
			     out => ['+bool'],
			     bi  => BIgeFun,
			     negated => '<'},

     '='		=> { in  => ['value','value'],
			     out => [],
			     BI  => BIunify},

     'status'	        => { in  => ['value'],
			     out => ['+tuple'],
			     bi  => BIstatus},

     'type'	        => { in  => ['+value'],
			     out => ['+atom'],
			     bi  => BItermType},
     );
1;;
