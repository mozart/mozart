functor

import
   Global(background:Background
	  authorMogulDB
	  getLabel:GetLabel
	 )
   String(capitalize:Capitalize) at 'x-ozlib://duchier/lib/String.ozf'
   Look
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
   TitleLook  = Look.niceTitle
   AuthorLook = Look.niceAuthor
   DescLook   = Look.niceDescription
   WhiteLook  = Look.niceWhite
   
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
	 Desc=lr(glue:nswe
		    look:WhiteLook
		    bg:Background
		    handle:self.handle
		    label(glue:nwe
			  look:TitleLook
			  feature:title)
		    continue newline
		    label(
		       anchor:ne
		       feature:author_desc
		       glue:nswe)
		    label(
		       feature:author_data
		       glue:nswe
		       anchor:nw
		       justify:left
		       look:AuthorLook)
		    newline
		    label(glue:nswe
			  feature:blurb_desc)
		    placeholder(
		       feature:blurb_place
		       bg:Background
		       padx:5 pady:5
		       glue:nswe)
		    newline
		)
      end
      meth display(Inf)
	 info<-Inf
	 {self.handle.title set("")}
	 {self.handle.author_desc set("")}
	 {self.handle.author_data set("")}
	 {self.handle.blurb_desc set("")}
	 {self.handle.blurb_place set(empty)}
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
	       {self.handle.author_desc set("Author : ")}
	       {self.handle.author_data set({AuthorsToString Info.author})}
%					   {ListToString Info.author})}
	    else
	       {self.handle.author_desc set("No author defined.")}
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
		  {self.handle.blurb_desc set("Description : ")}
		  {self.handle.blurb_place
		   set(text(glue:nswe
			    look:DescLook
			    bg:Background
			    handle:Handle
			    wrap:word
			    tdscrollbar:true
			    init:{ListToString Desc}))}
		  {Handle set(state:disabled)}
	       else
		  {self.handle.blurb_desc set("No description available.")}
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
