%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                                                                      %%
%%  FlexClock                                                           %%
%%                                                                      %%
%%  (c) 2000 Université catholique de Louvain.  All Rights Reserved.    %%
%%  The development of this software is supported by the PIRATES        %%
%%  project at the Université catholique de Louvain.  This file is      %%
%%  subject to the general Mozart license.                              %%
%%                                                                      %%
%%  Author: Donatien Grolaux                                            %%
%%                                                                      %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\ifndef APP
declare

[QTk]={Module.link ["QTkBare.ozf"]}

proc{DrawIcon Canvas Tag X Y R} skip end

\endif

local

   %% A QTk look defines default parameters for widgets.
   %% The Look here will set the default background to white

   MB={QTk.newBuilder}
   MW

   Look=MB.defaultLook
   local
      Bg=white
   in
      {Look.set label(bg:Bg)}
      {Look.set 'pure_canvas'(bg:Bg highlightthickness:0)}
      {Look.set placeholder(bg:Bg borderwidth:0)}
      {Look.set td(bg:Bg)}
      {Look.set lr(bg:Bg)}
      {Look.set lrline(bg:Bg)}
   end

   %% NotifyClock defines procedures to call each second with the
   %% current time.
   
   NotifyClock
   local
      Clocks={NewCell nil}
      proc{Loop}
	 T={OS.localTime}
      in
	 %% update in a separate thread so that the {Delay 1000}
	 %% is not influenced by the amount of time taken to update the views
	 thread
	    for I in {Access Clocks} do {I T} end
	 end
	 {Delay 1000}
	 {Loop}
      end
   in
      proc{NotifyClock P}
	 {Assign Clocks P|{Access Clocks}}
      end
      thread
	 {Loop}
      end
   end

   %% Formating functions for displaying the time and date in several
   %% textual formats
   
   fun{TwoPos I}
      if I<10 then "0"#I else I end
   end

   fun{FmtTime T}
      {TwoPos T.hour}#":"#{TwoPos T.min}
   end

   fun{FmtTimeS T}
      {FmtTime T}#":"#{TwoPos T.sec}
   end

   fun{FmtDate T}
      {TwoPos T.mDay}#"/"#{TwoPos T.mon+1}
   end

   fun{FmtDateY T}
      {FmtDate T}#"/"#(1900+T.year)
   end

   fun{FmtDay T}
      {List.nth ["Sunday" "Monday" "Tuesday" "Wednesday" "Thursday" "Friday" "Saturday"] T.wDay+1}
   end

   fun{FmtMonth T}
      {List.nth ["January" "February" "March" "April" "May" "June" "July" "August"
		 "September" "October" "November" "December"] T.mon+1}
   end

   %% The Analog function manages the analog clock.
   %% The clock is drawn on a canvas (simple drawing area), using basic graphics elements :
   %% lines, circles and polygons.
   %% Note that the size of the displayed clock is adapted to the size of the canvas.
   %% Each time the canvas is resized (<configure> event), the new size of the clock is calculated
   %% and the clock is displayed at this new size.

   fun{Analog Redraw}
      C
      Width={NewCell 1}
      Height={NewCell 1}
      Radius={NewCell 1}
      Lock={NewLock}
      CTime={NewCell r(min:~1 hour:~1 sec:0)}
      RTag MTag HTag ITag STag
      fun{XCoord V M R}
	 {Float.toInt
	  {Cos {Int.toFloat V}/{Int.toFloat M}*6.28-1.57}*{Int.toFloat R}}
      end
      fun{YCoord V M R}
	 {Float.toInt
	  {Sin {Int.toFloat V}/{Int.toFloat M}*6.28-1.57}*{Int.toFloat R}}    
      end
   in
      proc{Redraw T}
	 lock Lock then
	    W={Access Width} div 2
	    H={Access Height} div 2
	    R=({Access Radius}*9) div 10
	 in
	    {STag delete}
	    {C create(line W H
		      W+{XCoord T.sec 60 R-2}
		      H+{YCoord T.sec 60 R-2} tags:STag)}
	    if ({Access CTime}.min==T.min andthen {Access CTime}.hour==T.hour) then skip else
	       proc{DrawLine Deg MDeg Radius Tag}
		  {Tag delete}
		  DDeg=((Deg*360) div MDeg)
		  X1#Y1=W#H
		  X2#Y2=W+{XCoord DDeg-3 360 ((Radius*8) div 10)}#H+{YCoord DDeg-3 360 ((Radius*8) div 10)}
		  X3#Y3=W+{XCoord DDeg 360 Radius}#H+{YCoord DDeg 360 Radius}
		  X4#Y4=W+{XCoord DDeg+3 360 ((Radius*8) div 10)}#H+{YCoord DDeg+3 360 ((Radius*8) div 10)}
	       in
		  {C create(polygon X1 Y1 X2 Y2 X3 Y3 X4 Y4 X1 Y1 tags:Tag fill:gray)}
		  {C create(line X1 Y1 X2 Y2 X3 Y3 X4 Y4 X1 Y1 tags:Tag fill:black)}
	       end
	    in
	       {DrawLine T.min 60 R-4 MTag}
	       {DrawLine T.hour*60+T.min 720 (R div 2) HTag}
	    end
	    {Assign CTime T}
	 end
      end
      thread
	 {Wait C}
	 for Tag in [RTag MTag HTag ITag STag] do {C newTag(Tag)} end
	 {C bind(event:'<Configure>'
		 action:proc{$}
			   lock Lock then
			      CW={C winfo(width:$)}
			      CH={C winfo(height:$)}
			      W=CW div 2
			      H=CH div 2
			      {Assign Width CW}
			      {Assign Height CH}
			      {Assign Radius {Min (CW div 2) (CH div 2)}-4}
			      R=(({Access Radius}*9) div 10)+2
			      Old={Access CTime}
			   in
			      {RTag delete}
			      {C create(oval W-R H-R W+R H+R tags:RTag)}
			      {List.forAllInd {List.make 12}
			       proc{$ I _}
				  {C create(line
					    W+{XCoord I 12 R-3}
					    H+{YCoord I 12 R-3}
					    W+{XCoord I 12 R+3}
					    H+{YCoord I 12 R+3}
					    width:if (I mod 3)==0 then 3 else 1 end
					    tags:RTag)}
			       end}
			      {C create(oval W-2 H-2 W+2 H+2 fill:black tags:RTag)}
			      {Assign CTime r(min:~1 hour:~1 sec:0)}
			      {Redraw Old}
			      {DrawIcon C ITag W H R}
			   end
			end)}
      end
      canvas(handle:C bg:white glue:nswe)
   end
	  
   fun{Calendar Redraw}
      Old={NewCell r(yDay:0 year:0 mon:0)}
      Header=[label newline
	      label(text:"Mo") label(text:"Tu") label(text:"We") label(text:"Th")
	      label(text:"Fr") label(text:"Sa") label(text:"Su") newline
	      lrline(glue:we) continue continue continue continue continue continue newline]
      P
   in
      proc{Redraw T}
	 O={Access Old}
      in
	 if T.yDay==O.yDay andthen T.year==O.year andthen T.mon==O.mon then skip
	 else
	    {Assign Old T}
	    ML={List.nth [31
			  if (T.year div 4)==0 then 29 else 28 end
			  31 30 31 30 31 31 30 31 30 31] T.mon+1}
	    SD=(((7-(T.mDay mod 7))+T.wDay) mod 7) % starting day of week of the month
	    fun{Loop Skip Col Nu}
	       if Nu>ML then nil
	       elseif Col==8 then
		  newline|{Loop Skip 1 Nu}
	       elseif Skip>0 then
		  label|{Loop Skip-1 Col+1 Nu}
	       elseif Nu==T.mDay then
		  label(text:Nu bg:black fg:white)|{Loop 0 Col+1 Nu+1}
	       else
		  label(text:Nu)|{Loop 0 Col+1 Nu+1}
	       end
	    end
	    R={List.append Header {Loop SD 1 1}}
	 in
	    {P set({List.toTuple lr R})}
	 end
      end
      placeholder(handle:P)
   end

   H1 H2 H3 H4 H5 H6 H7 H8 H9 H10 H11 H12 H13 H14 H15 H16
   L1 L2 L3 L4 L5 L6 L7 L8 L9
   A1 A2 A3 A4 A5 A6 A7 A8 A9 A10
   C1 C2
   
   ViewList=[r(refresh:proc{$ T} {H1 set(text:{FmtTime T})} end
	       desc:label(handle:H1 glue:nswe bg:white)
	       area:40#10)
	     r(refresh:proc{$ T} {H2 set(text:{FmtTimeS T})} end
	       desc:label(handle:H2 glue:nswe bg:white)
	       area:80#10)
	     r(refresh:proc{$ T} {H3 set(text:{FmtTime T}#'\n'#{FmtDate T})} end
	       desc:label(handle:H3 glue:nswe bg:white)
	       area:40#30)
	     r(refresh:proc{$ T} {H4 set(text:{FmtTimeS T}#'\n'#{FmtDateY T})} end
	       desc:label(handle:H4 glue:nswe bg:white)
	       area:80#30)
	     r(refresh:proc{$ T} {H5 set(text:{FmtTimeS T}#'\n'#{FmtDay T}#", "#{FmtDateY T})} end
	       desc:label(handle:H5 glue:nswe bg:white)
	       area:130#30)
	     r(refresh:proc{$ T} {H6 set(text:{FmtTimeS T}#'\n'#{FmtDay T}#", "#
					 T.mDay#" "#{FmtMonth T}#" "#(1900+T.year))} end
	       desc:label(handle:H6 glue:nswe bg:white)
	       area:180#30)
	     r(refresh:proc{$ T} {A1 T} end
	       desc:td(handle:H7 glue:nswe {Analog A1} look:Look)
	       area:60#60)
	     r(refresh:proc{$ T} {L1 set(text:{FmtTime T}#"\n"#{FmtDate T})} {A2 T} end
	       desc:lr(handle:H8 glue:nswe {Analog A2} label(handle:L1) look:Look)
	       area:100#60)
	     r(refresh:proc{$ T} {L2 set(text:{FmtTimeS T}#"\n"#{FmtDateY T})} {A3 T} end
	       desc:lr(handle:H9 glue:nswe {Analog A3} label(handle:L2) look:Look)
	       area:120#60)
	     r(refresh:proc{$ T} {L3 set(text:{FmtTimeS T}#'\n'#{FmtDay T}#", "#{FmtDateY T})} {A4 T} end
	       desc:lr(handle:H10 glue:nswe {Analog A4} label(handle:L3) look:Look)
	       area:180#60)
	     r(refresh:proc{$ T} {L4 set(text:{FmtTimeS T}#'\n'#{FmtDay T}#", "#
					 T.mDay#" "#{FmtMonth T}#" "#(1900+T.year))} {A5 T} end
	       desc:lr(handle:H11 glue:nswe {Analog A5} label(handle:L4) look:Look)
	       area:250#60)
	     r(refresh:proc{$ T} {L5 set(text:{FmtTimeS T}#'\n'#{FmtDateY T})} {A6 T} end
	       desc:td(handle:H12 glue:nswe {Analog A6} label(handle:L5) look:Look)
	       area:100#100)
	     r(refresh:proc{$ T} {L6 set(text:{FmtTimeS T}#'\n'#{FmtDay T}#", "#{FmtDateY T})} {A7 T} end
	       desc:td(handle:H13 glue:nswe {Analog A7} label(handle:L6) look:Look)
	       area:130#100)
	     r(refresh:proc{$ T} {L7 set(text:{FmtTimeS T}#'\n'#{FmtDay T}#", "#
					 T.mDay#" "#{FmtMonth T}#" "#(1900+T.year))} {A8 T} end
	       desc:td(handle:H14 glue:nswe {Analog A8} label(handle:L7) look:Look)
	       area:180#100)
	     r(refresh:proc{$ T} {L8 set(text:{FmtTimeS T}#"\n"#
					 {FmtDay T}#", "#T.mDay#" "#{FmtMonth T}#" "#(1900+T.year))}
			  {A9 T} {C1 T}
		       end
	       desc:lr(handle:H15 glue:nswe {Analog A9} td(label(handle:L8) {Calendar C1}) look:Look)
	       area:280#160)
	     r(refresh:proc{$ T} {L9 set(text:{FmtTimeS T}#"\n"#
					 {FmtDay T}#", "#T.mDay#" "#{FmtMonth T}#" "#(1900+T.year))}
			  {A10 T} {C2 T}
		       end
	       desc:td(handle:H16 glue:nswe {Analog A10} label(handle:L9) {Calendar C2} look:Look)
	       area:180#230)]
   
   P

   
   
   MW={MB.buildMigratable td(placeholder(handle:P width:1 height:1 glue:nswe))}

   R1
   Win1={QTk.build td(title:"FlexClock Window 1" receiver(glue:nswe handle:R1))}
   R2
   Win2={QTk.build td(title:"FlexClock Window 2" receiver(glue:nswe handle:R2))}
   {Show 1}
   {R1 set({MW get(ref:$)})}
   
   {Show 2}
   Views={List.map ViewList
	  fun{$ R}
	     Width#Height=R.area
	  in
	     {Show placing#R.desc}
	     {Delay 100}
	     {P set(R.desc)}
	     {NotifyClock R.refresh}
	     Width#Height#(R.desc).handle
	  end}
   {Show 3}
   proc{Place}
      WW={P winfo(width:$)}
      WH={P winfo(height:$)}
      fun{Select Views Max#CH}
	 case Views
	 of W#H#Handle|R then
	    This=(W-WW)*(W-WW)+(H-WH)*(H-WH)
	 in
	    if W<WW andthen H<WH andthen
	       (Max==inf orelse This<Max) then
	       {Select R This#Handle}
	    else
	       {Select R Max#CH}
	    end
	 else CH end
      end
   in
      {P set({Select Views inf#Views.1.3})}
   end
   {Show 4}
   {P bind(event:'<Configure>'
	   action:Place)}
   {Show 5}
   {Win1 show}
   {Win2 show}
   {Show 6}
   proc{Run}
      {Delay 30000}
      {R2 set({MW get(ref:$)})}
      {Delay 30000}
      {R1 set({MW get(ref:$)})}
      {Run}
   end
in
   {Run}
   {Show 7}
   
end
   
