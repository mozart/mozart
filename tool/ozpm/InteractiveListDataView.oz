functor

import
   Global(background:Background)

export dataView:ListDataView

define

   class ListDataView
      feat
	 parent
	 setTitle
	 handle
      attr
	 info
	 title
      meth init(Parent ST Desc)
	 self.parent=Parent
	 self.setTitle=ST
	 Desc=listbox(glue:nswe
		      bg:Background
		      tdscrollbar:true
		      action:self#select
		      handle:self.handle)
      end
      meth display(Info)
	 info<-{Record.adjoinAt
		Info
		info
		{List.sort Info.info
		 fun{$ A B} {VirtualString.toAtom A.id}<{VirtualString.toAtom B.id} end}}
	 {self.setTitle Info.title}
	 {self.handle set({List.map @info.info fun{$ I} I.id end})}
     end
      meth get(Info)
	 Info=@info
	 skip
      end
      meth getClass(C)
	 C=ListDataView
      end
      meth select
	 N={self.handle get(firstselection:$)}
	 D
      in
	 try
	    Info={List.nth @info.info N}
	 in
	    D=r(info:Info)
	 catch _ then D=unit end
	 {self.parent displayInfo(D)}
      end
   end

end