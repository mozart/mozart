functor

import
   Global(background       : Background
	  fileMftPkl       : FILEMFTPKL
	  fileMftTxt       : FILEMFTTXT)
   Text(strip : Strip) at 'x-ozlib://duchier/lib/String.ozf'
   Look
export infoView:InfoView

define

   KeyLook=Look.key
   ValueLook=Look.value

   fun {NormalizeWS L}
      case L of nil then nil
      [] H|T then Hs Ts in
	 if {Char.isSpace H} then
	    {List.takeDropWhile L Char.isSpace Hs Ts}
	    if {Member &\n Hs} then &\n else &  end
	    |{NormalizeWS Ts}
	 else
	    H|{NormalizeWS T}
	 end
      end
   end
   
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
			   Vs0 = {VirtualString.toString {Strip V}}
			   Vs=if {Member D [body]} then Vs0 else {NormalizeWS Vs0} end
			in
			   if Vs\="" then
			      I#label(text:D look:KeyLook)|
			      I+1#label(text:Vs look:ValueLook)|
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
				 I#label(text:X.size look:Look.mysterious anchor:ne)|
				 I+1#label(text:X.path look:Look.mysterious anchor:nw)|
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
