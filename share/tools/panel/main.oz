%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   PanelTopClosed = {NewName}

   \insert configure.oz

   \insert discrete-scale.oz
   
   \insert runtime-bar.oz

   \insert load.oz

   \insert dialogs.oz
   
   \insert make-notes.oz
   
   \insert top.oz

in

   class PanelManager
      from BaseObject
      prop locking final
      attr ThisPanelTop:unit
      
      meth open
	 lock
	    case @ThisPanelTop==unit then
	       ThisPanelTop <- thread
				  {New PanelTop init(manager:self)}
			       end
	    else skip
	    end
	 end
      end

      meth !PanelTopClosed
	 lock
	    ThisPanelTop <- unit
	 end
      end

   end

end
