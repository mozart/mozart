%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   PanelTopClosed = {NewName}

   \insert configure.oz
   
   \insert notebook.oz

   \insert labelframe.oz

   \insert pie.oz

   \insert load.oz

   \insert make-notes.oz
   
   \insert top.oz

in

   class PanelManager
      from UrObject

      attr ThisPanelTop: Unit
      
      meth open
	 case @ThisPanelTop of !Unit then
	    ThisPanelTop <- thread
			       {New PanelTop init(manager:self)}
			    end
	 elseof T then
	    thread {T deiconify} end
	 end
      end

      meth iconify
	 case @ThisPanelTop of !Unit then true elseof T then
	    thread {T iconify} end
	 end
      end
	 
      meth deiconify
	 <<PanelManager open>>
      end

      meth options
	 true
      end

      meth !PanelTopClosed
	 ThisPanelTop <- Unit
      end

      meth close
	 <<UrObject close>>
	 case @ThisPanelTop of !Unit then true elseof T then
	    thread {T close} end
	 end
      end
      
   end

end
