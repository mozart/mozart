%%%
%%% Author:
%%%   Thorsten Brunklaus <bruni@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Thorsten Brunklaus, 2001
%%%
%%% Last Change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor $
import
   System(show)
   GTK at 'x-oz://system/gtk/GTK.ozf'
define
   Is = ["Maximum" "Minimum" "Average" "Projection"]
   
   class MyList from GTK.list
      meth new
	 Items = {Map Is fun {$ Title}
			    {New GTK.listItem newWithLabel(Title)}
			 end}
      in
	 GTK.list, new
	 GTK.list, appendItems(Items)
	 GTK.list, signalConnect('select-child'
				 proc {$ [Item]}
				    {System.show 'handler got'#
				     GTK.list, childPosition(Item $)}
				 end _)
      end
   end

   class MyToplevel from GTK.window
      meth new
	 GTK.window, new(GTK.'WINDOW_TOPLEVEL')
	 GTK.window, setBorderWidth(10)
	 GTK.window, setTitle("List Test")
	 GTK.window, add({New MyList new})
	 GTK.window, showAll
      end
   end

   _ = {New MyToplevel new}
end
