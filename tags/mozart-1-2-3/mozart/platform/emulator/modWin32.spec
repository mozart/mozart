# -*- perl -*-
###
### Author:
###   Leif Kornstaedt <kornstae@ps.uni-sb.de>
###
### Copyright:
###   Leif Kornstaedt, 1999
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

$module_init_fun_name = "win32_init";

%builtins_all =
    (
     'getRegistryKey'	=> { in  => ['+virtualString','+virtualString',
				     '+virtualString'],
			     out => ['+value'],
			     BI  => win32_getRegistryKey,
			     ifdef => WINDOWS},
    );
