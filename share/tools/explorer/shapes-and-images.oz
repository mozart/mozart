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
      polygon(X0 Y0 X2 Y1 X4 Y0 X3 Y2 X4 Y4 X2 Y3 X0 Y4 X1 Y2)
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
      polygon(X0 Y1 X1 Y0 X2 Y1 X1 Y2 X0 Y1)
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

      class ChooseImage from Image
	 feat Tag
	 meth init(parent:Parent)
	    self.Tag = {Tk.server tkGet($)}
	    <<Image init(parent: Parent
			 tcl:    o({ChooseShape ImageCenter ImageCenter
				    ImageScale}
				   fill:    ChooseColor
				   outline: LineColor
				   width:   NodeBorderWidth
				   tags:    self.Tag))>>
	 end
	 meth finish
	    <<ChooseImage tk(itemconf self.Tag
			     fill:ChooseTermColor width:ThickNodeBorderWidth)>>
	 end
	 meth clear
	    <<ChooseImage tk(itemconf self.Tag
			     fill:ChooseColor width:NodeBorderWidth)>>
	 end
      end
      
      class FailedImage from Image
	 meth init(parent:Parent)
	    <<Image init(parent: Parent
			 tcl:    o({FailedShape ImageCenter ImageCenter
				    ImageScale}
				   fill:    FailedColor
				   outline: LineColor
				   width:   NodeBorderWidth))>>
	 end
      end

      class SucceededImage from Image
	 meth init(parent:Parent)
	    <<Image init(parent: Parent
			 tcl:    o({SucceededShape ImageCenter ImageCenter
				    ImageScale}
				   fill:    EntailedColor
				   outline: LineColor
				   width:   NodeBorderWidth))>>
	 end
      end

      class BlockedImage from Image
	 meth init(parent:Parent)
	    <<Image init(parent: Parent
			 tcl:    o({BlockedShape ImageCenter ImageCenter
				    ImageScale}
				   fill:    BlockedColor
				   outline: LineColor
				   width:   NodeBorderWidth))>>
	 end
      end

   end
   
in

   Images = images(succeeded: SucceededImage
		   blocked:   BlockedImage
		   choose:    ChooseImage
		   failed:    FailedImage)

   Shapes = shapes(succeeded: SucceededShape
		   blocked:   BlockedShape
		   choose:    ChooseShape
		   failed:    FailedShape
		   hidden:    HiddenShape)

end


