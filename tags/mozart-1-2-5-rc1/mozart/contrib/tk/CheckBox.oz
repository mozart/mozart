functor
import Tk
export CheckBox
define
class CheckBox from Tk.canvas
   attr Factor XOFF YOFF HOFF VOFF On:false
      BoxColor BoxWidth
      VeeColor VeeWidth
   meth init(parent  :P
	     bg      :BG <= unit
	     colorBox:CB <= slateblue
	     colorVee:CV <= red
	     widthVee:WV <= 3
	     widthBox:WB <= 10
	     hoffset :HO <= 2
	     voffset :VO <= 2)
      %% magic numbers!
      Width  = WB+HO+(WV div 2)+2
      Height = WB+2*VO+WV+2
      M1 = tkInit(parent:P width:Width height:Height
		  highlightthickness:0)
      M2 = if BG==unit then M1 else {Adjoin tkInit(bg:BG) M1} end
   in
      Tk.canvas,M2
      XOFF     <- 1
      YOFF     <- 1+VO+(WV div 2)
      BoxColor <- CB
      VeeColor <- CV
      VeeWidth <- WV
      BoxWidth <- WB
      HOFF     <- HO
      VOFF     <- VO
      CheckBox,DrawBox
      Tk.canvas,tkBind(event:'<1>' action:self#toggle)
   end
   meth DrawBox
      Tk.canvas,tk(create rectangle
		   @XOFF @YOFF @XOFF+@BoxWidth @YOFF+@BoxWidth
		   outline:@BoxColor tags:box)
   end
   meth DrawVee
      Tk.canvas,tk(create line
		   @XOFF+@HOFF
		   @YOFF+(@BoxWidth div 2)
		   @XOFF+(@BoxWidth div 2)
		   @YOFF+@BoxWidth+@VOFF
		   @XOFF+@BoxWidth+@HOFF
		   @YOFF-@VOFF
		   capstyle:round
		   joinstyle:round
		   fill:@VeeColor
		   width:@VeeWidth
		   tags:vee)
   end
   meth set(B)
      if @On\=B then
	 if B then CheckBox,DrawVee
	 else Tk.canvas,tk(delete vee) end
	 On <- B
      end
   end
   meth toggle {self action({Not @On})} end
   meth action(B) CheckBox,set(B) end
end
end

/*
declare T = {New Tk.toplevel tkInit(width:150 height:150)}
{T tk(configure bg:ivory)}
declare B = {New CheckBox init(parent:T widthBox:30 widthVee:5 hoffset:7 voffset:3)}
declare B = {New CheckBox init(parent:T)}
{Tk.send pack(B expand:true)}
{B tk(configure bg:ivory)}
{B toggle}
{B tk(configure highlightthickness:0)}
{B tkClose}
{T tkClose}
*/
