functor

import Global(background:Background)

export infoView:InfoView

define

   class InfoView
      feat
	 setTitle
	 handle
	 parent
      attr
	 info:unit
      meth init(Parent ST Desc)
	 self.parent=Parent
	 self.setTitle=ST
	 Desc=listbox(glue:nswe
		      bg:Background
		      tdscrollbar:true
		      lrscrollbar:true
		      handle:self.handle)
      end
      meth display(Inf)
	 info<-Inf
	 {self.handle set(nil)}
	 if Inf==unit then
	    {self.setTitle ""}
	 else
	    Info=Inf.info
	 in
	    {self.setTitle {VirtualString.toString Info.id}}
	    local
	       L1={List.filter
		   {List.map
		    {Record.toListInd Info}
		    fun{$ En}
		       I#V=En
		    in
		       case I
		       of lsla then unit
		       [] filelist then unit
		       [] id then unit
		       else
			  if {List.is V} then
			     I#":"#{List.drop
				    {VirtualString.toString
				     {List.foldL V fun{$ S X} S#","#X end ""}}
				    1}
			  elseif {VirtualString.is V} then
			     I#":"#{VirtualString.toString V}
			  else
			     unit
			  end
		       end
		    end}
		   fun{$ L} L\=unit end}
	       L2=if {HasFeature Info filelist} then
		     ""|"Contains the following files :"|""|{List.map Info.filelist fun{$ R} R end}
		  else
		     nil
		  end
	       L={List.append L1 L2}
	    in
	       {self.handle set(L)}
	    end
	 end
      end
      meth get(Info)
	 Info=@info
	 skip
      end
      meth getClass(C)
	 C=InfoView
      end
   end

end
