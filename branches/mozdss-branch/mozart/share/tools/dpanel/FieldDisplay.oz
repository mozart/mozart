functor
import
   Tk
export
   FieldDisplayClass
define
   class FieldDisplayClass from Tk.frame
      feat
	 can
	 linedict
	 lastdict
	 totaldict
	 
	 lineStart
	 numberStart
	 barStart
	 maxBarLen
      meth open(lineStart:L <= 5 numberStart:N <= 150 barStart:B <= 250
		parent:P width:W height:H) = M
	 SY 
      in
	 Tk.frame,tkInit(parent:P)
	 SY={New Tk.scrollbar tkInit(parent:self width:8)}
	 
	 self.lineStart = L 
	 self.numberStart = N
	 self.barStart = B
	 self.maxBarLen = W - self.barStart
	 self.linedict = {NewDictionary}
	 self.lastdict  = {NewDictionary}
	 self.totaldict = {NewDictionary}
	 self.can = {New Tk.canvas tkInit(parent:self width:W height:H
					  bg:white relief:sunken bd:2
					 )}
	 {Tk.addYScrollbar self.can SY}
	 {Tk.batch [grid(self.can row:0 column:0 sticky:news)
		    grid(SY row:0 column:1 sticky:ns)]}
      end
      
      meth update(S) MaxMsgs in
	 {Record.forAllInd S
	  proc{$ Ind E}
	     if  {Dictionary.member  self.linedict Ind} then
		(self.lastdict).Ind:= (self.totaldict).Ind - E
		(self.totaldict).Ind:=E
		{self.can tk(itemconfigure text:E Ind#number)}
	     else
		Line = {Length {Dictionary.entries self.linedict}} + 1
	     in
		(self.linedict).Ind:=Line
		(self.lastdict).Ind:=E
		(self.totaldict).Ind:=E
		{self.can tk(crea text self.lineStart Line * 17  text:Ind
			     anchor:nw tags:Ind#text)}
		{self.can tk(crea text self.numberStart Line * 17 text:E anchor:nw
			     tags:Ind#number)}
		{self.can tk(crea rect self.barStart  Line * 17 self.barStart
			     Line * 17 + 3 fill:blue  tags:Ind#line)}
	     end
	  end}
	 {self.can tk(configure scrollregion:q(0 0 400 ({Length {Dictionary.entries self.linedict}} + 1) * 17  ))}
	 MaxMsgs =  {FoldL {Dictionary.items self.totaldict} Max 1}
	 {ForAll {Dictionary.keys self.linedict}
	  proc{$ Key}
	     Line = self.linedict.Key  in
	     {self.can tk(coords Key#line self.barStart Line * 17
			  self.barStart + ( self.maxBarLen * self.totaldict.Key) div MaxMsgs
			  Line * 17 +3)}
	  end}
      end
   end
end


