###
### Authors:
###   Tobias Müller <tmueller@ps.uni-sb.de>
###
### Copyright:
###   Tobias Müller, 1999
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

$module_init_fun_name = "module_init_ri";

$module_init_fun      = "oz_init_module";

$module_name          = "RI";

%builtins_all = 
(
 #
 # Built-ins
 #
 
 'newVar'		=> { in  => ['+float','+float','float'],
			     out => [],
			     bi  => ri_newVar,
                             fcp => ignore},

 'declVar'		=> { in  => ['float'],
			     out => [],
			     bi  => ri_declVar,
                             fcp => ignore},

 'setPrecision'		=> { in  => ['float'],
			     out => [],
			     bi  => ri_setPrecision,
                             fcp => ignore},

 'getLowerBound'	=> { in  => ['float', '+float'],
			     out => [],
			     bi  => ri_getLowerBound,
                             fcp => ignore},

 'getUpperBound'	=> { in  => ['float', '+float'],
			     out => [],
			     bi  => ri_getUpperBound,
                             fcp => ignore},

 'getWidth'		=> { in  => ['float', '+float'],
			     out => [],
			     bi  => ri_getWidth,
                             fcp => ignore},

 'getInf'		=> { in  => ['+float'],
			     out => [],
			     bi  => ri_getInf,
                             fcp => ignore},

 'getSup'		=> { in  => ['+float'],
			     out => [],
			     bi  => ri_getSup,
                             fcp => ignore},
		
 #
 # Propagators
 #

 'plus'		=> { in  => ['float','float','float'],
			     out => [],
			     bi  => ri_plus},
 
 'times'		=> { in  => ['float','float','float'],
			     out => [],
			     bi  => ri_times},
 
 'lessEq'		=> { in  => ['float','float'],
			     out => [],
			     bi  => ri_lessEq},
 
 'greater'		=> { in  => ['float','float'],
			     out => [],
			     bi  => ri_greater},
 
 'intBounds'		=> { in  => ['float','float'],
			     out => [],
			     bi  => ri_intBounds},
 
 'intBoundsSPP'		=> { in  => ['float','float'],
			     out => [],
			     bi  => ri_intBoundsSPP},
 );
