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
     'is'	 => { in  => ['+value'],
		      out => ['+bool'],
		      bi  => BIisFloat},

     '/'	 => { in  => ['+float','+float'],
		      out => ['+float'],
		      bi  => BIfdiv},

     'exp'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIexp},

     'log'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIlog},

     'sqrt'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIsqrt},

     'sin'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIsin},

     'asin'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIasin},

     'sinh'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIsinh},

     'asinh'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIasinh},

     'cos'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIcos},

     'acos'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIacos},

     'cosh'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIcosh},

     'acosh'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIacosh},

     'tan'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BItan},

     'atan'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIatan},

     'tanh'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BItanh},

     'atanh'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIatanh},

     'ceil'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIceil},

     'floor'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIfloor},

     'round'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIround},

     'atan2'	 => { in  => ['+float','+float'],
		      out => ['+float'],
		      bi  => BIatan2},

     'fPow'	 => { in  => ['+float','+float'],
		      out => ['+float'],
		      bi  => BIfPow},

     'fMod'	 => { in  => ['+float','+float'],
		      out => ['+float'],
		      bi  => BIfMod},

     'toString'  => { in  => ['+float'],
		      out => ['+string'],
		      BI  => BIfloatToString},


     'toInt'	 => { in  => ['+float'],
		      out => ['+int'],
		      bi  => BIfloatToInt},
     );
1;;
