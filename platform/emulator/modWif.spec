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
    ##* WIF (Wish InterFace)

    'init'              => { in     => ['value','value','value'],
                             out    => [],
                             BI     => BIwif_init},

    'write'             => { in     => ['!value'],
                             out    => [],
                             BI     => BIwif_write},

    'writeReturn'       => { in     => ['!value','value','value'],
                             out    => [],
                             BI     => BIwif_writeReturn},

    'writeReturnMess'=> { in     => ['!value','value','value','value'],
                             out    => [],
                             BI     => BIwif_writeReturnMess},

    'writeBatch'        => { in     => ['!value'],
                             out    => [],
                             BI     => BIwif_writeBatch},

    'writeTuple'        => { in     => ['!value','value'],
                             out    => [],
                             BI     => BIwif_writeTuple},

    'writeTagTuple'     => { in     => ['!value','value','value'],
                             out    => [],
                             BI     => BIwif_writeTagTuple},

    'writeFilter'       => { in     => ['!value','value','value',
                                        'value','value'],
                             out    => [],
                             BI     => BIwif_writeFilter},

    'close'             => { in     => ['!value','value'],
                             out    => [],
                             BI     => BIwif_close},

    'genTopName'        => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genTopName},

    'genWidgetName'     => { in     => ['value'],
                             out    => ['value'],
                             BI     => BIwif_genWidgetName},

    'genTagName'        => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genTagName},

    'genVarName'        => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genVarName},

    'genImageName'      => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genImageName},

    'getNames'  => { in     => [],
                             out    => ['value','value','value'],
                             BI     => BIwif_getNames},


    'addFastGroup'      => { in  => ['+value','value'],
                             out => ['value'],
                             BI  => BIaddFastGroup},

    'delFastGroup'      => { in  => ['value'],
                             out => [],
                             BI  => BIdelFastGroup},

 );
