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

$module_init_fun_name = "fsp_init";

$boot_module_name     = "FSP";

%builtins_all =
(
    'isIn'		=> { in  => ['int','fset','bool'],
			     out => [],
			     bi  => fsp_isIn},

    'reified.equal'	=> { in  => ['fset','fset','bool'],
			     out => [],
			     bi  => fsp_equalR},

    'reified.isIn'		=> { in  => ['int','fset','int'],
			     out => [],
			     bi  => fsp_isInR},

    'include'	=> { in  => ['int','fset'],
			     out => [],
			     bi  => fsp_include},

    'exclude'	=> { in  => ['int','fset'],
			     out => [],
			     bi  => fsp_exclude},

    'ini.match'		=> { in  => ['fset','+value'],
			     out => [],
			     bi  => fsp_match},

    'int.seq'		=> { in  => ['+value'],
			     out => [],
			     bi  => fsp_seq},

    'int.minN'		=> { in  => ['fset','+value'],
			     out => [],
			     bi  => fsp_minN},

    'int.maxN'		=> { in  => ['fset','+value'],
			     out => [],
			     bi  => fsp_maxN},

    'card'		=> { in  => ['fset','int'],
			     out => [],
			     bi  => fsp_card},

    'union'		=> { in  => ['fset','fset','fset'],
			     out => [],
			     bi  => fsp_union},

    'intersect'	=> { in  => ['fset','fset','fset'],
			     out => [],
			     bi  => fsp_intersection},

    'subsume'	=> { in  => ['fset','fset'],
			     out => [],
			     bi  => fsp_subsume},

    'disjoint'	=> { in  => ['fset','fset'],
			     out => [],
			     bi  => fsp_disjoint},

    'distinct'	=> { in  => ['fset','fset'],
			     out => [],
			     bi  => fsp_distinct},

    'monitorIn'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => fsp_monitorIn},

    'monitorOut'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => fsp_monitorOut},

    'int.min'		=> { in  => ['fset','int'],
			     out => [],
			     bi  => fsp_min},

    'int.max'		=> { in  => ['fset','int'],
			     out => [],
			     bi  => fsp_max},

    'int.convex'	=> { in  => ['fset'],
			     out => [],
			     bi  => fsp_convex},

    'diff'		=> { in  => ['fset','fset','fset'],
			     out => [],
			     bi  => fsp_diff},

    'reified.include'	=> { in  => ['int','fset','int'],
			     out => [],
			     bi  => fsp_includeR},

    'reified.bounds'	=> { in  => ['+fset','fset','int','int','int'],
			     out => [],
			     bi  => fsp_bounds},

    'reified.boundsN'	=> { in  => ['+value','+value','+value',
				     '+value','+value'],
			     out => [],
			     bi  => fsp_boundsN},

    'disjointN'	=> { in  => ['+value'],
			     out => [],
			     bi  => fsp_disjointN},

    'unionN'	=> { in  => ['+value','fset'],
			     out => [],
			     bi  => fsp_unionN},

    'partition'	=> { in  => ['+value','fset'],
			     out => [],
			     bi  => fsp_partition},

    'intersectN'	=> { in  => ['+value','fset'],
			     out => [],
			     bi  => fsp_intersectionN},

    'reified.partition'=> { in  => ['+value','fset','+value'],
			     out => [],
			     bi  => fsp_partitionReified},

 );
