### -*- perl -*-
###
### Author:
###   Leif Kornstaedt <kornstae@ps.uni-sb.de>
###
### Copyright:
###   Leif Kornstaedt, 2000
###
### Last change:
###   $Date$ by $Author$
###   $Revision$
###
### This file is part of Mozart, an implementation of Oz 3:
###   http://www.mozart-oz.org
###
### See the file "LICENSE" or
###   http://www.mozart-oz.org/LICENSE.html
### for information on usage and redistribution 
### of this file, and for a DISCLAIMER OF ALL 
### WARRANTIES.
###

%builtins_all =
(
    'is'			=> { in  => ['+value'],
				     out => ['+bool'],
				     BI  => BIwordIs},

    'make'			=> { in  => ['+int','+int'],
				     out => ['+value'],
				     BI  => BIwordMake},

    'size'			=> { in  => ['+value'],
				     out => ['+int'],
				     BI  => BIwordSize},

    'toInt'			=> { in  => ['+value'],
				     out => ['+int'],
				     BI  => BIwordToInt},

    'toIntX'			=> { in  => ['+value'],
				     out => ['+int'],
				     BI  => BIwordToIntX},

    '+'				=> { in  => ['+value','+value'],
				     out => ['+value'],
				     BI  => BIwordPlus},

    '-'				=> { in  => ['+value','+value'],
				     out => ['+value'],
				     BI  => BIwordMinus},

    '*'				=> { in  => ['+value','+value'],
				     out => ['+value'],
				     BI  => BIwordTimes},

    'div'			=> { in  => ['+value','+value'],
				     out => ['+value'],
				     BI  => BIwordDiv},

    'mod'			=> { in  => ['+value','+value'],
				     out => ['+value'],
				     BI  => BIwordMod},

    'orb'			=> { in  => ['+value','+value'],
				     out => ['+value'],
				     BI  => BIwordOrb},

    'xorb'			=> { in  => ['+value','+value'],
				     out => ['+value'],
				     BI  => BIwordXorb},

    'andb'			=> { in  => ['+value','+value'],
				     out => ['+value'],
				     BI  => BIwordAndb},

    'notb'			=> { in  => ['+value'],
				     out => ['+value'],
				     BI  => BIwordNotb},

    '<<'			=> { in  => ['+value','+value'],
				     out => ['+value'],
				     BI  => BIwordShl},

    '>>'			=> { in  => ['+value','+value'],
				     out => ['+value'],
				     BI  => BIwordLsr},

    '~>>'			=> { in  => ['+value','+value'],
				     out => ['+value'],
				     BI  => BIwordAsr},

     '<'		        => { in  => ['+value','+value'],
				     out => ['+bool'],
				     BI  => BIwordLess},

     '=<'		        => { in  => ['+value','+value'],
				     out => ['+bool'],
				     BI  => BIwordLessEq},

     '>'		        => { in  => ['+value','+value'],
				     out => ['+bool'],
				     BI  => BIwordGreater},

     '>='		        => { in  => ['+value','+value'],
				     out => ['+bool'],
				     BI  => BIwordGreaterEq},

 );
