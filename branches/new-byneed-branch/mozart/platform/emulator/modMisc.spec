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
    ###
    ### Statistics
    ###
        
    'instructionsPrint'	=> { in  => [],
			     out => [],
			     BI  => BIinstructionsPrint,
			     ifdef=>'PROFILE_INSTR'},

    'instructionsPrintCollapsable' => { in  => [],
			     out => [],
			     BI  => BIinstructionsPrintCollapsable,
			     ifdef=>'PROFILE_INSTR'},

    'instructionsPrintReset' => { in  => [],
			     out => [],
			     BI  => BIinstructionsPrintReset,
			     ifdef=>'PROFILE_INSTR'},

    'biPrint'		=> { in  => [],
			     out => [],
			     BI  => BIbiPrint,
			     ifdef=>'PROFILE_BI'},

    ###
    ### Ralf's private stuff
    ###

    'funReturn'		=> { in  => ['value'],
			     out => [],
			     doesNotReturn => 1,
			     BI  => BIfunReturn},

    'getReturn'		=> { in  => [],
			     out => ['value'],
			     BI  => BIgetReturn},


    ###
    ### Tobias's private stuff
    ###


 );
