declare

[QTk]={Module.link ["QTk.ozf"]}

T={New Tk.toplevel tkInit}
C={New Tk.canvas tkInit(parent:T)}
{Tk.send pack(C)}
Tag={New Tk.canvasTag tkInit(parent:C)}

{Show {Tk.getTclName Tag}}

%Forget={Tk.defineUserCmd "ShowHello" proc{$ X} {Show 'hello '#X} end [int(1)]}
F={New Tk.font tkInit(family:"Courier" size:"10")}

TclId
{C tkReturnInt(create(rectangle 10 10 100 100 fill:red) TclId)}
{C tk(create line 5 5 10 10)}
{C tk(create text 100 100 text:"{Hello World}" font:F)}
{C tk(create text 50 150 text:"Test" font:"System")}
{C tk(create poly 20 30 40 64 78 65 98 34 smooth:true fill:blue width:2)}
{Show TclId}


%{F tk(configure size:15)}
%{Browse {F tkReturnList(configure $)}}

T1={New Tk.canvasTag tkInit(parent:C)}
T2={New Tk.canvasTag tkInit(parent:C)}

{C tk(create arc 200 200 300 300 extent:90 start:30 style:chord width:3 dash:25 fill:green tags:q(T1 T2))}

%{C tk(bind TclId "<1>" "ShowHello")}


fun{Dump C}
   FontNames={Tk.returnListAtom font(names)}
   fun{FontToString F}
      if {List.member {VirtualString.toAtom F} FontNames} then
	 I={Tk.returnList font(configure F)}
	 fun{Loop X}
	    case X of Opt|Val|Ls then
	       {String.toAtom Opt.2}#Val|{Loop Ls}
	    else nil end
	 end
	 R={Loop I}
	 {Browse R}
      in
	 {VirtualString.toString
	  {List.foldL R.2
	   fun{$ Old I#V}
	      if I==underline orelse I==overstrike then
		 if V=="0" then Old else Old#" "#I end
	      else
		 Old#" "#V
	      end
	   end
	   R.1.2}}
      else
	 F
      end
   end
   Init={C tkReturnListInt(find(closest 0 0) $)}
in
   if Init==nil then nil % canvas is empty
   else
      fun{Loop T}
	 N={C tkReturnListInt(find(below T) $)}
      in
	 if N==nil then
	    T
	 else
	    {Loop N.1}
	 end
      end
      fun{Parse T}
	 fun{GetParams}
	    fun{Grab L G}
	       L1={List.dropWhile L fun{$ C} C==&  end}
	    in
	       if L1.1==&{ then
		  fun{Loop L Width R}
		     case L
		     of &{|Ls then
			&{|{Loop Ls Width+1 R}
		     [] &}|Ls then
			if Width==0 then
			   R=Ls.2
			   nil
			else
			   &}|{Loop Ls Width-1 R}
			end
		     [] L|Ls then
			L|{Loop Ls Width R}
		     end
		  end
	       in
		  {Loop L1.2 0 $ G}
	       else
		  {List.takeDropWhile L1 fun{$ C} C\=&} andthen C\=& end G}
	       end
	    end
	    fun{Loop L}
	       case L
	       of &{|&-|Ls then
		  Cmd R Remain P
	       in
		  {List.takeDropWhile Ls fun{$ C} C\=& end Cmd R}
		  Remain={Grab {Grab {Grab {Grab R _} _} _} P}
		  {String.toAtom Cmd}#P|{Loop Remain}
	       [] L|Ls then
		  {Loop Ls}
	       else nil end
	    end
	    fun{MapAndFilter L}
	       case L
	       of tags#V|Ls then {Show {String.toAtom V}} {MapAndFilter Ls}
	       [] font#F|Ls then
		  {Show font#{String.toAtom F}}
		  {Show ffont#{String.toAtom {FontToString F}}}
		  font#{FontToString F}|{MapAndFilter Ls}
	       [] smooth#V|Ls then smooth#if V=="bezier" then "1" else "0" end|{MapAndFilter Ls}
	       [] _#nil|Ls then {MapAndFilter Ls}
	       [] L|Ls then L|{MapAndFilter Ls}
	       else nil end
	    end
	    R={C tkReturn(itemconfigure(T) $)}
	 in
	    {MapAndFilter {Loop R}}
	 end
% 	    RetRec={NewCell R}
% 	 in
% 	    {ForAll L
% 	     proc{$ P}
% 		MyP={List.last {Source tkReturnList(itemconfigure(Tag "-"#P) $)}}
% %		MyP={Source tkReturn(itemconfigure(Tag {StringToAtom {VirtualString.toString "-"#P}}) $)}
% 		{Wait MyP}
% 	     in
% 		if MyP=="{}" then skip else
% 		   {Assign RetRec {AdjoinAt {Access RetRec} P MyP}}
% 		end
% 	     end}
% 	    {Access RetRec}
% 	 end
	 Type={C tkReturnAtom(type(T) $)}
	 RetI=create(Type
		     b({C tkReturnListFloat(coords(T) $)}))
	 RetP={GetParams}
% 	 {GetParams
% 	       case Type
% 	       of arc        then [dash activedash disableddash dashoffset fill activefill disabledfill offset outline activeoutline disabledoutline outlinestipple activeoutlinestipple disabledoutlinestipple stipple activestipple disabledstipple state width activewidth disabledwidth extent start style]
% 	       [] bitmap     then [state anchor background activebackground disabledbackground bitmap activebitmap disabledbitmap foreground activeforeground disabledforeground]
% 	       [] image      then [state anchor image activeimage disabledimage]
% 	       [] line       then [dash activedash disableddash dashoffset fill activefill disabledfill stipple activestipple disabledstipple state width activewidth disabledwidth arrow arrowshape capstyle joinstyle smooth splinesteps]
% 	       [] oval       then [dash activedash disableddash dashoffset fill activefill disabledfill offset outline activeoutline disabledoutline outlinestipple activeoutlinestipple disabledoutlinestipple stipple activestipple disabledstipple state width activewidth disabledwidth]
% 	       [] polygon    then [dash activedash disableddash dashoffset fill activefill disabledfill offset outline activeoutline disabledoutline outlinestipple activeoutlinestipple disabledoutlinestipple stipple activestipple disabledstipple state width activewidth disabledwidth joinstyle smooth splinesteps]
% 	       [] rectangle  then [dash activedash disableddash dashoffset fill activefill disabledfill offset outline activeoutline disabledoutline outlinestipple activeoutlinestipple disabledoutlinestipple stipple activestipple disabledstipple state width activewidth disabledwidth]
% 	       [] text       then [fill activefill disabledfill stipple activestipple disabledstipple state anchor font justify text width]
% 	       [] window     then [state anchor height width window]
% 	       end}
	 Ret={Record.adjoin {List.toRecord r RetP} RetI}
	 N={C tkReturnListInt(find(above T) $)}
      in
	 if N==nil then
	    Ret|nil
	 else
	    Ret|{Parse N.1}
	 end
      end
   in
      {Parse {Loop Init.1}}
   end
end

Du={Dump C}
%{Browse Du}
NC
{{QTk.build td(title:"Copy" canvas(handle:NC))} show}

{ForAll Du proc{$ M} {NC M} end}


{F tk(configure size:20)}

%{Browse {Tk.returnList font(configure "{Courier 10}")}}

