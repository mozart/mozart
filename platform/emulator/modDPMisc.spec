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

# -*-perl-*-

%builtins_all =
    (
     'close' 		=> { in  => ['+int'],
			     out => [],
			     BI  => BIclose},

     'crash'		=> { in  => [],
			     out => [],
			     BI  => BIcrash,
			     doesNotReturn=>1},

     'dvset'		=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIdvset},

     'siteStatistics'	=> { in  => [],
			     out => ['+[value]'],
			     BI  => BIsiteStatistics},

     'getTablesInfo'	=> { in  => [],
			     out => ['+[value]'],
			     BI  => BItablesExtract},

     'perdioStatistics'	=> { in  => [],
			     out => ['+record'],
			     BI  => BIperdioStatistics},

     'slowNet'           => { in  => ['+int', '+int'],
			      out => [],
			      bi  => BIslowNet},

     'marshalerPerf'    => { in  => ['+value', '+int'],
			     out => [],
			     bi  => BImarshalerPerf},

     );
