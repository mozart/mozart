%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local
   DarkColor   = '#828282'
   BrightColor = '#ffffff'
   BackColor   = '#d9d9d9'
   PieSize     = 70
   Border      = 5
   PieTop      = PieSize + Border

   ZeroTime    = time(copy:      0 
		      gc:        0 
		      load:      0
		      propagate: 0 
		      run:       0
		      system:    0
		      user:      0)
in

   class Pie
      from Tk.canvas
      feat Tag
      attr
	 Saved: ZeroTime
	 Clear: ZeroTime

      meth init(parent:P)
	 <<Pie tkInit(parent: P
		      width:  PieSize + 2 * Border
		      height: PieSize + 2 * Border)>>
	 self.Tag = {New Tk.canvasTag tkInit(parent:self)}
	 <<Pie tk(crea oval Border-2 Border-2 PieTop+2 PieTop+2
		  outline: DarkColor)>>
	 <<Pie tk(crea oval Border-1 Border-1 PieTop+1 PieTop+1
		  outline: BrightColor)>>
      end

      meth clear
	 Clear <- @Saved
	 Saved <- Unit
      end
      
      meth display(T)
	 case T==@Saved then true else
	    C        = @Clear
	    GcTime   = T.gc   - C.gc
	    CopyTime = T.copy - C.copy
	    PropTime = T.propagate - C.propagate
	    RunTime  = T.user - C.user
	    LoadTime = T.load - C.load
	    ArcTag   = self.Tag
	 in
	    {ArcTag tk(delete)}
	    case RunTime==0 then true else
	       HalfTime  = RunTime div 2
	       GcEx      = (GcTime * 360 + HalfTime)   div RunTime
	       CopyStart = GcEx
	       CopyEx    = (CopyTime * 360 + HalfTime) div RunTime
	       PropStart = CopyStart + CopyEx 
	       PropEx    = (PropTime * 360 + HalfTime) div RunTime
	       LoadStart = PropStart + PropEx 
	       LoadEx    = (LoadTime * 360 + HalfTime) div RunTime
	       RunStart  = LoadStart + LoadEx
	       RunEx     = case RunStart==0 then 359.99 else
			      360 - RunStart
			   end
	    in
	       Saved <- T
	       <<Pie tk(crea arc Border Border PieTop PieTop
			start:0         extent:GcEx
			fill:TimeColors.gc outline: ''
			tags:ArcTag)>>
	       <<Pie tk(crea arc Border Border PieTop PieTop
			start:CopyStart extent:CopyEx
			fill:TimeColors.copy outline: ''
			tags:ArcTag)>>
	       <<Pie tk(crea arc Border Border PieTop PieTop
			start:PropStart extent:PropEx
			fill:TimeColors.prop outline: ''
			tags:ArcTag)>>
	       <<Pie tk(crea arc Border Border PieTop PieTop
			start:LoadStart extent:LoadEx
			fill:TimeColors.load outline: ''
			tags:ArcTag)>>
	       <<Pie tk(crea arc Border Border PieTop PieTop
			start:RunStart extent:RunEx
			fill:TimeColors.run outline: ''
			tags:ArcTag)>>
	    end
	 end
      end
   end

end
