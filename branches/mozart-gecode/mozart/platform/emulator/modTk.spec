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

%builtins_all =
(
    ##* Tk

    'init'	        => { in     => ['value','value','value'],
			     out    => [],
			     BI     => BItk_init},

    'send'		=> { in     => ['!value'],
			     out    => [],
			     BI     => BItk_write},

    'return'		=> { in     => ['!value','value','value'],
			     out    => [],
			     BI     => BItk_writeReturn},

    'returnMess'	=> { in     => ['!value','value','value','value'],
			     out    => [],
			     BI     => BItk_writeReturnMess},

    'batch'		=> { in     => ['!value'],
			     out    => [],
			     BI     => BItk_writeBatch},

    'sendTuple'	=> { in     => ['!value','value'],
			     out    => [],
			     BI     => BItk_writeTuple},

    'sendTagTuple'	=> { in     => ['!value','value','value'],
			     out    => [],
			     BI     => BItk_writeTagTuple},

    'sendFilter'	=> { in     => ['!value','value','value',
				        'value','value'],
			     out    => [],
			     BI     => BItk_writeFilter},

    'close'		=> { in     => ['!value','value'],
			     out    => [],
			     BI     => BItk_close},

    'genTopName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BItk_genTopName},

    'genWidgetName'	=> { in     => ['value'],
			     out    => ['value'],
			     BI     => BItk_genWidgetName},

    'genTagName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BItk_genTagName},

    'genVarName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BItk_genVarName},

    'genImageName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BItk_genImageName},

    'genFontName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BItk_genFontName},

    'getNames'		=> { in     => [],
			     out    => ['value','value','value'],
			     BI     => BItk_getNames},


    'addGroup'		=> { in     => ['+value','value'],
			     out    => ['value'],
			     BI     => BItk_addGroup},

    'delGroup'		=> { in     => ['value'],
			     out    => [],
			     BI     => BItk_delGroup},

 );


