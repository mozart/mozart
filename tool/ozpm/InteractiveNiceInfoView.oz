functor

import
   Global(background:Background
	  authorMogulDB
	  getLabel:GetLabel
	 )
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   String(capitalize:Capitalize) at 'x-ozlib://duchier/lib/String.ozf'

export infoView:NiceInfoView

define
   
   fun{AuthorsToString L}
      fun{AuthorIDToName ID}
	 {Global.authorMogulDB condGet({VirtualString.toAtom ID} ID $)}.name
      end
   in
      if {List.is L} then
	 {List.drop
	  {VirtualString.toString
	   {List.foldL L fun{$ S X} S#"\n"#{AuthorIDToName X} end ""}
	  } 1}
      else
	 {VirtualString.toString {AuthorIDToName L}}
      end
   end
   %%
   %%
   %%
   TitleLook={QTk.newLook}
   {TitleLook.set label(bg:red
			glue:we
			fg:white
			font:{QTk.newFont font(family:'Helvetica'
					       weight:bold
					       size:16)})}
   AuthorLook={QTk.newLook}
   {AuthorLook.set label(bg:Background
			 font:{QTk.newFont font(family:'Helvetica'
						slant:italic
						size:12)})}

   DescLook={QTk.newLook}
   {DescLook.set text(bg:Background
		      font:{QTk.newFont font(family:'Helvetica'
					     size:12)})}
					      
   
   WhiteLook={QTk.newLook}
   {WhiteLook.set label(bg:Background)}
   {WhiteLook.set td(bg:Background)}
   {WhiteLook.set lr(bg:Background)}
   
   class NiceInfoView
      feat
	 setTitle
	 handle
	 parent
      attr
	 info:unit
      meth init(Parent ST Desc)
	 self.parent=Parent
	 self.setTitle=ST
	 Desc=td(glue:nswe
		 bg:Background
		 look:WhiteLook
%		 tdscrollbar:true
		 td(glue:nswe
		    handle:self.handle
		    label(glue:nwe
			  look:TitleLook
			  feature:title)
		    lr(glue:nwe
		       feature:author
		       label(glue:w
			     feature:desc)
		       label(glue:nwe
			     anchor:nw
			     justify:left
			     look:AuthorLook
			     feature:data))
		    lr(glue:nswe
		       feature:blurb
		       label(glue:nw
			     feature:desc)
		       placeholder(feature:place
				   bg:Background
				   padx:5 pady:5
				   glue:nswe))
		))
      end
      meth display(Inf)
	 info<-Inf
	 {self.handle.title set("")}
	 {self.handle.author.desc set("")}
	 {self.handle.author.data set("")}
	 {self.handle.blurb.desc set("")}
	 {self.handle.blurb.place set(empty)}
	 if Inf==unit then
	    {self.setTitle ""}
	 else
	    Info=Inf.info
	    fun{ListToString L}
	       {List.map
		if {List.is L} then
		   {List.drop
		    {VirtualString.toString
		     {List.foldL L fun{$ S X} S#"\n"#X end ""}
		    } 1}
		else
		   {VirtualString.toString L}
		end
		fun{$ C} if C==&\n then & else C end end}
	    end
	 in
	    {self.setTitle {VirtualString.toString Info.id}}
	    {self.handle.title set({Capitalize {GetLabel Info.id}})}
	    if {CondSelect Info author unit}\=unit then 
	       {self.handle.author.desc set("Author : ")}
	       {self.handle.author.data set({AuthorsToString Info.author})}
%					   {ListToString Info.author})}
	    else
	       {self.handle.author.desc set("No author defined.")}
	    end
	    local
	       Desc=if {HasFeature Info body} andthen
		       Info.body\=unit andthen
		       {VirtualString.toString Info.body}\=nil then
		       Info.body
		    elseif {HasFeature Info blurb} andthen
		       Info.blurb\=unit andthen
		       {VirtualString.toString Info.blurb}\=nil then
		       Info.blurb
		    else
		       ""
		    end
	       Handle
	    in
	       if Desc\=nil then
		  {self.handle.blurb.desc set("Description : ")}
		  {self.handle.blurb.place
		   set(text(glue:nswe
			    look:DescLook
			    bg:Background
			    handle:Handle
			    wrap:word
			    tdscrollbar:true
			    init:{ListToString Desc}))}
		  {Handle set(state:disabled)}
	       else
		  {self.handle.blurb.desc set("No description available.")}
	       end
	    end
	 end
      end
      meth get(Info)
	 Info=@info
	 skip
      end
      meth getClass(C)
	 C=NiceInfoView
      end
   end

end
