###
### Authors:
###   Andreas Sundstroem (andreas@sics.se)
###   Erik Klintskog (erik@sics.se)
###
### Copyright:
###   Andreas Sundstroem, 1998
###   Erik Klintskog, 1998
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
     'siteStatistics'	=> { in  => [],
			     out => ['+[value]'],
			     BI  => BIsiteStatistics},

     'getTablesInfo'	=> { in  => [],
			     out => ['+[value]'],
			     BI  => BItablesExtract},

     'getNetInfo'	=> { in  => [],
			     out => ['+[value]'],
			     BI  => BI_DistMemInfo},

     'perdioStatistics'	=> { in  => [],
			     out => ['+record'],
			     BI  => BIperdioStatistics},
     );
