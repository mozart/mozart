functor

import
   Global(background       : Background
	  fileMftPkl       : FILEMFTPKL
	  fileMftTxt       : FILEMFTTXT)
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   System(show:Show)
   Browser(browse:Browse)

export infoView:InfoView

define

   KeyLook={QTk.newLook}
   {KeyLook.set label(ipadx:2 ipady:2
		      glue:news
		      anchor:ne
		      bg:lightblue
		      borderwidth:1
		      relief:raised
		     )}
   Font={QTk.newFont font(family:"Helvetica")}
   ValueLook={QTk.newLook}
   {ValueLook.set label(borderwidth:1
			font:Font
			ipadx:2
			ipady:2
			relief:raised
			glue:nswe
			anchor:nw
			background:Background
			justify:left)}
   
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
	 Desc=scrollframe(glue:nswe
			  tdscrollbar:true
			  lrscrollbar:true
			  bg:Background
			  placeholder(glue:nswe
				      handle:self.handle))
      end
      meth display(Inf)
	 info<-Inf
	 {self.handle set(empty)}
	 if Inf==unit then
	    {self.setTitle ""}
	 else
	    Info=Inf.info
	 in
	    {self.setTitle {VirtualString.toString Info.id}}
	    local
	       fun{Loop1 L I}
		  case L of X|Xs then
		     D#V=X
		  in
		     case D
		     of lsla then {Loop1 Xs I}
		     [] filelist then {Loop1 Xs I}
		     [] id then {Loop1 Xs I}
		     else
			if {List.is V} then
			   if V\=nil then
			      I#label(text:D look:KeyLook)|
			      I+1#label(text:{List.drop
					      {VirtualString.toString
					       {List.foldL V fun{$ S X} S#"\n"#X end ""}}
					      1}
					look:ValueLook)|
			      I+2#newline|
			      {Loop1 Xs I+3}
			   else
			      {Loop1 Xs I}
			   end
			elseif {VirtualString.is V} then
			   Vs={VirtualString.toString V}
			in
			   if Vs\="" andthen Vs\="/n" then
			      I#label(text:D look:KeyLook)|
			      I+1#label(text:""#V
					look:ValueLook)|
			      I+2#newline|
			      {Loop1 Xs I+3}
			   else
			      {Loop1 Xs I}
			   end
			else
			   {Loop1 Xs I}
			end
		     end
		  else
		     if {HasFeature Info lsla} then
			fun{Loop L I}
			   case L
			   of X|Xs then
			      if X.path==FILEMFTPKL orelse X.path==FILEMFTTXT then 
				 {Loop Xs I}
			      else
				 I#label(text:X.size
					 ipadx:2
					 font:Font
					 background:Background
					 anchor:ne
					 glue:nwe)|
				 I+1#label(text:X.path
					   font:Font
					   background:Background
					   anchor:nw
					   glue:nwe)|
				 I+2#newline|{Loop Xs I+3}
			      end
			   else
			      nil
			   end
			end
		     in
			I#label(text:"Included files" look:KeyLook)|
			I+1#td(borderwidth:1
			       relief:raised
			       glue:nswe
			       background:Background
			       {List.toRecord lr
				glue#nw|
				background#Background|
				{Loop Info.lsla 1}})|nil
		     else
			nil
		     end
		  end
	       end
	       L1={Loop1 {Record.toListInd Info} 4}
	       L={List.toRecord lr
		  glue#nswe|
		  background#Background|
		  1#label(text:'Key' look:KeyLook)|
		  2#label(text:'Value' look:KeyLook anchor:w)|
		  3#newline|L1}
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
