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
   
export
   
   DrawSchedule
   
import

   Common(monday:         Monday
	  tuesday:        Tuesday
	  wednesday:      Wednesday
	  thursday:       Thursday
	  friday:         Friday
	  quartersPerDay: QuartersPerDay)

   Scheduler(timeTable:      TimeTable)
   
   Tk
   
define
   
   Colors    = if Tk.isColor then
		  colors('2': lightyellow1
			 '4': lightsalmon2
			 '6': cyan3
			 '8': lightgoldenrod1
			 'M': mistyrose3
			 'F': tomato)
	       else
		  colors('2': white
			 '4': white
			 '6': white
			 '8': white
			 'M': white
			 'F': white)
	       end
   
   RoomLimit      = 10
   HeightLecture  = 15
   DayHeight      = (RoomLimit + 1) * HeightLecture
   YOff           = 6
   XOff           = 10
   CanvasWidth    = 720
   Roff           = 2
   CanvasHeight   = 5 * DayHeight
   Quarter        = 20 
   WeekDaysWidth  = 90
   TimeLineY      = 30
   RoomMarkOffset = ~3
   Font           = 'lucidasanstypewriter-12'

   proc {GetEarliest Until Ind Lecture Canvas Day NewUntil}
      if Ind > RoomLimit then
	 fail
      elseif Until.Ind =< Lecture.start then
	 {Canvas
	  tk(crea rectangle
	     ((Lecture.start-1) mod 36)*Quarter+Roff
	     (Day-1)*DayHeight+HeightLecture*(Ind-1)+TimeLineY
	     (((Lecture.start-1) mod 36)+(Lecture.dur))*Quarter+Roff
	     (Day-1)*DayHeight+HeightLecture*(Ind)+TimeLineY
	     fill:Colors.{String.toAtom
			  [{Atom.toString Lecture.name}.1]})}
	 case Lecture.size of big
	 then {Canvas tk(crea line
			 (((Lecture.start-1) mod 36)+
			  (Lecture.dur))*Quarter+RoomMarkOffset*2+Roff
			 (Day-1)*DayHeight+HeightLecture*(Ind-1)+
			 TimeLineY
			 (((Lecture.start-1) mod 36)+
			  (Lecture.dur))*Quarter+RoomMarkOffset*2+Roff
			 (Day-1)*DayHeight+HeightLecture*(Ind)+
			 TimeLineY)}
	 else skip
	 end
	 if  Lecture.size==big orelse Lecture.size==small then 
	    {Canvas tk(crea line
		       (((Lecture.start-1) mod 36)+
			(Lecture.dur))*Quarter+RoomMarkOffset+Roff
		       (Day-1)*DayHeight+HeightLecture*(Ind-1)+
		       TimeLineY
		       (((Lecture.start-1) mod 36)+
			(Lecture.dur))*Quarter+RoomMarkOffset+Roff
		       (Day-1)*DayHeight+HeightLecture*(Ind)+
		       TimeLineY) }
	 else skip
	 end
	 {Canvas
	  tk(crea text
	     ((Lecture.start-1) mod 36)*Quarter+XOff+Roff
	     (Day-1)*DayHeight+HeightLecture*(Ind-1)+YOff+TimeLineY
	     text : Lecture.name
	     anchor:w
	     font:Font)}
	 NewUntil = {AdjoinAt Until Ind Lecture.start+Lecture.dur}
	   else {GetEarliest Until Ind+1 Lecture Canvas Day NewUntil}
      end
   end
   
   proc {DoDisplay SortedLectures Day Canvas Until}
      case SortedLectures of nil then skip
      [] L|Lr then
	 NewUntil = {GetEarliest Until 1 L Canvas Day} in
	 {DoDisplay Lr Day Canvas NewUntil}
      end
   end

   
   proc {DrawSchedule FlatSols Parent}
      W
      CanvasColor = if Tk.isColor then mediumturquoise else white end
      Canvas WeekDays
   in
      {TimeTable save("CurrentOut.ozt")}
      
      W = {New Tk.toplevel tkInit(parent:Parent background:CanvasColor)} 
      {Tk.send wm(title W
		  " Katholische Hochschule fuer Soziale Arbeit -- Wintersemester 1995/96 ")}
      Canvas = {New Tk.canvas tkInit(parent: W 
				     width:  CanvasWidth
				     height: CanvasHeight)}
      WeekDays = {New Tk.canvas tkInit(parent: W
				       width:  WeekDaysWidth
				       height: CanvasHeight)}

      {List.forAllInd ['Monday' 'Tuesday' 'Wednesday' 'Thursday' 'Friday']
       proc {$ Ind Day}
	  {WeekDays tk(crea text 5 Ind*DayHeight-(DayHeight div 2)+18
		       text:Day anchor:w)}
       end}

      {Tk.send pack(WeekDays Canvas side:left)}

      {For 3 QuartersPerDay 4
       proc{$ I} 
	  Text={Int.toString 8 + (I+1)div 4}
       in 
	  {Canvas tk(crea line I*Quarter 0 I*Quarter 10)}
	  {Canvas tk(crea text I*Quarter 2*10 text: Text)} 
       end}
      
      {List.forAllInd [Monday Tuesday Wednesday Thursday Friday]
       proc{$ Ind Day}
	  LecturesOnThatDay = {List.filter FlatSols
			       fun {$ L}
				  %% We know that start time is fixed!
				  thread 
				     cond L.start::Day then true else false end
				  end
			       end}
	  SortedLectures = {Sort LecturesOnThatDay
			    fun {$ L1 L2} L1.start<L2.start end}
	  TupleUntil = {MakeTuple until RoomLimit} 
       in
	  {Record.forAll TupleUntil fun{$} 0 end} 
	  {DoDisplay SortedLectures Ind Canvas TupleUntil}
	  if Ind < 5 then
	     {Canvas tk(crea line 0 Ind*DayHeight+3*HeightLecture div 2
			CanvasWidth Ind*DayHeight+3*HeightLecture div 2)}
	  else skip
	  end
       end}
   end
end
