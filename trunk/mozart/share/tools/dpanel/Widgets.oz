%%%
%%% Authors:
%%%   Nils Franzen <nilsf@sics.se>
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

functor
import
   Tk
export
   CardFrame
   Toplevel
define
   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% Cardlayout
   class CardFrame from Tk.frame
      feat AddCard RemoveCard showCard
      prop final
      meth tkInit(parent:T padx:PADX<=0 pady:PADY<=0 width:CWidth<=400 height:CHeight<=300 ...)=M
	 Tk.frame, {Record.subtract {Record.subtract {Record.subtract {Record.subtract M padx} pady} height} width}
	 MCX=2+PADX MCY=PADY
	 CardLock={NewLock}
	 SYVisible={NewCell true}
	 LastX={NewCell 0}
	 LCol=white  DCol= '#a9a9a9'
	 Y=30 DX=5 W=3 H=3 SC=2
	 DB={NewDictionary}
	 DC={New Tk.canvas tkInit(parent:self height:Y+1 bd:0 highlightthickness:0)}
	 SY={New Tk.scrollbar tkInit(parent:self width:8 orient:horizontal)}
	 SelectedCard={NewCell unit}
	 proc{Layout}
	    Cs={Sort {Dictionary.items DB} fun{$ O N} O.id<N.id end}
	    OS NS={Exchange SelectedCard OS}
	 in
	    if OS==unit andthen Cs\=nil then
	       NS=Cs.1
	       {NS.frametag tk(move CWidth+MCX CWidth+MCY)}
	    else
	       OS=NS
	    end
	    {Assign LastX DX}
	    {ForAll Cs proc{$ X} O N CX in
			  {Exchange LastX O N}
			  CX=O-X.currentx
			  {DC tk(move X.movetag CX 0)}
			  N=O+X.width+3
			  {Dictionary.put DB X.id card(id:X.id name:X.name movetag:X.movetag
						       frame:X.frame frametag:X.frametag
						       width:X.width currentx:O)}
		       end}
	     {DC tk(config scrollregion:q(0 0 {Access LastX} Y))}
	    if OS==unit then F={Dictionary.get DB NS.id} in
	       {DC tk(coords lline 0 Y F.currentx Y)}
	       {DC tk(coords rline F.currentx+F.width Y CWidth Y)}
	    end 
	    {DC tk('raise' lline)}
	    {DC tk('raise' rline)}
	 end
	 proc{ShowCard C}
	    OS NS={Exchange SelectedCard OS}
	    E={Dictionary.get DB C}
	 in
	    {OS.frametag tk(move ~CWidth-MCX ~CWidth-MCY)}
	    {DC tk(coords lline 0 Y E.currentx Y)}
	    {DC tk(coords rline E.currentx+E.width Y CWidth Y)}
	    {E.frametag tk(move CWidth+MCX CWidth+MCY)}
	    NS=E
	 end
	 DCC={New Tk.canvas tkInit(parent:self bd:0 highlightthickness:0 width:CWidth height:CHeight)}
      in
	 %% Top side
	 {DC tk(crea line
		0 Y 1 Y
		width:2
		fill:LCol
		tags:lline)}
	 {DC tk(crea line
		2 Y-1 CWidth Y-1
		width:2
		fill:LCol
		tags:rline)}

	 %% Left side
	 {DCC tk(crea line
		 0 0
		 0 100
		 fill:LCol
		 width:2
		 tags:dborder)}
	 {DCC tk(crea line
		 0 100
		 100 100
		 100 0
		 fill:DCol
		 width:2
		 tags:lborder)}
	 {DCC tkBind(event:'<Configure>'
		     action:proc{$}
			       lock CardLock then
				  W1={Tk.returnInt winfo(width(DCC))}-1 W=W1-3-(2*PADX)
				  H1={Tk.returnInt winfo(height(DCC))}-1 H=H1-1-(2*PADY)
			       in
				  {DCC tk(coords dborder
					  1 0
					  1 H1)}
				  {DCC tk(coords lborder
					  1 H1
					  W1 H1
					  W1 0)}
				  {ForAll {Dictionary.items DB} proc{$ E}
								   {DCC tk(itemconf E.frametag width:W height:H)}
								   {E.frame tk(conf width:W height:H)}
								end}
			       end
			    end)}
	 
	 {Tk.addXScrollbar DC SY}
	 {Tk.batch [grid(SY row:0 column:0 sticky:we)
		    grid(DC row:1 column:0 pady:0 ipady:0 sticky:we)
		    grid(DCC row:2 column:0 pady:0 ipady:0 sticky:news)
		    grid(columnconfigure self 0 weight:1)
		    grid(rowconfigure self 2 weight:1)
		   ]}
	 self.AddCard=proc{$ A}	% Export AddCard
			 lock CardLock then
			    if {Dictionary.member DB A.id}==false then
			       Tag={New Tk.canvasTag tkInit(parent:DC)}
			       FTag={New Tk.canvasTag tkInit(parent:DCC)}
			       {DC tk(crea text ~CWidth Y-3 text:A.title tags:Tag anchor:sw)}
			       [TX0 Y0 X1 Y1]={DC tkReturnListInt(bbox Tag $)}
			       X0=TX0-2
			    in
			       %% Create flap
			       {DC tk(crea rect X0-W Y0-H X1+W Y1+H outline:nil tags:q(Tag bgtag))}	 
			       {DC tk(crea line X0-W Y0-H+SC X0-W Y width:2 fill:LCol tags:Tag)}
			       {DC tk(crea line X0-W Y0-H+SC X0-W+SC Y0-H width:2 fill:LCol tags:Tag)}
			       {DC tk(crea line X0-W+SC Y0-H X1+W-SC Y0-H width:2 fill:LCol tags:Tag)}
			       {DC tk(crea line X1+W-SC Y0-H X1+W Y0-H+SC width:2 fill:DCol tags:Tag)}
			       {DC tk(crea line X1+W Y0-H+SC X1+W Y width:2 fill:DCol tags:Tag)}
			       {Tag tkBind(event:'<1>' action:ShowCard#A.id)}
			       {Dictionary.put DB A.id card(id:A.id name:A.title frame:A.frame movetag:Tag
							    frametag:FTag width:2*W+X1-X0 currentx:X0-W)}
			       
			       %% Display frame
			       {DCC tk(create window 0 0 window:A.frame anchor:nw tags:FTag)}
			       {Delay 100}
			       {FTag tk(move ~CWidth ~CWidth)}
			       {Layout}
			    end
			 end
		      end
	 self.RemoveCard=proc{$ A}	% Export RemoveCard
			    lock CardLock then
			       if {Dictionary.member DB A.id} then C={Dictionary.get DB A.id} in
				  {Dictionary.remove DB A.id}
				  {DC tk(delete C.movetag)}
				  {DCC tk(delete C.frametag)}
				  {Layout}
			       end
			    end
			 end
	 {DC tkBind(event:'<Configure>' action:proc{$}
						  lock CardLock then
						     LX={Access LastX}
						     SYV={Access SYVisible}
						     X={Tk.returnInt winfo(width DC)}
						  in
						     if X>LX andthen SYV then
							{Tk.send grid(forget(SY))}
							{Assign SYVisible false}
						     elseif X<LX andthen SYV==false then
							{Tk.send grid(SY row:0 column:0 sticky:we)}
							{Assign SYVisible true}
						     end
						  end
					       end)}
	 self.showCard=ShowCard
      end
      meth showCard(id:ID) {self.showCard ID} end
      meth addCard(id:_ title:_ frame:_)=M {self.AddCard M} end
      meth removeCard(id:_)=M {self.RemoveCard M} end
   end

   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% Extended toplevel
   class Toplevel from Tk.toplevel
      attr Pos:0#0
      meth SaveLocation
	 [X Y]={Map [x y] fun{$ Q} {Tk.returnInt winfo(Q self)} end}
      in
	 Pos<-X#Y
      end
      meth GetLocation($) XY=@Pos in "+"#XY.1#"+"#XY.2 end
      meth tkIconify
	 {self SaveLocation}
	 {Tk.send wm(iconify self)}
      end
      meth tkHide
	 if {Tk.returnAtom wm(state self)}\=withdrawn then
	    {self SaveLocation}
	    {Tk.send wm(withdraw self)}
	 end
      end
      meth tkShow XY={self GetLocation($)} in
	 if {Tk.returnAtom wm(state self)}==withdrawn then
	    {Tk.send wm(deiconify self)}
	 end
	 {Tk.batch [wm(deiconify self) wm(geometry self XY)]}
      end
      meth tkStatus($) {Tk.returnAtom wm(state self)} end
      meth tkSetTitle(Str) {Tk.send wm(title self Str)} end
   end
end

/*

declare
[M]={Module.link ['Widgets.ozf']}

T={New Tk.toplevel tkInit()}
CF={New M.cardFrame tkInit(parent:T)}
{Tk.send pack(CF fill:x pady:5 padx:4)}

F1={New Tk.frame tkInit(parent:CF)} % bg:red)}
F2={New Tk.frame tkInit(parent:CF)} % bg:cyan)}
L1={New Tk.label tkInit(parent:F1 text:"Card1")}
L2={New Tk.label tkInit(parent:F2 justify:center text:"Card\nnumero\n2")}

{Tk.send pack(L1 side:left)}
{Tk.send pack(L2 side:bottom)}


{CF addCard(id:1 title:"Templates" frame:F1)}
{CF addCard(id:2 title:"Session Templates" frame:F2)}

%{CF removeCard(id:2)}

*/
