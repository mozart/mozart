%%%  Programming Systems Lab, DFKI Saarbruecken, 
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: all
%%%  Email: wuertz@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$


%%%
%%% Load the application
%%%

\define ALONEDEMO

functor

require
   DemoUrls(image)


prepare
   ImageNames = [DemoUrls.image#'college/title.xbm']

import
   Tk
   TkTools
   Application
   FD
   Search
   OS
   Open
   Compiler
   Show
   Browse
   
define

   Images = {TkTools.images ImageNames}

   TopWindow
   ControllerLabel
   
   \insert 'College/Schedule.oz'
   
   
   {Controller}
end
