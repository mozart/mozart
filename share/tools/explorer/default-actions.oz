%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   InfoActions = [add(information:
			 proc {$ N P}
			    {Show N#{P}}
			 end
		      label: 'Show')
		  add(information:
			 proc {$ N P}
			    {Browse N#{P}}
			 end
		      label: 'Browse')
		  add(information:separator)]

   local

      local
	 fun {Close Fs}
	    case {System.isVar Fs} then nil
	    else !Fs=F|Fr in F|{Close Fr}
	    end
	 end
      in
	 fun {OFSArity R}
	    OpenArity = {RecordC.monitorArity R Flag}
	    Flag      = !True
	 in
	    {Close OpenArity}
	 end
      end
      
      
      local
	 proc {MergeTuple I X Y T OE ON}
	    case I==0 then true
	    else T.I={MergeTree X.I Y.I OE ON} {MergeTuple I-1 X Y T OE ON}
	    end
	 end
	 
	 proc {MergeRecord As X Y R OE ON}
	    case As of nil then true
	    [] A|Ar then
	       R.A={MergeTree X.A Y.A OE ON} {MergeRecord Ar X Y R OE ON}
	    end
	 end
	 
      in
	 fun {MergeTree X Y OE ON}
	    if X=Y then {OE X}
	    [] true then
	       case {System.isVar X} orelse {System.isVar Y} then {ON X Y}
	       else XT={Value.type X} in
		  case XT=={Value.type Y} then
		     case XT
		     of tuple  then L={Label X} W={Width X} in
			case L=={Label Y} andthen W=={Width Y} then
			   NT={MakeTuple L W} in {MergeTuple W X Y NT OE ON} NT
			else {ON X Y}
			end
		     [] record then
			L={Label X} W={Width X}
		     in
			case L=={Label Y} andthen W=={Width Y} then
			   A={Arity X}
			in
			   case A=={Arity Y} then
			      NR={MakeRecord L A} in
			      {MergeRecord A X Y NR OE ON} NR
			   else {ON X Y}
			   end
			else {ON X Y}
			end
		     else {ON X Y}
		     end
		  else {ON X Y}
		  end
	       end
	    end
	 end
      end

      fun {Equal X} X end

      fun {NotEqual X Y} X#Y end
   
      fun {Merge P1 P2}
	 {MergeTree {P1} {P2} Equal NotEqual}
      end

   in

      CmpActions = [add(compare:
			   proc {$ N1 P1 N2 P2}
			      {Show N1#N2#{Merge P1 P2}}
			   end
			label: 'Show Merge')
		    add(compare:
			   proc {$ N1 P1 N2 P2}
			      {Browse N1#N2#{Merge P1 P2}}
			   end
			label: 'Browse Merge')
		    add(compare:separator)]



   end

   local

      NormalFont = !FontFamily#140#!FontMatch
      BoldFont   = !BoldFontFamily#140#!FontMatch

      fun {Statistics Number DepthN ChoicesN SolsN FailsN UnsN}
	 Window = {New Tk.toplevel tkInit(title:'Statistics '#' ('#Number#')'
					  relief:sunken)}
	 Upper = {New Tk.frame tkInit(parent:Window bd:Border relief:raised
				      highlightthickness:0)}
      
	 Depth      = {New Tk.frame tkInit(parent:Upper
					   highlightthickness:0)}
	 DepthLabel = {New Tk.label tkInit(parent:Depth anchor:w
					   font: NormalFont
					   text:'Depth:')}
	 DepthPrint = {New Tk.label tkInit(parent:Depth anchor:e
					   width:3
					   font: BoldFont
					   text:DepthN)}
      
	 Choices      = {New Tk.frame tkInit(parent:Upper
					     highlightthickness:0)}
	 ChoicesCanvas = {New Images.choice init(parent:Choices)}
	 ChoicesLabel = {New Tk.label tkInit(parent:Choices anchor:w
					     font: NormalFont
					     text:'Choice Nodes:')}
	 ChoicesPrint = {New Tk.label tkInit(parent:Choices anchor:e
					     width:3
					     font: BoldFont
					     text:ChoicesN)}
      
	 Sols      = {New Tk.frame tkInit(parent:Upper
					  highlightthickness:0)}
	 SolsCanvas = {New Images.solved init(parent:Sols)}
	 SolsLabel = {New Tk.label tkInit(parent:Sols anchor:w
					  font: NormalFont
					  text:'Solution Nodes:')}
	 SolsPrint = {New Tk.label tkInit(parent:Sols anchor:e
					  width:3
					  font: BoldFont
					  text:SolsN)}
      
	 Fails      = {New Tk.frame tkInit(parent:Upper
					   highlightthickness:0)}
	 FailsCanvas = {New Images.failed init(parent:Fails)}
	 FailsLabel = {New Tk.label tkInit(parent:Fails anchor:w
					   font: NormalFont
					   text:'Failure Nodes:')}
	 FailsPrint = {New Tk.label tkInit(parent:Fails anchor:e
					   width:3
					   font: BoldFont
					   text:FailsN)}

	 Uns      = {New Tk.frame tkInit(parent:Upper
					 highlightthickness:0)}
	 UnsCanvas = {New Images.unstable init(parent:Uns)}
	 UnsLabel = {New Tk.label tkInit(parent:Uns anchor:w
					 font: NormalFont
					 text:'Unstable Nodes:')}
	 UnsPrint = {New Tk.label tkInit(parent:Uns anchor:e
					 width:3
					 font: BoldFont
					 text:UnsN)}
      
	 Lower = {New Tk.frame tkInit(parent:Window bd:Border relief:raised
				      highlightthickness:0)}
	 Center = {New Tk.frame tkInit(parent:Lower bd:Border relief:sunken
				       highlightthickness:0)}
	 Button = {New Tk.button tkInit(parent:Center relief:raised bd:Border
					text:'Dismiss'
					action: proc {$}
						   {Window close}
						end)}
	 
      in
	 {Tk.batch [pack(DepthLabel
			 o(side:left fill:x expand:yes padx:Pad pady:Pad))
		    pack(DepthPrint
			 o(side:right padx:Pad pady:Pad))
		    pack(ChoicesCanvas ChoicesLabel
			 o(side:left fill:x expand:yes padx:Pad pady:Pad))
		    pack(ChoicesPrint
			 o(side:right padx:Pad pady:Pad))
		    pack(SolsCanvas SolsLabel
			 o(side:left fill:x expand:yes padx:Pad pady:Pad))
		    pack(SolsPrint
			 o(side:right padx:Pad pady:Pad))
		    pack(FailsCanvas FailsLabel
			 o(side:left fill:x expand:yes padx:Pad pady:Pad))
		    pack(FailsPrint
			 o(side:right padx:Pad pady:Pad))
		    pack(UnsCanvas UnsLabel
			 o(side:left fill:x expand:yes padx:Pad pady:Pad))
		    pack(UnsPrint
			 o(side:right padx:Pad pady:Pad))
		    pack(Depth Choices Sols
			 case UnsN>0 then o(Fails Uns) else Fails end
			 o(side:top expand:yes fill:x padx:Pad pady:Pad))
		    pack(Upper
			 o(side:top fill:both))
		    pack(Center o(padx:BigPad pady:BigPad))
		    pack(Button o(padx:BigPad pady:BigPad))
		    pack(Lower o(side:bottom fill:both))]}
	 {Tk.batch [wm(iconname   Window TitleName)
		    wm(iconbitmap Window BitMap)
		    wm(resizable Window 0 0)
		    focus(Window)]}
	 {Window tkBind(event:'<Return>'
			action: proc {$} {Window close} end)}
	 Window
      end
   
   in

      StatActions = [add(statistics:
			    proc {$ N S}
			       {Show N#{Record.subtract S shape}}
			    end
			 label: 'Show')
		     add(statistics:
			    proc {$ N S}
			       {Browse N#{Record.subtract S shape}}
			    end
			 label: 'Browse')
		     add(statistics:separator)
		     add(statistics:
			    proc {$ N S C}
			       C = {Statistics N
				    S.depth S.choice S.solved S.failed
				    S.unstable}
			    end
			 label: 'Display Window')
		     add(statistics:separator)]

   end
   
in

   {Append InfoActions {Append CmpActions StatActions}}

end
