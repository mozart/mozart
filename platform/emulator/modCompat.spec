### -*- perl -*-
###
### Authors:
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

%builtins_all =
    (
     'importFloat'	=> { in  => ['value','value'],
			     out => ['value'],
			     BI  => compat_importFloat},

     'importName'	=> { in  => ['value','value'],
			     out => ['value'],
			     BI  => compat_importName},

     'importBuiltin'	=> { in  => ['value'],
			     out => ['value'],
			     BI  => compat_importBuiltin},

     'importClass'	=> { in  => ['value','value','value'],
			     out => ['value'],
			     BI  => compat_importClass},

     'importChunk'	=> { in  => ['value','value'],
			     out => ['value'],
			     BI  => compat_importChunk},

     'importFSetValue'	=> { in  => ['value'],
			     out => ['value'],
			     BI  => compat_importFSetValue},

     );
