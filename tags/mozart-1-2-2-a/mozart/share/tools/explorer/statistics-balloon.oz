%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local
   fun {MakeBalloon X Y Stat}
      W         = {New Tk.toplevel tkInit(withdraw:true bg:black)}
      BackFrame = {New Tk.frame tkInit(parent: W
				       bg:     PopupBgColor)}

      NodeInfo  = ({New Images.choose init(parent: BackFrame
					   bg:     PopupBgColor)} |
		   {New Tk.label tkInit(parent: BackFrame
					font:   BoldStatusFont
					bg:     PopupBgColor
					text:   Stat.c)} |
		   {New Images.succeeded init(parent: BackFrame
					      bg:     PopupBgColor)} |
		   {New Tk.label tkInit(parent: BackFrame
					font:   BoldStatusFont
					bg:     PopupBgColor
					text:   Stat.s)} |
		   {New Images.failed init(parent: BackFrame
					   bg:     PopupBgColor)} |
		   {New Tk.label tkInit(parent: BackFrame
					font:   BoldStatusFont
					bg:     PopupBgColor
					text:   Stat.f)} |
		   if Stat.b>0 then
		      [{New Images.suspended init(parent: BackFrame
						bg:     PopupBgColor)}
		       {New Tk.label tkInit(parent: BackFrame
					    font:   BoldStatusFont
					    bg:     PopupBgColor
					    text:   Stat.b)}]
		   else nil
		   end)
      
      TextInfo = [{New Tk.label tkInit(parent: BackFrame
				       text:   ' Start:'
				       bg:     PopupBgColor
				       font:   StatusFont)}
		  {New Tk.label tkInit(parent: BackFrame
				       text:   Stat.start
				       bg:     PopupBgColor
				       font:   BoldStatusFont)}
		  {New Tk.label tkInit(parent: BackFrame
				       text:   'Depth:'
				       bg:     PopupBgColor
				       font:   StatusFont)}
		  {New Tk.label tkInit(parent: BackFrame
				       text:   Stat.depth
				       bg:     PopupBgColor
				       font:   BoldStatusFont)}]
   in
      {Tk.batch [pack(b(NodeInfo) b(TextInfo) side:left fill:x padx:1 pady:1)
		 pack(BackFrame expand:true fill:x padx:1 pady:1)
		 wm(overrideredirect W true)
		 wm(geometry W '+'#X#'+'#Y)
		 update(idletasks)
		 wm(deiconify W)]}
      W
   end
   
   class StatClass
      prop final
      attr Token:unit
      meth init
	 Token <- unit
      end
      meth popup(M X Y)
	 NT
      in
	 Token <- NT
	 thread
	    [PX PY] = {Map [pointerx pointery]
		       fun {$ WI}
			  {Tk.returnInt winfo(WI
					      M.canvas)}
		       end}
	 in
	    {WaitOr NT {Alarm PopUpDelay}}
	    if {IsFree NT} then
	       W={MakeBalloon PX+10 PY+10
		  {M getStatisticsByXY(X Y $)}}
	    in
	       {Wait NT}
	       {W tkClose} 
	    end
	 end
      end
      meth close
	 @Token = unit
      end
   end

in

   fun {NewBalloonPort}
      StatServer = {New StatClass init}
      Stream
   in
      thread
	 {ForAll Stream StatServer}
      end
      {NewPort Stream}
   end

end
