###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Christian Schulte <schulte@ps.uni-sb.de>
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
     'new'	       => { in  => ['+int','+int'],
			    out => ['+bitArray'],
			    BI  => BIbitArray_new},

     'is'	       => { in  => ['+value'],
			    out => ['+bool'],
			    BI  => BIbitArray_is},

     'set'	       => { in  => ['+bitArray','+int'],
			    out => [],
			    BI  => BIbitArray_set},

     'clear'	        => { in  => ['+bitArray','+int'],
			     out => [],
			     BI  => BIbitArray_clear},

     'test'	        => { in  => ['+bitArray','+int'],
			     out => ['+bool'],
			     BI  => BIbitArray_test},

     'low'	        => { in  => ['+bitArray'],
			     out => ['+int'],
			     BI  => BIbitArray_low},

     'high'	        => { in  => ['+bitArray'],
			     out => ['+int'],
			     BI  => BIbitArray_high},

     'clone'	        => { in  => ['+bitArray'],
			     out => ['+bitArray'],
			     BI  => BIbitArray_clone},

     'disj'               => { in  => ['+bitArray','+bitArray'],
			     out => [],
			     BI  => BIbitArray_or},

     'conj'              => { in  => ['+bitArray','+bitArray'],
			     out => [],
			     BI  => BIbitArray_and},

     'card'	        => { in  => ['+bitArray'],
			     out => ['+int'],
			     BI  => BIbitArray_card},

     'disjoint'	        => { in  => ['+bitArray','+bitArray'],
			     out => ['+bool'],
			     BI  => BIbitArray_disjoint},

     'subsumes'	        => { in  => ['+bitArray','+bitArray'],
			     out => ['+bool'],
			     BI  => BIbitArray_subsumes},

     'nimpl'	        => { in  => ['+bitArray','+bitArray'],
			     out => [],
			     BI  => BIbitArray_nimpl},

     'toList'	        => { in  => ['+bitArray'],
			     out => ['+[int]'],
			     BI  => BIbitArray_toList},

     'fromList'	        => { in  => ['+[int]'],
			     out => ['+bitArray'],
			     BI  => BIbitArray_fromList},

     'complementToList'	=> { in  => ['+bitArray'],
			     out => ['+[int]'],
			     BI  => BIbitArray_complementToList},
     );
1;;
