%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local
   DarkColor       = '#828282'
   BrightColor     = '#ffffff'
   BackColor       = '#d9d9d9'

   RidgeWidth      = 2
   InnerOffset     = 4
   OuterOffset     = 0
   Delta           = RidgeWidth + InnerOffset + OuterOffset
   TextOffset      = 5
   TextIndent      = 25

   Nothing     = {NewName}

   local
      fun {Part Is Js Jr}
	 case Is of nil then Jr=nil [{Tk.string.toInt Js}]
	 [] I|Ir then
	    case I==&  then NewJs in
	       Jr=nil {Tk.string.toInt Js}|{Part Ir NewJs NewJs}
	    else NewJr in
	       Jr=I|NewJr {Part Ir Js NewJr}
	    end
	 end
      end
   in
      fun {Partition Is}
	 Js in {Part Is Js Js}
      end
   end
   
in

   class Labelframe
      from Tk.canvas
      feat
	 Tag
	 TextWidth
	 TextHeight
      attr
	 Width:   0
	 Height:  0

      meth tkInit(parent: Parent
		  text:   Text
		  width:  W
		  height: H
		  font:   Font   <= Nothing)
	 <<Tk.canvas tkInit(parent:Parent
			    highlightthickness: 0
			    yscrollincrement: 1
			    xscrollincrement: 1)>>
	 TextTag  = {New Tk.canvasTag tkInit(parent:self)}
	 X1 X2 Y1 Y2
      in
	 <<Tk.canvas tk(crea text
			Delta + TextIndent
			OuterOffset
			anchor:  nw
			justify: left
			text:    Text
			tags:    TextTag)>>
	 [X1 Y1 X2 Y2] = {Partition <<Tk.canvas tkReturn(bbox(TextTag) $)>>}
	 self.TextWidth  = X2 - X1
	 self.TextHeight = Y2 - Y1
	 Width  <- 2 * Delta + W
	 Height <- Delta + OuterOffset + InnerOffset + H + self.TextHeight
	 <<Tk.canvas tk(configure width:@Width height:@Height)>>
      end

      meth add(InnerWindow)
	 <<Tk.canvas tk(crea window
			Delta - 1
			self.TextHeight + OuterOffset + InnerOffset - 1
			anchor: nw
			width:  @Width - 2 * Delta
			height: @Height - Delta - InnerOffset -
			        OuterOffset - self.TextHeight
			window: InnerWindow)>>
	 X0 = !OuterOffset
	 X1 = X0 + 1
	 X2 = OuterOffset + RidgeWidth + TextIndent + 1
	 X3 = X2 + self.TextWidth + 1
	 X4 = @Width - OuterOffset - 2
	 X5 = X4 + 1
	 Y0 = OuterOffset + self.TextHeight div 2
	 Y1 = Y0 + 1
	 Y2 = @Height - 2
	 Y3 = Y2 + 1
      in
 	 <<Tk.canvas tk(crea line X3 Y0 X5 Y0 X5 Y3 X0 Y3 X0 Y0 X2 Y0
			fill:DarkColor)>>
 	 <<Tk.canvas tk(crea line X3 Y1 X4 Y1 X4 Y2 X1 Y2 X1 Y1 X2 Y1
			fill:BrightColor)>>
      end
      
   end

end
