%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$


local

   IncStep = 10
   IncTime = 100
   IncWait = 500

in

   class NumberEntry from Tk.frame
      feat entry min max parent action
      attr Val:0 TimeStamp:0 Increment:1
      meth tkInit(parent:P min:Min max:Max init:N<=Min width:W<=6 action:A)
	 Tk.frame, tkInit(parent:P highlightthickness:2 bd:2 relief:sunken)
	 Entry     = {New Tk.entry  tkInit(parent:             self
					   width:              W
					   font:               MediumFont
					   highlightthickness: 0
					   bd:                 0
					   bg:                 EnterColor)}
	 IncButton = {New Tk.button tkInit(parent:             self
					   highlightthickness: 0
					   bd:                 1
					   bitmap:BitMapDir#'mini-inc.xbm')}
	 DecButton = {New Tk.button tkInit(parent:             self
					   highlightthickness: 0
					   bd:                 1
					   bitmap:BitMapDir#'mini-dec.xbm')}
      in
	 {Tk.batch [grid(Entry     row:0 column:0 rowspan:2 sticky:ns)
		    grid(IncButton row:0 column:1           sticky:ns)
		    grid(DecButton row:1 column:1           sticky:ns)]}
	 {Entry     tkBind(event:'<Return>' action:self # Enter)}
	 {IncButton tkBind(event:'<ButtonPress-1>'   action:self#Inc(1))}
	 {DecButton tkBind(event:'<ButtonPress-1>'   action:self#Inc(~1))}
	 {IncButton tkBind(event:'<ButtonRelease-1>' action:self#IncStop)}
	 {DecButton tkBind(event:'<ButtonRelease-1>' action:self#IncStop)}
	 self.entry  = Entry
	 self.min    = Min
	 self.max    = Max
	 self.parent = P
	 self.action = A
	 NumberEntry, set(N)
      end
      
      meth set(I)
	 E=self.entry
      in
	 Val <- I {E tk(delete 0 'end')} {E tk(insert 0 I)}
      end

      meth Update(I)
	 NumberEntry, set(I)
	 {self.action I}
      end
      
      meth get($)
	 @Val
      end
      
      meth Enter
	 S = {self.entry tkReturn(get $)}
      in
	 case {String.isInt S} then
	    NumberEntry,Update({String.toInt S}) {Tk.send focus(self.parent)}
	 else NumberEntry,set(@Val)
	 end
      end

      meth Inc(I)
	 TimeStamp <- @TimeStamp+1
	 Increment <- I
	 NumberEntry, DoInc(@TimeStamp IncStep IncWait)
      end

      meth DoInc(TS S W)
	 case @TimeStamp==TS then
	    NewS = case S==0 then Increment <- @Increment * 10 IncStep
		   else S-1
		   end
	 in
	    NumberEntry, Update({Max case self.max of~1 then @Val + @Increment
				     elseof M then
					{Min @Val + @Increment M}
				     end
				 self.min})
	    {Delay W}
	    NumberEntry, DoInc(TS NewS IncTime)
	 else skip
	 end
      end

      meth IncStop
	 TimeStamp <- @TimeStamp + 1
      end
   end

end
