%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local
   TownSize   = 3
   TextOffset = 11
in

   class Environment
      from Tk.canvas
      feat putSrc putDst outer
      
      meth init
	 Outer = {New Tk.frame tkInit(parent:self.frame)}
      in
	 Tk.canvas,tkInit(parent:Outer relief:sunken bd:3
			  width:  Country.width
			  height: Country.height
			  bg:     BackColor)
	 {Tk.send pack(self)}
	 self.outer = Outer
      end

      meth draw
	 Environment,DrawCities(Country.graph)
      end
      
      meth DrawCities(CityS)
	 case CityS of nil then skip
	 [] Src#Pos#DstS|CityR then
	    Tag = {New Tk.canvasTag tkInit(parent:self)}
	 in
	    {Tag tkBind(event:'<1>'
			action: self.putSrc # Src)}
	    {Tag tkBind(event:'<2>'
			action: self.putDst # Src)}
	    Environment,ConnectCity(Pos.x Pos.y DstS)
		       ,DrawCities(CityR)
		       ,tk(crea rectangle Pos.x-TownSize Pos.y-TownSize
			   Pos.x+TownSize Pos.y+TownSize
			   fill:CityColor tag:Tag)
		       ,tk(crea text Pos.x Pos.y+TextOffset
			   fill:    NameColor
			   justify: center
			   text:    Src
			   tag:     Tag)
	 end
      end
      
      meth ConnectCity(SrcX SrcY DstS)
	 case DstS of nil then skip
	 [] Dst|DstR then
	    Environment,tk(crea line SrcX SrcY Dst.x Dst.y fill:StreetColor)
		       ,ConnectCity(SrcX SrcY DstR)
	 end
      end

   end

end





