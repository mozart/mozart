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
    'get'       => { in  => ['+literal'],
                             out => ['value'],
                             BI  => BIgetProperty},

    'condGet'   => { in  => ['+literal','value'],
                             out => ['value'],
                             BI  => BIcondGetProperty},

    'put'       => { in  => ['+literal','value'],
                             out => [],
                             BI  => BIputProperty},

 );
