%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   fun {FailedShape ScaledMyX ScaledMyY Scale}
      ScaledWidth = Scale * RectangleWidthF
   in
      o(rectangle
	ScaledMyX - ScaledWidth	  ScaledMyY - ScaledWidth
	ScaledMyX + ScaledWidth   ScaledMyY + ScaledWidth)
   end

   fun {HiddenShape ScaledMyX ScaledMyY Scale}
      ScaledCircleWidth = Scale * CircleWidthF
      ScaledSpace       = Scale * HalfHorSpaceF
      BottomY           = ScaledMyY + Scale * VerSpaceF
   in
      o(polygon
	ScaledMyX                 ScaledMyY - ScaledCircleWidth
	ScaledMyX - ScaledSpace   BottomY
	ScaledMyX + ScaledSpace   BottomY)
   end

   fun {ChoiceShape ScaledMyX ScaledMyY Scale}
      ScaledCircleWidth = Scale * CircleWidthF
   in
      o(oval
	ScaledMyX - ScaledCircleWidth
	ScaledMyY - ScaledCircleWidth
	ScaledMyX + ScaledCircleWidth
	ScaledMyY + ScaledCircleWidth)
   end


   fun {BlockedShape ScaledMyX ScaledMyY Scale}
      ScaledHalfWidth = Scale * SmallRectangleWidthF
      ScaledFullWidth = Scale * RectangleWidthF
      X0          = ScaledMyX - ScaledFullWidth
      X1          = ScaledMyX - ScaledHalfWidth
      X2          = !ScaledMyX
      X3          = ScaledMyX + ScaledHalfWidth
      X4          = ScaledMyX + ScaledFullWidth
      Y0          = ScaledMyY - ScaledFullWidth
      Y1          = ScaledMyY - ScaledHalfWidth
      Y2          = !ScaledMyY
      Y3          = ScaledMyY + ScaledHalfWidth
      Y4          = ScaledMyY + ScaledFullWidth
   in
      o(polygon X0 Y0 X2 Y1 X4 Y0 X3 Y2 X4 Y4 X2 Y3 X0 Y4 X1 Y2)
   end

   fun {SucceededShape ScaledMyX ScaledMyY Scale}
      ScaledWidth = Scale * RhombeWidthF
      X0          = ScaledMyX - ScaledWidth
      X1          = !ScaledMyX
      X2          = ScaledMyX + ScaledWidth
      Y0          = ScaledMyY - ScaledWidth
      Y1          = !ScaledMyY 
      Y2          = ScaledMyY + ScaledWidth
   in
      o(polygon X0 Y1 X1 Y0 X2 Y1 X1 Y2 X0 Y1)
   end


   local

      class Image
	 from Tk.canvas
	 
	 meth init(parent:Parent tcl:Tcl)
	    <<Tk.canvas tkInit(parent: Parent
			       width:  ImageSize
			       height: ImageSize)>>
	    <<Tk.canvas tk(crea Tcl)>>
	 end
      
      end

   in

      class ChoiceImage from Image
	 feat Tag
	 meth init(parent:Parent)
	    self.Tag = {Tk.server tkGet($)}
	    <<Image init(parent: Parent
			 tcl:    o({ChoiceShape ImageCenter ImageCenter
				    ImageScale}
				   o(fill:    ChoiceColor
				     outline: LineColor
				     width:   NodeBorderWidth
				     tags:    self.Tag)))>>
	 end
	 meth finish
	    <<ChoiceImage tk(itemconf self.Tag o(fill:ChoiceTermColor
						 width: TermNodeBorderWidth))>>
	 end
	 meth clear
	    <<ChoiceImage tk(itemconf self.Tag o(fill:ChoiceColor
						 width: NodeBorderWidth))>>
	 end
      end
      
      class FailedImage from Image
	 meth init(parent:Parent)
	    <<Image init(parent: Parent
			 tcl:    o({FailedShape ImageCenter ImageCenter
				    ImageScale}
				   o(fill:    FailedColor
				     outline: LineColor
				     width:   NodeBorderWidth)))>>
	 end
      end

      class SucceededImage from Image
	 meth init(parent:Parent)
	    <<Image init(parent: Parent
			 tcl:    o({SucceededShape ImageCenter ImageCenter
				    ImageScale}
				   o(fill:    EntailedColor
				     outline: LineColor
				     width:   NodeBorderWidth)))>>
	 end
      end

      class BlockedImage from Image
	 meth init(parent:Parent)
	    <<Image init(parent: Parent
			 tcl:    o({BlockedShape ImageCenter ImageCenter
				    ImageScale}
				   o(fill:    BlockedColor
				     outline: LineColor
				     width:   NodeBorderWidth)))>>
	 end
      end

   end
   
in

   Images = images(succeeded: SucceededImage
		   blocked:   BlockedImage
		   choice:    ChoiceImage
		   failed:    FailedImage)

   Shapes = shapes(succeeded: SucceededShape
		   blocked:   BlockedShape
		   choice:    ChoiceShape
		   failed:    FailedShape
		   hidden:    HiddenShape)

end


