%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1998
%%%   Simon Lindblom, 1998
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
%   Browser(browse:Browse)
export
   start:Start
define
   Cs=[aliceblue antiquewhite aquamarine azure beige bisque black
       blanchedalmond blue blueviolet brown burlywood cadetblue chartreuse
       chocolate coral cornflowerblue cornsilk cyan darkblue darkcyan
       darkgoldenrod darkgray darkgreen darkgrey darkkhaki darkmagenta
       darkolivegreen darkorange darkorchid darkred darksalmon darkseagreen
       darkslateblue darkslategray darkslategrey darkturquoise darkviolet
       deeppink deepskyblue dimgray dimgrey dodgerblue firebrick floralwhite
       forestgreen gainsboro ghostwhite gold goldenrod gray green
       greenyellow grey honeydew hotpink indianred ivory khaki lavender
       lavenderblush lawngreen lemonchiffon lightblue lightcoral lightcyan
       lightgoldenrod lightgoldenrodyellow lightgray lightgreen lightgrey
       lightpink lightsalmon lightseagreen lightskyblue lightslateblue
       lightslategray lightslategrey lightsteelblue lightyellow limegreen
       linen magenta maroon mediumaquamarine mediumblue mediumorchid
       mediumpurple mediumseagreen mediumslateblue mediumspringgreen
       mediumturquoise mediumvioletred midnightblue mintcream mistyrose
       moccasin navajowhite navy navyblue oldlace olivedrab orange orangered
       orchid palegoldenrod palegreen paleturquoise palevioletred papayawhip
       peachpuff peru pink plum powderblue purple red rosybrown royalblue
       saddlebrown salmon sandybrown seagreen seashell sienna skyblue
       slateblue slategray slategrey snow springgreen steelblue tan thistle
       tomato turquoise violet violetred wheat white whitesmoke yellow
       yellowgreen]

   proc{Start UICell Change}
      FgC BgC FoS
      T={New Tk.toplevel tkInit(title:"Configure Client" delete:proc{$}
								   {T tkClose}
								   Change=false
								end)}
      
      ColorF={New Tk.font tkInit(size:{Access UICell}.fontsize family:helvetica)}

      FF={New Tk.frame tkInit(parent:T bd:2 relief:sunken)}
      FontB1={New Tk.button tkInit(parent:FF text:"small (8pt)" action:proc{$}
									  {Assign FoS 8}
									  {ColorF tk(conf size:8)}
								     end)}
      FontB2={New Tk.button tkInit(parent:FF text:"medium (10pt)" action:proc{$}
									    {Assign FoS 10}
									    {ColorF tk(conf size:10)}
								       end)}
      FontB3={New Tk.button tkInit(parent:FF text:"large (12pt)" action:proc{$}
									   {Assign FoS 12}
									   {ColorF tk(conf size:12)}
								      end)}
      FontL={New Tk.label tkInit(parent:FF text:"Font Size:")}
      ColorL={New Tk.label tkInit(parent:T bd:1 font:ColorF relief:groove
				  text:"This will be your font and color selection!")}
      CF={New Tk.frame tkInit(parent:T bd:2 relief:sunken)}
      FgL={New Tk.label tkInit(parent:CF text:"Foreground: "#{Access UICell}.foreground)}
      BgL={New Tk.label tkInit(parent:CF text:"Background: "#{Access UICell}.background)}
      FgLB={New Tk.listbox tkInit(parent:CF
				  setgrid:true
				  width:20
				  height:15
				  selectmode:single)}
      SY1 = {New Tk.scrollbar tkInit(parent:CF width:9)}
      {Tk.addYScrollbar FgLB SY1}
      BgLB={New Tk.listbox tkInit(parent:CF
				  setgrid:true
				  width:20
				  height:15
				  selectmode:single)}
      SY2 = {New Tk.scrollbar tkInit(parent:CF width:9)}
      {Tk.addYScrollbar BgLB SY2}
      DoneB={New Tk.button tkInit(parent:T text:"Use This New Setting!"
				  action:proc{$}
					    {Assign UICell ui(fontsize:{Access FoS}
							      foreground:{Access FgC}
							      background:{Access BgC}
							      browser:{BrowserV tkReturnInt($)}==1)}
					    {T tkClose}
					    Change=true
					 end)}
      CancelB={New Tk.button tkInit(parent:T text:"Cancel!"
				    action:proc{$}
					      {T tkClose}
					      Change=false
					   end)}
      BrowserF={New Tk.frame tkInit(parent:T)}
      BrowserV={New Tk.variable tkInit({CondSelect {Access UICell} browser false})}
      BrowserL={New Tk.label tkInit(parent:BrowserF text:"Using web-browser")}
      BrowserB={New Tk.checkbutton tkInit(parent:BrowserF variable:BrowserV)}
   in
      {Tk.batch [grid(FontL row:0 column:0 sticky:e)
		 grid(FontB1 row:0 column:1 sticky:we pady:3 padx:1)
		 grid(FontB2 row:0 column:2 sticky:we padx:1)
		 grid(FontB3 row:0 column:3 sticky:we padx:1)
		 grid(FF row:0 column:0 columnspan:2 sticky:news ipadx:2 pady:3)
		 grid(ColorL row:5 column:0 columnspan:2 sticky:we padx:1 ipady:4 ipadx:4)
		 grid(FgL row:0 column:0 sticky:nwe)
		 grid(BgL row:0 column:2 sticky:nwe)
		 grid(FgLB row:3 column:0 sticky:news)
		 grid(BgLB row:3 column:2 sticky:news)
		 grid(SY1 row:3 column:1 sticky:ns)
		 grid(SY2 row:3 column:3 sticky:ns)
		 grid(CF row:7 column:0 columnspan:2 sticky:news pady:3 ipadx:2)
		 grid(BrowserL row:0 column:1 sticky:w)
		 grid(BrowserB row:0 column:0 sticky:e)
		 grid(BrowserF row:8 column:0 columnspan:2 sticky:news pady:3 ipadx:2)
		 grid(DoneB row:10 column:0 sticky:we)
		 grid(CancelB row:10 column:1 sticky:we)
		]}
      FgC={NewCell {ColorL tkReturn(cget '-foreground' $)}}
      BgC={NewCell {ColorL tkReturn(cget '-background' $)}}
      FoS={NewCell {Access UICell}.fontsize}
      {ForAll Cs proc{$ X}
		    {FgLB tk(insert 'end' X)}
		    {BgLB tk(insert 'end' X)}
		 end}

      {FgLB tkBind(event:'<1>'
		   action: proc {$} I={FgLB tkReturnInt(curselection $)} in
			      if I\=false andthen I\=nil then
				 C={Nth Cs I+1}
			      in
				 {ColorL tk(config fg:C)}
				 {Assign FgC C}
			      end
			   end)}
      {BgLB tkBind(event:'<1>'
		   action: proc {$} I={BgLB tkReturnInt(curselection $)} in
			      if I\=false andthen I\=nil then
				 C={Nth Cs I+1}
			      in
				 {ColorL tk(config bg:C)}
				 {Assign BgC C}
			      end
			   end)}
      local
	 C={Access UICell}
      in
	 {List.forAllInd Cs proc{$ I X}
			       if X==C.foreground then
				  {FgLB tk(see I-1)}
			       end
			       if X==C.background then
				  {BgLB tk(see I-1)}
				  {BgLB tk(selection set I-1 I-1)}
			       end
			    end}
      end
      
      
   end
end	    

