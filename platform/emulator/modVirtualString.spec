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

# -*-perl-*-

%builtins_all =
    (
     'is'       => { in  => ['!+value'],
                     out => ['+bool'],
                     BI  => BIvsIs},

     'length'  => { in  => ['!virtualString','!+int'],
                    out => ['+int'],
                    BI  => BIvsLength},

     'toByteString'     => { in  => ['!+virtualString','!+int',
                                     '+virtualString'],
                             out => ['+byteString'],
                             bi  => BIvsToBs},
     # compute a CRC for a virtual string.  This could be used to add a CRC
     # to connection tickets, for instance.
     'getCRC'                => { in  => ['+virtualString'],
                                  out => ['+int'],
                                  BI  => BIvsCRC},
     'encodeBase64' => {
         in => ['+virtualString'],
         out => ['+string'],
         BI => BIvsEncodeB64,
         },
     'decodeBase64' => {
         in => ['+virtualString'],
         out => ['+string'],
         BI => BIvsDecodeB64,
         },


     );
1;;
