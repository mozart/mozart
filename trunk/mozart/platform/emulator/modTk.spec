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

%builtins_all =
(
    ##* Tk

    'init'	        => { in     => ['value','value','value'],
			     out    => [],
			     BI     => BIwif_init},

    'send'		=> { in     => ['!value'],
			     out    => [],
			     BI     => BIwif_write},

    'return'		=> { in     => ['!value','value','value'],
			     out    => [],
			     BI     => BIwif_writeReturn},

    'returnMess'	=> { in     => ['!value','value','value','value'],
			     out    => [],
			     BI     => BIwif_writeReturnMess},

    'batch'		=> { in     => ['!value'],
			     out    => [],
			     BI     => BIwif_writeBatch},

    'sendTuple'	=> { in     => ['!value','value'],
			     out    => [],
			     BI     => BIwif_writeTuple},

    'sendTagTuple'	=> { in     => ['!value','value','value'],
			     out    => [],
			     BI     => BIwif_writeTagTuple},

    'sendFilter'	=> { in     => ['!value','value','value',
				        'value','value'],
			     out    => [],
			     BI     => BIwif_writeFilter},

    'close'		=> { in     => ['!value','value'],
			     out    => [],
			     BI     => BIwif_close},

    'genTopName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BIwif_genTopName},

    'genWidgetName'	=> { in     => ['value'],
			     out    => ['value'],
			     BI     => BIwif_genWidgetName},

    'genTagName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BIwif_genTagName},

    'genVarName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BIwif_genVarName},

    'genImageName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BIwif_genImageName},

    'getNames'		=> { in     => [],
			     out    => ['value','value','value'],
			     BI     => BIwif_getNames},


    'addGroup'		=> { in  => ['+value','value'],
			     out => ['value'],
			     BI  => BIaddFastGroup},

    'delGroup'		=> { in  => ['value'],
			     out => [],
			     BI  => BIdelFastGroup},

 );


