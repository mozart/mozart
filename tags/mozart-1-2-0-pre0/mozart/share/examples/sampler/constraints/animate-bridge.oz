%%%
%%% Authors:
%%%   Jörg Würtz <wuertz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Jörg Würtz, 1998
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

declare Bridge InitialDelay=2000 RepDelay=180

in
local
   %% Sizes
   CanvasWidth = 150
   CanvasHeight = 100
   Zoom = 6
   OffY = Zoom*30
   OffX = 0
   LineWidth = 2
   Outside = 200*Zoom
   ScheduleLength = 104
   %% Colors
   [GroundColor BColor MColor TColor VColor FColor PColor] =
   [burlywood grey55 grey72 steelblue1 khaki1 khaki1 black]
   OutlineColor = black
   CanvasColor = lightskyblue1
   %% Data for tasks
   Start = start( a1: 19 a2: 6 a3: 0 a4: 4 a5: 2 a6: 26
		  b1: 34 b2: 14 b3: 26 b4: 48 b5: 10 b6: 44
		  m1: 36 m2: 20 m3: 28 m4: 52 m5: 12 m6: 60
		  p1: 2 p2: 28
		  s1: 26 s2: 10 s3: 22 s4: 44 s5: 6 s6: 34
		  t1: 52 t2: 36 t3: 92 t4: 64 t5: 80
		  v1: 64 v2: 92 )
   Duration = dur(a1: 4 a2: 2 a3: 2 a4: 2  a5: 2  a6: 5  
		  p1: 20 p2: 13 
		  s1: 8  s2: 4  s3: 4  	s4: 4  	s5: 4  	s6: 10 
		  b1: 1  b2: 1  b3: 1  	b4: 1  	b5: 1  	b6: 1  
		  m1: 16 m2: 8 	m3: 8 	m4: 8 	m5: 8 	m6: 20
		  t1: 12 t2: 12	t3: 12	t4: 12	t5: 12
		  v1: 15 v2: 10)
   TaskNames = {Record.arity Start}
   Starting = {MakeTuple s ScheduleLength+1}
   Running = {MakeTuple r ScheduleLength+1}
   Ending = {MakeTuple e ScheduleLength+1}

   Coo = fun{$ Ls}
	    {List.toTuple o {FoldR Ls fun{$ X#Y In}
					 Zoom*X+OffX|Zoom*Y+OffY|In
				      end nil}}
	 end

   %% Graphics data
   Outline = [0#0 16#28  31#28  37#46  49#46  50#37  57#37  58#46
	      58#46  70#46  71#37  78#37  79#46  91#46  92#37  99#37
	      100#46  112#46 118#28 132#28 150#0 150#80 0#80 0#0]
   LeftOutline = [0#0 16#28 31#28 31#0 0#0]
   RightOutline = [118#28 132#28 150#0 118#0 118#28]
   In2 = [34#37 50#37 49#46 37#46 34#37]
   In3 = [57#37 71#37 70#46 58#46 57#37]
   In4 = [78#37 92#37 91#46 79#46 78#37]
   In5 = [99#37 115#37 112#46 100#46 99#37]

   S1 = [16#19 16#28 29#28 29#19] S1T = [21#26]
   S2 = [39#37 39#46 47#46 47#37] S2T = [41#44]
   S3 = [60#37 60#46 68#46 68#37] S3T = [62#44]
   S4 = [81#37 81#46 89#46 89#37] S4T = [83#44]
   S5 = [102#37 102#46 110#46 110#37] S5T = [104#44]
   S6 = [119#19 119#28 132#28 132#19] S6T = [124#26]
   B1 = [16#19 29#28] B1T = [21#23]
   B2 = [39#37 47#46] B2T = [41#41]
   B3 = [60#37 68#46] B3T = [62#41]
   B4 = [81#37 89#46] B4T = [83#41]
   B5 = [102#37 110#46] B5T = [104#41]
   B6 = [119#19 132#28] B6T = [124#23]
   M1 = [17#19 28#19 28#6 22#6] M1T = [22#14]
   M2 = [40#37 46#6] M2T = [41#23]
   M3 = [61#37 67#6] M3T = [62#23]
   M4 = [82#37 88#6] M4T = [83#23]
   M5 = [103#37 109#6] M5T = [104#23]
   M6 = [120#19 131#19 126#6 120#6] M6T = [123#14]
   T1 = [22#6 43#0] T1T = [31#4]
   T2 = [43#6 64#0] T2T = [52#4]
   T3 = [64#6 86#0] T3T = [73#4]
   T4 = [86#6 106#0] T4T = [95#4]
   T5 = [106#6 126#0] T5T = [115#4]
   V1 = [0#0 16#28 16#19 17#19 22#6 22#0 0#0] V1T = [12#10]
   V2 = [150#0 132#28 132#19 131#19 126#6 126#0 150#0] V2T = [136#10]
   F21 = [39#37 39#46 37#46 34#37 39#37]
   F22 = [47#37 47#46 49#46 50#37 47#37]
   F31 = [60#46 60#37 57#37 58#46 60#46]
   F32 = [68#46 70#46 71#37 68#37 68#46]
   F41 = [81#46 81#37 78#37 79#46 81#46]
   F42 = [89#46 91#46 92#37 89#37 89#37 89#46]
   F51 = [102#46 102#37 99#37 100#46 102#46]
   F52 = [110#46 112#46 115#37 110#37 110#46]
   A1 = LeftOutline A1L = [0#0 16#28  31#28] A1T = [12#30]
   A2 = In2 A2L = [31#28 37#46  49#46  50#37] A2T = [33#48]
   A3 = In3 A3L = [57#37  58#46  70#46  71#37] A3T = [54#48]
   A4 = In4 A4L = [78#37  79#46  91#46  92#37] A4T = [75#48]
   A5 = In5 A5L = [99#37 100#46  112#46 118#28] A5T = [96#48]
   A6 = RightOutline A6L = [118#28 132#28 150#0] A6T = [134#30]
   P1T = [67#59]
   P1 = [60#46 60#55 61#55 61#46 62#46 62#57 63#57 63#46 65#46 65#57 66#57
	66#46 67#46 67#55 68#55 68#46 60#46]
   P2T = [88#59]
   P2 = [81#46 81#55 82#55 82#46 83#46 83#57 84#57 84#46 86#46 86#57 87#57
	 87#46 88#46 88#55 89#55 89#46 81#46]
   
   proc {DoOne C Tag Action}
      Task = Action.1 in
      case {Label Action}
      of mkLine then
	 {C tk(crea line {Coo Task} width: LineWidth tag: Tag)}
      [] mkText then
	 {C tk(crea text {Coo Task} text: Action.2
	       tag: Tag
	       anchor: sw)}
      [] mkRec then
	 {C tk(crea rectangle {Coo Task}
	       fill: Action.2
	       tag: Tag
	       width: LineWidth)}
      [] mkFPoly then
	 {C tk(crea polygon {Coo Task} fill: Action.2
	       tag: Tag
	       outline: OutlineColor
	       width: LineWidth)}
      [] mkPoly then
	 {C tk(crea polygon {Coo Task} fill: Action.2
	       tag: Tag)}
      else fail
      end
   end

   class Tag from Tk.canvasTag end
   
   class TaskC from BaseObject 
      attr initAction
	 continuation
	 tag
      meth init(A C)
	 initAction <- A
	 continuation <- C
      end
      meth start(C)
	 tag <- {New Tag tkInit(parent:C)}
	 {DoOne C @tag @initAction}
      end
      meth moveIn
	 {@tag tk(move ~Outside ~Outside)}
      end
      meth moveOut
	 {@tag tk(move Outside Outside)}
      end
      meth continuation(C)
	 Tag = @tag in
	 {ForAll @continuation proc{$ Action}
				  {DoOne C Tag Action}
			       end}
      end
   end
   
   fun {MkObject Action Cont}
      {New TaskC init(Action Cont)}
   end
   
   Tasks = tasks(s1: {MkObject mkLine(S1) [mkText(S1T 'S1')]}
		 s2: {MkObject mkLine(S2) [mkText(S2T 'S2')]}
		 s3: {MkObject mkLine(S3) [mkText(S3T 'S3')]}
		 s4: {MkObject mkLine(S4) [mkText(S4T 'S4')]}
		 s5: {MkObject mkLine(S5) [mkText(S5T 'S5')]}
		 s6: {MkObject mkLine(S6) [mkText(S6T 'S6')]}
		 b1: {MkObject mkRec(B1 BColor) [mkText(S1T 'S1')
						 mkText(B1T 'B1')]}
		 b2: {MkObject mkRec(B2 BColor) [mkText(S2T 'S2')
						 mkText(B2T 'B2')
						 mkFPoly(F21 FColor)
						 mkFPoly(F22 FColor)]}
		 b3: {MkObject mkRec(B3 BColor) [mkText(S3T 'S3')
						 mkText(B3T 'B3')
						 mkFPoly(F31 FColor)
						 mkFPoly(F32 FColor)]}
		 b4: {MkObject mkRec(B4 BColor) [mkText(B4T 'B4')
						 mkText(S4T 'S4')
						 mkFPoly(F41 FColor)
						 mkFPoly(F42 FColor)]}
		 b5: {MkObject mkRec(B5 BColor) [mkText(S5T 'S5')
						 mkText(B5T 'B5')
						 mkFPoly(F51 FColor)
						 mkFPoly(F52 FColor)]}
		 b6: {MkObject mkRec(B6 BColor) [mkText(S6T 'S6')
						 mkText(B6T 'B6')]}
		 m1: {MkObject mkFPoly(M1 MColor) [mkText(M1T 'M1')]}
		 m2: {MkObject mkRec(M2 MColor) [mkText(M2T 'M2')]}
		 m3: {MkObject mkRec(M3 MColor) [mkText(M3T 'M3')]}
		 m4: {MkObject mkRec(M4 MColor) [mkText(M4T 'M4')]}
		 m5: {MkObject mkRec(M5 MColor) [mkText(M5T 'M5')]}
		 m6: {MkObject mkFPoly(M6 MColor) [mkText(M6T 'M6')]}
		 t1: {MkObject mkRec(T1 TColor) [mkText(T1T 'T1')]}
		 t2: {MkObject mkRec(T2 TColor) [mkText(T2T 'T2')]}
		 t3: {MkObject mkRec(T3 TColor) [mkText(T3T 'T3')]}
		 t4: {MkObject mkRec(T4 TColor) [mkText(T4T 'T4')]}
		 t5: {MkObject mkRec(T5 TColor) [mkText(T5T 'T5')]}
		 v1: {MkObject mkFPoly(V1 VColor) [mkText(V1T 'V1')]}
		 v2: {MkObject mkFPoly(V2 VColor) [mkText(V2T 'V2')]}
		 a1: {MkObject mkPoly(A1 CanvasColor) [mkLine(A1L)
						       mkText(A1T 'A1')]}
		 a2: {MkObject mkPoly(A2 CanvasColor) [mkLine(A2L)
						       mkText(A2T 'A2')]}
		 a3: {MkObject mkPoly(A3 CanvasColor) [mkLine(A3L)
						       mkText(A3T 'A3')]}
		 a4: {MkObject mkPoly(A4 CanvasColor) [mkLine(A4L)
						       mkText(A4T 'A4')]}
		 a5: {MkObject mkPoly(A5 CanvasColor) [mkLine(A5L)
						       mkText(A5T 'A5')]}
		 a6: {MkObject mkPoly(A6 CanvasColor) [mkLine(A6L)
						       mkText(A6T 'A6')]}
		 p1: {MkObject mkPoly(P1 PColor) [mkText(P1T 'P1')]}
		 p2: {MkObject mkPoly(P2 PColor) [mkText(P2T 'P2')]}
		)
   
   fun {RunningP T C} Start.T<C andthen C<Start.T+Duration.T end
   fun {StartingP T C} Start.T==C end
   fun {EndingP T C} Start.T+Duration.T==C end
   fun {Extract P C}
      {Filter TaskNames fun {$ T} {P T C} end}
   end
   proc {Anim Ts Message}
      {ForAll Ts proc {$ T} {Tasks.T Message} end}
   end
in
   %% Initialize data structures
   {Loop.'for' 0 ScheduleLength 1
    proc{$ C}
       Running.(C+1) = {Extract RunningP C}
       Starting.(C+1) = {Extract StartingP C}
       Ending.(C+1) = {Extract EndingP C}
    end}
   
   class Bridge from Time.repeat 
      attr canvas
	 toggle
	 time
      meth check
	 Canvas = @canvas
	 IncTime = @time+1
      in 
	 if @toggle==0 then
	    {Anim Starting.IncTime start(Canvas)}
	    {Anim Running.IncTime moveIn}
	    {Anim Ending.IncTime moveIn}
	    {Anim Ending.IncTime continuation(Canvas)}
	 else
	    {Anim {Append Running.IncTime Starting.IncTime} moveOut}
	    time  <- IncTime
	 end
	 toggle <- (@toggle + 1) mod 2
	 if @time>ScheduleLength then {self stop} end
      end
      meth init
	 W = {New Tk.toplevel tkInit }
	 {Tk.send wm(title W 'Bridge Construction')}
	 Canvas = {New Tk.canvas tkInit(parent:W
					background: CanvasColor
					highlightthickness:0
					width:CanvasWidth*Zoom
					height:CanvasHeight*Zoom)}
      in
	 canvas <- Canvas
	 {Tk.send pack(Canvas)}
	 {ForAll [Outline LeftOutline RightOutline In2 In3 In4 In5]
	  proc{$ P}
	     {Canvas tk(crea(polygon {Coo P}  o(fill:GroundColor)))}
	  end}
	 toggle <- 0
	 time <- 0
      end
   end
end

thread
   B={New Bridge init}
in
   {B setRepDelay(RepDelay)}
   {B setRepAction(check)}
   {Delay InitialDelay}
   {B go}
end
