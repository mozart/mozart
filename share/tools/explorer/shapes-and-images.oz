%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
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

   fun {FailedShape ScaledMyX ScaledMyY Scale}
      ScaledWidth = Scale * RectangleWidthF
   in
      rectangle(ScaledMyX - ScaledWidth	  ScaledMyY - ScaledWidth
		ScaledMyX + ScaledWidth   ScaledMyY + ScaledWidth)
   end

   fun {HiddenShape ScaledMyX ScaledMyY Scale}
      ScaledCircleWidth = Scale * CircleWidthF
      ScaledSpace       = Scale * HalfHorSpaceF
      BottomY           = ScaledMyY + Scale * VerSpaceF
   in
      polygon(ScaledMyX                 ScaledMyY - ScaledCircleWidth
	      ScaledMyX - ScaledSpace   BottomY
	      ScaledMyX + ScaledSpace   BottomY)
   end

   fun {ChooseShape ScaledMyX ScaledMyY Scale}
      ScaledCircleWidth = Scale * CircleWidthF
   in
      oval(ScaledMyX - ScaledCircleWidth
	   ScaledMyY - ScaledCircleWidth
	   ScaledMyX + ScaledCircleWidth
	   ScaledMyY + ScaledCircleWidth)
   end


   fun {SuspendedShape ScaledMyX ScaledMyY Scale}
      ScaledHalfWidth = Scale * SmallRectangleWidthF
      ScaledFullWidth = Scale * RectangleWidthF
      X0          = ScaledMyX - ScaledFullWidth
      X1          = ScaledMyX - ScaledHalfWidth
      X2          = ScaledMyX
      X3          = ScaledMyX + ScaledHalfWidth
      X4          = ScaledMyX + ScaledFullWidth
      Y0          = ScaledMyY - ScaledFullWidth
      Y1          = ScaledMyY - ScaledHalfWidth
      Y2          = ScaledMyY
      Y3          = ScaledMyY + ScaledHalfWidth
      Y4          = ScaledMyY + ScaledFullWidth
   in
      polygon(X0 Y0 X2 Y1 X4 Y0 X3 Y2 X4 Y4 X2 Y3 X0 Y4 X1 Y2)
   end

   fun {SucceededShape ScaledMyX ScaledMyY Scale}
      ScaledWidth = Scale * RhombeWidthF
      X0          = ScaledMyX - ScaledWidth
      X1          = ScaledMyX
      X2          = ScaledMyX + ScaledWidth
      Y0          = ScaledMyY - ScaledWidth
      Y1          = ScaledMyY 
      Y2          = ScaledMyY + ScaledWidth
   in
      polygon(X0 Y1 X1 Y0 X2 Y1 X1 Y2 X0 Y1)
   end


   local

      NoArg = {NewName}
      
      class Image
	 from Tk.canvas
	 
	 meth init(parent:Parent tcl:Tcl bg:Bg)
	    if Bg==NoArg then
	       Image,tkInit(parent:Parent width:ImageSize height:ImageSize
			    highlightthickness:0)
	    else
	       Image,tkInit(parent:Parent width:ImageSize height:ImageSize
			    highlightthickness:0
			    bg:Bg)
	    end
	    Image,tk(crea Tcl)
	 end
      
      end

   in

      class ChooseImage
	 from Image
	 prop final
	    
	 feat Tag
	    
	 meth init(parent:Parent bg:Bg<=NoArg)
	    self.Tag = {Tk.getId}
	    Image,init(parent: Parent
		       bg:     Bg
		       tcl:    o({ChooseShape ImageCenter ImageCenter
				  ImageScale}
				 fill:    ChooseColor
				 outline: LineColor
				 width:   NodeBorderWidth
				 tags:    self.Tag))
	 end
	 meth finish
	    ChooseImage,tk(itemconf self.Tag
			   fill:ChooseTermColor width:ThickNodeBorderWidth)
	 end
	 meth clear
	    ChooseImage,tk(itemconf self.Tag
			   fill:ChooseColor width:NodeBorderWidth)
	 end
      end
      
      class FailedImage
	 from Image
	 prop final

	 meth init(parent:Parent bg:Bg<=NoArg)
	    Image,init(parent: Parent
		       bg:     Bg
		       tcl:    o({FailedShape ImageCenter ImageCenter
				  ImageScale}
				 fill:    FailedColor
				 outline: LineColor
				 width:   NodeBorderWidth))
	 end
      end

      class SucceededImage
	 from Image
	 prop final

	 meth init(parent:Parent bg:Bg<=NoArg)
	    Image,init(parent: Parent
		       bg:     Bg
		       tcl:    o({SucceededShape ImageCenter ImageCenter
				  ImageScale}
				 fill:    EntailedColor
				 outline: LineColor
				 width:   NodeBorderWidth))
	 end
      end

      class SuspendedImage
	 from Image
	 prop final
	    
	 meth init(parent:Parent bg:Bg<=NoArg)
	    Image,init(parent: Parent
		       bg:     Bg
		       tcl:    o({SuspendedShape ImageCenter ImageCenter
				  ImageScale}
				 fill:    SuspendedColor
				 outline: LineColor
				 width:   NodeBorderWidth))
	 end
      end

   end
   
in

   Images = images(succeeded: SucceededImage
		   suspended:   SuspendedImage
		   choose:    ChooseImage
		   failed:    FailedImage)

   Shapes = shapes(succeeded: SucceededShape
		   suspended:   SuspendedShape
		   choose:    ChooseShape
		   failed:    FailedShape
		   hidden:    HiddenShape)

end


