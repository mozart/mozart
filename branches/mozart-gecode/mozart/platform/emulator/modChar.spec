###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Michael Mehl <mehl@dfki.de>
###   Christian Schulte <schulte@ps.uni-sb.de>
###
### Copyright:
###   Denys Duchier, 1998
###   Michael Mehl, 1998
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
     'is'	=> { in  => ['+value'],
		     out => ['+bool'],
		     BI  => BIcharIs},

     'isAlNum'	=> { in  => ['+char'],
		     out => ['+bool'],
		     BI  => BIcharIsAlNum},

     'isAlpha'	=> { in  => ['+char'],
		     out => ['+bool'],
		     BI  => BIcharIsAlpha},

     'isCntrl'	=> { in  => ['+char'],
		     out => ['+bool'],
		     BI  => BIcharIsCntrl},

     'isDigit'	=> { in  => ['+char'],
		     out => ['+bool'],
		     BI  => BIcharIsDigit},

     'isGraph'	=> { in  => ['+char'],
		     out => ['+bool'],
		     BI  => BIcharIsGraph},

     'isLower'	=> { in  => ['+char'],
		     out => ['+bool'],
		     BI  => BIcharIsLower},

     'isPrint'	=> { in  => ['+char'],
		     out => ['+bool'],
		     BI  => BIcharIsPrint},

     'isPunct'	=> { in  => ['+char'],
		     out => ['+bool'],
		     BI  => BIcharIsPunct},

     'isSpace'	=> { in  => ['+char'],
		     out => ['+bool'],
		     BI  => BIcharIsSpace},

     'isUpper'	=> { in  => ['+char'],
		     out => ['+bool'],
		     BI  => BIcharIsUpper},

     'isXDigit'	=> { in  => ['+char'],
		    out => ['+bool'],
		    BI  => BIcharIsXDigit},

     'toLower'	=> { in  => ['+char'],
		     out => ['+char'],
		     BI  => BIcharToLower},

     'toUpper'	=> { in  => ['+char'],
		     out => ['+char'],
		     BI  => BIcharToUpper},

     'toAtom'	=> { in  => ['+char'],
		     out => ['+atom'],
		     BI  => BIcharToAtom},

     'type'	=> { in  => ['+char'],
		     out => ['+atom'],
		     BI  => BIcharType},
     );
1;;
