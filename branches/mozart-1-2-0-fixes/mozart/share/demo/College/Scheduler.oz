%%%
%%% Authors:
%%%   Tobias Mueller <tmueller@ps.uni-sb.de>
%%%   Joerg Wuertz <wuertz@de.ibm.com>
%%%
%%% Copyright:
%%%   Tobias Mueller, 1998
%%%   Joerg Wuert, 1997
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

   Open

   FD

   OS

   Search

   Compiler

   Inspector(inspect: Inspect)

   Graphics(drawSchedule: DrawSchedule)

   Controller(topWindow: TopWindow)
   Latex(laTeX: LaTeX)
   Main(controllerLabel: ControllerLabel)

   Common(editor:          Editor
	  monday:          Monday
	  tuesday:         Tuesday
	  wednesday:       Wednesday
	  thursday:        Thursday
	  friday:          Friday
	  quartersPerDay:  QuartersPerDay
	  quartersPerHour: QuartersPerHour)

export

   TimeTable
   
define
   
   HourLimit           = 180
   LecturesStartForDay = 8
   MaximumRooms        = 7
   Teacher1Off         = 3
   
   Week = [Monday Tuesday Wednesday Thursday Friday] 
   
   MorningQuarters = 18
   
   [MondayM TuesdayM WednesdayM ThursdayM FridayM]
   = {Map Week fun {$ S#_} S#(S + MorningQuarters - 1) end}

   Afternoon 
   [MondayA TuesdayA WednesdayA ThursdayA FridayA]
   = Afternoon
   = {Map Week fun {$ S#E} (S + MorningQuarters)#E end}


   fun {DayTimeToQuarters Day FH FQ TH TQ}
      Left Right Offset in
      Offset = (36 * (Day.1 div QuartersPerDay) + 1) 
      Left  = Offset + (FH - LecturesStartForDay) * QuartersPerHour + FQ - 1
      Right = Offset + (TH-LecturesStartForDay) * QuartersPerHour + TQ - 1
      Left#Right
   end

   AstaTime = {DayTimeToQuarters Tuesday 12 3 13 3}


   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %% Lecture and Professor Manipulation :-} Functions
   %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   local
   
      DayMap= map(monday:    Monday
		  tuesday:   Tuesday
		  wednesday: Wednesday
		  thursday:  Thursday
		  friday:    Friday)
   
      %% Translate the constraint description
      %% into a domain that can stand after /* :: and \:: */
      fun {MakeC Desc}
	 case Desc
	 of inDays(L) then {Map L fun {$ D} DayMap.D end}
	 [] weekInterval(SH#SM#EH#EM) then
	    {Map Week
	     fun {$ Day}
		{DayTimeToQuarters Day SH SM div 15 EH EM div 15}
	     end}
	 [] dayInterval(Day#SH#SM#EH#EM) then
	    [{DayTimeToQuarters DayMap.Day SH (SM div 15) EH (EM div 15)}]
	 [] fix(Day#H#M) then
	    [{DayTimeToQuarters DayMap.Day H (M div 15) H (M div 15)}]
	 else {Record.foldR Desc
	       fun {$ D In} {Append {MakeC D} In} end nil}
	 end
      end

      %% compute the constraint procedure
      %% for a lecturer from his/her constraint description
      proc {ApplyConstraint Desc X}
	 case {Label Desc}
	 of noT  then  X :: compl({MakeC Desc.1})
	 [] nil	 then  skip
	 else          X :: {MakeC Desc}
	 end
      end

      %% compute the lectures of a prof
      %% from the lecture list
      fun {ProfToLectures PName Ls}
	 {Filter Ls fun {$ L} L.professor==PName end}
      end

   in
      
      fun {MakeLectures LecturesDescs Semester Ordering}
	 {Map {Record.toList LecturesDescs.Semester}
	  fun {$ Ls}
	     {Map Ls
	      fun {$ L}
		 Start = {FD.int 1#(HourLimit-L.dur)}
	      in
		 if {HasFeature L constraints}
		 then {ApplyConstraint L.constraints Start}
		 else skip
		 end
		 {Adjoin L l(start:    Start
			     ordering: Ordering
			     semester: Semester)}
	      end}
	  end}
      end
      
      %% translate the prof record into a
      %% list of records, containing constraint procedures
      fun {MakeProfessors ProfDesc Ls}
	 {Record.foldRInd ProfDesc
	  fun {$ I X In}
	     PLs={ProfToLectures I Ls}
	  in
	     {ForAll PLs proc {$ PL} {ApplyConstraint X PL.start} end}
	     professor(lectures:PLs name:I) | In
	  end
	  nil}
      end
   end

   %% get Lectures with size out of a given list of Specifiers
   fun {FilterSize Specifiers Lectures}
      {Filter Lectures fun {$ X} {Member X.size Specifiers} end}
   end

   %% make record for all lectures for easy access
   fun {MakeLectureRecord Flats}
      {List.toRecord lectures {Map Flats fun {$ L} L.name#L end}}
   end
   
   %% search for professor in list of profs
   fun {SearchProf Profs Name}
      {List.dropWhile Profs fun {$ P} P.name \= Name end}.1
   end
   
   %% %%%%%%%%%%%
   %% Enumeration
   %% %%%%%%%%%%%

   local
      fun {SkipWait Xs}
	 case Xs
	 of X|Xr then case {FD.reflect.size X.start}
		      of 1 then {SkipWait Xr}
		      else Xs
		      end
	 [] nil then nil
	 end
      end
      
      proc {Choose Xs HoleTail HoleHead MinYet SizeYet ?Min ?Ys}
	 case Xs
	 of X|Xr then 
	    SizeX = {FD.reflect.size X.start} in
	    case SizeX of 1 then 
	       {Choose Xr HoleTail HoleHead MinYet SizeYet Min Ys}
	    elseif SizeX < SizeYet then
	       NewHole in
	       HoleTail = MinYet | HoleHead
	       {Choose Xr Ys NewHole X SizeX Min NewHole}
	    else
	       Ys=X | {Choose Xr HoleTail HoleHead MinYet SizeYet Min}
	    end
	 [] nil then Min = MinYet Ys = nil HoleTail = HoleHead
	 end
      end
      fun {Cost I Ls C}
	 case Ls of nil then
	    {Exception.raiseError college(Cost [I Ls C] 'tragic error')}
	    _
	 [] A#B|Lr then
	    if A =< I andthen I =< B
	    then C
	    else {Cost I Lr C+1}
	    end
	 end
      end

      fun {GetFirst Domain Min MinVal Ordering}
	 case Domain of nil then Min
	 [] D|Dr
	 then TMinVal = {Cost D Ordering 1} in
	    if TMinVal<MinVal then {GetFirst Dr D TMinVal Ordering}
	    else {GetFirst Dr Min MinVal Ordering}
	    end
	 end
      end
   in
      proc {Enum Xs}
	 choice
	    case {SkipWait Xs}
	    of nil then skip
	    [] X|Xr then
	       Y Yr Hole Val YDom YVar in
	       {Choose Xr Yr Hole X {FD.reflect.size X.start} Y Hole}
	       {Wait Y}
	       YVar = Y.start
	       YDom = {FD.reflect.domList YVar}
	       Val = {GetFirst YDom.2 YDom.1 {Cost YDom.1 Y.ordering 1}
		      Y.ordering}
	       choice
		  YVar=Val {Enum Yr}
	       []
		  YVar\=:Val {Enum Y|Yr}
	       end
	    end
	 end
      end
   end

   %% %%%%%%%%%%%%%%%%%%%%%
   %% Branch and Bound Cost Function
   %% %%%%%%%%%%%%%%%%%%%%%
   local 
      fun {SumUp Lectures Time}
	 case Lectures of nil then nil
	 [] L|Lr
	 then case {Atom.toString L.name}.1
	      of &M then 0|{SumUp Lr Time}
	      [] &F then 0|{SumUp Lr Time}
	      else (L.start::Time)|{SumUp Lr Time}
	      end
	 end
      end
   in
      fun {Cost Lectures}
	 Costs = {FD.decl} in
	 {FD.sum {SumUp Lectures [MondayA TuesdayA WednesdayA ThursdayA FridayA]} '=:' Costs}
	 Costs
      end
   end
   

   %% %%%%%%%%%%%%%%%%%%%%%
   %% Constraint Procedures
   %% %%%%%%%%%%%%%%%%%%%%%

   proc {OnSpecialDayOnly Lectures Day}
      {ForAll Lectures proc {$ Lec} Lec.start :: Day end}
   end
   proc {OnSpecialTimeOnly Lectures Time}
      {ForAll Lectures proc {$ Lec} Lec.start :: Time end}
   end
   proc {ForbidAsta Lectures}
      {ForAll Lectures
       proc {$ Lec} Lec.start :: compl((AstaTime.1-Lec.dur+1)#AstaTime.2) end}
   end
   proc {DayBreak Break Lectures}
      {ForAll Lectures
       proc {$ Lec}
	  {ForAll Break proc{$ B}
			   Left = (B.1-Lec.dur)
			in
			   if Left < 0 then
			      Lec.start :: compl(0#B.2)
			   else
			      Lec.start :: compl(Left#B.2)
			   end
			end}
       end}
   end
   
   proc {NotParallel Offset Teacher1 Teacher15}
      {ForAll Teacher1.lectures
       proc {$ FL}
	  {ForAll Teacher15.lectures
	   proc{$ LFL}
	      {FD.disjoint FL.start FL.dur+Offset LFL.start LFL.dur+Offset}
	   end}
       end}
   end

   %% The lectures of a semester must not overlap
   
   proc {NoOverlapSemester Lectures}
      case Lectures of nil then skip
      [] L|Lr
      then {NoOverlap1 Lr L} {NoOverlapSemester Lr}
      end
   end
   proc {NoOverlap1 Lecs Lec1}
      case Lecs of nil then skip
      [] L|Lr
      then {NoOverlap2 Lec1 L} {NoOverlap1 Lr Lec1}
      end
   end
   proc {NoOverlap2 L1 L2}
      {ForAll L1
       proc{$ L}
	  {ForAll L2
	   proc {$ LP}
	      %% After one hour must be a quarter
	      %% and after two hours two quarters
	      %% recreation time
	      LDur LPDur in
	      LDur= if L.dur<4
		    then L.dur+1
		    else L.dur+2
		    end
	      LPDur= if LP.dur<4
		     then LP.dur+1
		     else LP.dur+2
		     end
	      {FD.disjoint L.start LDur LP.start LPDur}
	   end}
       end}
   end

   %% The lectures of a professor must not overlap
   proc {NoOverlapLectures Lectures}
      case Lectures of nil then skip
      [] L|Lr
      then {NoOverlapLecs1 L Lr} {NoOverlapLectures Lr}
      end
   end

   proc {NoOverlapLecs1 Lec Lec1}
      case Lec1 of nil then skip
      [] L|Lr
      then {NoOverlapLecs2 Lec L} {NoOverlapLecs1 Lec Lr}
      end
   end
   
   proc {NoOverlapLecs2 L1 L2}
      %% After one hour must be a quarter
      %% and after two hours two quarters
      %% recreation time
      L1Dur L2Dur in
      L1Dur= if L1.dur < 4 then L1.dur + 1 else L1.dur + 2 end
      L2Dur= if L2.dur < 4 then L2.dur + 1 else L2.dur + 2
	     end
      {FD.disjoint L1.start L1Dur L2.start L2Dur}
   end

   %% Constraint AtMostLectures for rooms
   
   local 
      fun {SumUpLectures Lectures Hour}
	 case Lectures
	 of L|Lr
	 then
	    %% Rooms are empty for a quarter after each lecture
	    Left = Hour-(L.dur+1)+1
	 in 
	    if Left < 0 then (L.start :: 0#Hour)|{SumUpLectures Lr Hour}
	    else             (L.start :: Left#Hour)|{SumUpLectures Lr Hour}
	    end
	 [] nil then nil
	 end
      end
   in
      proc {AtMostLectures Lectures Limit}
	 {For 1 HourLimit 1
	  proc{$ Hour}
	     {FD.sum {SumUpLectures Lectures Hour} '=<:' Limit}
	  end}
      end
   end

   %% Constraint ThreeDaysOnly and OnDifferentDays

   local
      fun {SumUp Lectures Day}
	 case Lectures of nil then nil
	 [] L|Lr
	 then (L.start :: Day)|{SumUp Lr Day}
	 end
      end
      proc {SumUpDays Lectures BDay Day}
	 S= {SumUp Lectures Day}
      in
	 BDay = ({FoldL S FD.plus 0} >=: 1)
      end
   in
      proc {ThreeDaysOnly Lectures DayLimit}
	 [BMo BTu BWe BTh BFr]={FD.dom 0#1}
      in
	 BMo+BTu+BWe+BTh+BFr =<: DayLimit
	 {SumUpDays Lectures BMo Monday}
	 {SumUpDays Lectures BTu Tuesday}
	 {SumUpDays Lectures BWe Wednesday}
	 {SumUpDays Lectures BTh Thursday}
	 {SumUpDays Lectures BFr Friday}
      end
      proc {OnDifferentDays Lectures}
	 [BMo BTu BWe BTh BFr]={FD.dom 0#1}
      in
	 BMo+BTu+BWe+BTh+BFr =: {Length Lectures}
	 {SumUpDays Lectures BMo Monday}
	 {SumUpDays Lectures BTu Tuesday}
	 {SumUpDays Lectures BWe Wednesday}
	 {SumUpDays Lectures BTh Thursday}
	 {SumUpDays Lectures BFr Friday}
      end
      proc {OnSameDay Lectures}
	 [BMo BTu BWe BTh BFr]={FD.dom 0#1}
      in
	 BMo+BTu+BWe+BTh+BFr =: 1
	 {SumUpDays Lectures BMo Monday}
	 {SumUpDays Lectures BTu Tuesday}
	 {SumUpDays Lectures BWe Wednesday}
	 {SumUpDays Lectures BTh Thursday}
	 {SumUpDays Lectures BFr Friday}
      end	 
   end
   local
      fun {SumUpOverlaps L1 Lectures}
	 case Lectures of nil then nil
	 [] L2|Lr then B={FD.int 0#1} L1Dur L2Dur in
	    if L1.name==L2.name then 0|{SumUpOverlaps L1 Lr}
	    else L1S L2S in
	       L1Dur= if L1.dur < 4 then L1.dur + 1 else L1.dur + 2 end
	       L2Dur= if L2.dur < 4 then L2.dur + 1 else L2.dur + 2 end
	       L1S = L1.start
	       L2S = L2.start

	       /* That is the (operational) semantics of FD.tasksOverlap.
	          FD.tasksOverlap implements constructive disjunction.
	       condis
		  B=:1
		  L1S + L1Dur >: L2S
		  L2S + L2Dur >: L1S
	       []
		  B=:0
		  L1S + L1Dur =<: L2S
	       []
		  B=:0
		  L2S + L2Dur =<: L1S
	       end
	       */
	       {FD.tasksOverlap L1S L1Dur L2S L2Dur B}
	       B|{SumUpOverlaps L1 Lr}
	    end
	 end
      end 
   in
      proc {AtmostOneOverlap Lecture Lectures}
	 {FD.sum {SumUpOverlaps Lecture Lectures} '=<:' 1}
      end
   end

 
   %% %%%%%%%%%%%
   %% The Problem
   %% %%%%%%%%%%%

   %% MakeProblem makes the problem procedure
   %% using the problem description as global variable
   
   fun {MakeProblem ProblemDescription}

      proc {$ FlatAllLectures}
	 
	 %% %%%%%%%%
	 %% Lectures
	 %% %%%%%%%%
	 
	 LecturesDescription = ProblemDescription.lectures

	 %% Second Semester
	 %% %%%%%%%%%%%%%%%
	 
	 LecturesSecond = {MakeLectures
			   LecturesDescription
			   second
			   %% other priorities

			   [MondayM MondayA
			    TuesdayA TuesdayM
			    WednesdayM WednesdayA
			    FridayM  FridayA
			    ThursdayM ThursdayA
			   ]
			  }
	 %% Fourth Semester
	 %% %%%%%%%%%%%%%%%

	 LecturesFourth = {MakeLectures
			   LecturesDescription
			   fourth
			   %% other priorities

			   [
			    WednesdayM WednesdayA
			    TuesdayM TuesdayA
			    MondayM MondayA
			    ThursdayM ThursdayA
			    FridayM FridayA
			   ]


			  }
	 GrundStudiumLectures = {Append LecturesSecond LecturesFourth}
	 FlatGrundStudiumLectures = {List.flatten GrundStudiumLectures}

	 %% Sixth Semester
	 %% %%%%%%%%%%%%%%

	 LecturesSixth = {MakeLectures
			  LecturesDescription
			  sixth
			  %% other priorities

			  [ WednesdayM WednesdayA
			    FridayM FridayA
			    ThursdayM ThursdayA
			    MondayM MondayA
			    TuesdayM TuesdayA
			  ]
			 }

	 FlatLecturesSixth = {List.flatten LecturesSixth}

	 %% Eighth Semester
	 %% %%%%%%%%%%%%%%%

	 LecturesEighth = {MakeLectures
			   LecturesDescription
			   eighth
			   [
			    ThursdayM ThursdayA
			    WednesdayA WednesdayM
			    MondayA  MondayM
			    TuesdayA TuesdayM
			    FridayM FridayA
			   ]}
	 
	 FlatLecturesEighth = {List.flatten LecturesEighth}

	 NotOnThursdayLectures = {Append GrundStudiumLectures LecturesSixth}
	 AllSemesterLectures = {Append NotOnThursdayLectures LecturesEighth}
	 FlatAllSemesterLectures={List.flatten AllSemesterLectures}

	 %% Medien Lectures
	 %% %%%%%%%%%%%%%%%

	 MedienLectures = {MakeLectures
			   LecturesDescription
			   medien
			   [
			    ThursdayA TuesdayA
			    TuesdayM ThursdayM
			    WednesdayM WednesdayA
			    MondayM MondayA
			    FridayM FridayA
			   ]}

	 FlatMedienLectures = {List.flatten MedienLectures}
	 
	 %% Fakult Lectures
	 %% %%%%%%%%%%%%%%%

	 FakLectures = {MakeLectures
			LecturesDescription
			fac
			[
			 WednesdayM WednesdayA
			 ThursdayM ThursdayA
			 MondayM MondayA
			 FridayM FridayA
			 TuesdayM TuesdayA
			]
		       }

	 FlatFakLectures = {List.flatten FakLectures}

	 AllLectures = {Append AllSemesterLectures
			{Append MedienLectures FakLectures}}

	 !FlatAllLectures = {List.flatten AllLectures}

	 L = {MakeLectureRecord FlatAllLectures}
	 
	 %% %%%%%%%%%%
	 %% Professors
	 %% %%%%%%%%%%

	 ProfessorsDescription = ProblemDescription.professors

	 Professors = {MakeProfessors ProfessorsDescription FlatAllLectures}

      in

	 %% %%%%%%%%%%%%%%%
	 %% The Constraints
	 %% %%%%%%%%%%%%%%%

	 %% General constraints
	 %% Lecture must be finished at 17.00

	 {ForAll FlatAllLectures
	  proc{$ L}
	     Dur = L.dur
	     fun {AfterHours Day} Day.2 + 1 - Dur#Day.2  end
	  in
	     L.start :: compl({Map Week AfterHours})
	  end}


	 %% All semesters but the eighth are not scheduled on thursday
	 %% seems to be to hard
	 
	 {OnSpecialDayOnly FlatLecturesSixth Wednesday}
	 %% The break must be lecturefree

	 {DayBreak
	  {List.map [0 1 2 3 4] fun{$ I} (20+I*36)#(23+I*36) end}
	  {Append FlatLecturesEighth FlatGrundStudiumLectures}}
	 
	 {DayBreak {List.map [0 1 2 3 4]
		    fun{$ I} (16+I*36)#(19+I*36) end}
	  FlatLecturesSixth}

	 %% At Asta time no lectures allowed
	 {ForbidAsta FlatAllLectures}

	 %% The Lectures for a semester must not overlap
	 {NoOverlapSemester [[L.'2.1' L.'2.2' L.'2.3']
			     [L.'2.6']
			     [L.'2.7']
			     [ L.'2.9']
			     [L.'2.10.1' L.'2.10.2' L.'2.10.3']
			     [L.'2.5.1' L.'2.5.2' L.'2.8.1' L.'2.8.2'
			      L.'2.13.1' L.'2.13.2' L.'2.4.1' L.'2.4.2'
			      L.'2.14.1' L.'2.14.2' L.'2.15.1' L.'2.15.2'
			      L.'2.15.3' L.'2.15.4' ]]}

	 {ForAll [L.'2.5.1' L.'2.5.2' L.'2.8.1' L.'2.8.2'
		  L.'2.13.1' L.'2.13.2' L.'2.4.1' L.'2.4.2'
		  L.'2.14.1' L.'2.14.2']
	  proc{$ Lecture}
	     {AtmostOneOverlap Lecture [L.'2.5.1' L.'2.5.2' L.'2.8.1'
					L.'2.8.2' L.'2.13.1' L.'2.13.2'
					L.'2.4.1' L.'2.4.2' L.'2.14.1'
					L.'2.14.2' L.'2.15.1' L.'2.15.2'
					L.'2.15.3' L.'2.15.4']}
	  end}
	 {ForAll [L.'2.15.1' L.'2.15.2' L.'2.15.3' L.'2.15.4']
	  proc{$ Lecture}
	     {AtmostOneOverlap Lecture [L.'2.5.1' L.'2.5.2' L.'2.8.1'
					L.'2.8.2' L.'2.13.1' L.'2.13.2'
					L.'2.4.1' L.'2.4.2' L.'2.14.1'
					L.'2.14.2']}
	  end}


	 {NoOverlapSemester [[L.'4.1'] [L.'4.6'] [L.'4.11' ]
			     [ L.'4.2.1' L.'4.2.2' L.'4.3.1' L.'4.3.2'
			       L.'4.4.1' L.'4.4.2' L.'4.5.1' L.'4.5.2' 
			       L.'4.7.1' L.'4.7.2' L.'4.10.1' L.'4.10.2' 
			       L.'4.12.1' L.'4.12.2'
			       L.'4.8.1' L.'4.8.2' L.'4.8.3' L.'4.8.4']]}

	 {ForAll [ L.'4.2.1' L.'4.2.2' L.'4.3.1' L.'4.3.2'
		   L.'4.4.1' L.'4.4.2' L.'4.5.1' L.'4.5.2' 
		   L.'4.7.1' L.'4.7.2' L.'4.10.1' L.'4.10.2'
		   L.'4.12.1' L.'4.12.2']
	  proc{$ Lecture}
	     {AtmostOneOverlap
	      Lecture [ L.'4.2.1' L.'4.2.2' L.'4.3.1' L.'4.3.2'
			L.'4.4.1' L.'4.4.2' L.'4.5.1' L.'4.5.2' 
			L.'4.7.1' L.'4.7.2' L.'4.10.1' L.'4.10.2'
			L.'4.12.1' L.'4.12.2'
			L.'4.8.1' L.'4.8.2' L.'4.8.3' L.'4.8.4']}
	  end}

	 {ForAll [L.'4.8.1' L.'4.8.2' L.'4.8.3' L.'4.8.4']
	  proc{$ Lecture}
	     {AtmostOneOverlap
	      Lecture [ L.'4.2.1' L.'4.2.2' L.'4.3.1' L.'4.3.2'
			L.'4.4.1' L.'4.4.2' L.'4.5.1' L.'4.5.2' 
			L.'4.7.1' L.'4.7.2' L.'4.10.1' L.'4.10.2'
			L.'4.12.1' L.'4.12.2' ]}
	  end}



	 {NoOverlapSemester [[L.'6.2.1' L.'6.2.2'] [L.'6.5' L.'6.6'] ]}
	 {NoOverlapSemester [[L.'8.1'][L.'8.2'][L.'8.3'][L.'8.4']
			     [L.'8.5'][L.'8.6'][L.'8.7.2'][L.'8.8']
			     [L.'8.9'][L.'8.10']
			    ]}

	 %% Lectures for a professor must not overlap
	 {ForAll Professors proc{$ L} {NoOverlapLectures L.lectures} end}

	 %% At most MaximumRooms concurrent
	 {AtMostLectures {FilterSize [big small tiny] FlatAllLectures}
	  MaximumRooms}

	 %% Aula and 155 lectures at most 2
	 {AtMostLectures {FilterSize [big] FlatAllLectures} 2}

	 %% Rooms Aula, 155, 152, 154, 250 lectures at most 5
	 {AtMostLectures {FilterSize [big small] FlatAllLectures} 5}

	 %% All professors have a teaching limit of 3 days per week
	 {ForAll Professors
	  proc{$ Professor} {ThreeDaysOnly Professor.lectures 3} end}

	 %% At most 6 hours per day
	 {ForAll Professors
	  proc{$ Professor}
	     {ForAll [Monday Tuesday Wednesday Thursday Friday]
	      proc{$ Day}
		 {FoldL Professor.lectures
		  fun{$ I L}
		     {FD.plus {FD.times L.dur (L.start::Day)} I}
		  end 0} =<: 6*4
	      end}
	  end}
	 
	 %% Teacher1/Teacher15 couple cannot teach at the same time (+offset)
	 {NotParallel
	  Teacher1Off
	  {SearchProf Professors 'Teacher1'}
	  {SearchProf Professors 'Teacher15'}}
	 
	 %% Special constraints for 2nd semester
	 L.'2.1'.start=L.'2.2'.start=L.'2.3'.start
	 L.'2.10.1'.start=L.'2.10.2'.start=L.'2.10.3'.start
	 L.'2.15.1'.start=L.'2.15.2'.start=L.'2.15.3'.start=L.'2.15.4'.start 

	 %% Special constraints for 4th semester
	 {OnDifferentDays [L.'4.7.1' L.'4.7.2']}  % Teacher17's lecture
	 
	 %% Special constraints for 6th semester
	 L.'6.5'.start=L.'6.6'.start
	 (L.'6.5'.start) >: {FD.max L.'6.2.1'.start L.'6.2.2'.start}

	 L.'6.2.1'.start=L.'6.2.2'.start=L.'6.2.3'.start

	 %% Special constraints for eighth semester

	 %% Special constraints for Mediendidaktik
	 {NoOverlapSemester [FlatGrundStudiumLectures FlatMedienLectures]}

	 %% Special constraints for Fakultative Veranstaltungen
	 {NoOverlapSemester FakLectures}
	 {OnSpecialTimeOnly FlatFakLectures Afternoon}

	 %% Special constraint for Teacher32
	 {OnSameDay [L.'F.2.1' L.'F.2.2']}
	 
	 %% %%%%%%%%%%%%%%%
	 %% The Enumeration
	 %% %%%%%%%%%%%%%%%

	 {Enum FlatAllSemesterLectures}
	 {Enum {Append FlatMedienLectures FlatFakLectures}}
      end
   end      

   class TimeTableClass from BaseObject
      attr
	 problemDescription: nil
	 solution
	 dir
	 first: true
	 solver

      meth readProblem(InputFileName)
	 if InputFileName==false then skip
	 else
	    InputFile = {New Open.file init(name:InputFileName
					    flags: [read])}
	    Read = {InputFile read(size:all list:$)}
	 in
	    problemDescription <- {Compiler.virtualStringToValue Read}
	 end
      end

      meth setDir(D)
	 dir <- D
      end

      meth getDir(?D)
	 D = @dir 
      end

      meth setProblem(P)
	 problemDescription <- P
      end

      meth inspectProblem
	 {Inspect @problemDescription}
      end

      meth constrainProblem
	 solution <- {{MakeProblem @problemDescription}}
      end

      meth solveProblem
	 PD = @problemDescription
      in
	 first <- true
	 if PD == nil then
	    {ControllerLabel tk(configure text:'No problem loaded')}
	 else
	    solver <- {New Search.object
		       script({MakeProblem PD}
			      proc{$ Old New}
				 CN = {Cost New}
				 CO = {Cost Old} in
				 CN <: CO
				 thread
				    {Wait CO}
				    /*{Show {FD.reflect.min CO}}*/
				 end
			      end
			      rcd : 20)}
	    local Solver = @solver in 
	       {self setSolution(thread {Solver next($)} end)}
	    end
	 end
      end
      meth optimizeProblem
	 Solver = @solver in
	 {ControllerLabel tk(configure text:'Computing...')}
	 {self setSolution(thread {Solver next($)} end)}
      end
      meth setSolution(Sol)
	 thread
	    if Sol==nil then
	       {ControllerLabel tk(configure text:'No solution')}
	    elseif Sol==stopped then
	       {ControllerLabel tk(configure text:'Stopped')}
	    else
	       {self help(Sol)}
	    end
	 end
      end
      meth help(Sol)
	 Solver = @solver in 
	 solution <- Sol
	 if @first  then
	    first <- false
	    {self graphic}
	 else skip
	 end
	 {self setSolution(thread {Solver next($)} end)}
      end
      meth get(?Sol)
	 Sol=@solution
      end
      meth anyTime
	 {@solver stop}
      end
      meth inspect
	 {Inspect {Map @solution fun {$ X} {Record.subtract X ordering} end}}
      end
      meth text
	 {LaTeX show @solution}
      end
      meth print
	 {LaTeX print @solution}
      end
      meth graphic
	 {DrawSchedule @solution.1 TopWindow}
      end
      meth edit(FileName)
	 if FileName==false then skip
	 else 
	    {OS.system Editor#' '#FileName#'\&' _}
	 end
      end
      meth save(OutputFileName)
	 if OutputFileName==false then skip
	 else 
	    F = {New Open.file init(name:OutputFileName
				    flags: [read write 'create'])}
	 in
	    {F write(vs:{Value.toVirtualString @solution 100 100})}
	 end
      end
\ifndef ALONEDEMO
      meth read(FileName)
	 if FileName==false then skip
	 else 
	    File = {New Open.file init(name:FileName
				       flags: [read write 'create'])}
	 in
	    solution <- {Compiler.virtualStringToValue
			 {File read(size:all list:$)}}
	 end
      end
\else
      meth read(_)
	 solution <- 
	 \insert KathHochWS9596Sol.ozt
      end
\endif
   end
   
   TimeTable = {New TimeTableClass	
\ifdef ALONEDEMO
		setProblem(
		   \insert KathHochWS9596.ozt
		   )
\else
		noop
\endif
	       }
end
