%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org/
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   FD Schedule
export
   Script Order
prepare

   MT10=
   mt10(
      tasks:[pa(dur:0)
	     a1(dur:29  pre:[pa] res:m1)  a2(dur:78  pre:[a1] res:m2) 
	     a3(dur: 9  pre:[a2] res:m3)  a4(dur:36  pre:[a3] res:m4) 
	     a5(dur:49  pre:[a4] res:m5)  a6(dur:11  pre:[a5] res:m6) 
	     a7(dur:62  pre:[a6] res:m7)  a8(dur:56  pre:[a7] res:m8) 
	     a9(dur:44  pre:[a8] res:m9)  a10(dur:21 pre:[a9] res:m10) 
	     b1(dur:43  pre:[pa] res:m1)  b2(dur:90  pre:[b1] res:m3) 
	     b3(dur:75  pre:[b2] res:m5)  b4(dur:11  pre:[b3] res:m10) 
	     b5(dur:69  pre:[b4] res:m4)  b6(dur:28  pre:[b5] res:m2) 
	     b7(dur:46  pre:[b6] res:m7)  b8(dur:46  pre:[b7] res:m6) 
	     b9(dur:72  pre:[b8] res:m8)  b10(dur:30 pre:[b9] res:m9) 
	     c1(dur:91  pre:[pa] res:m2)  c2(dur:85  pre:[c1] res:m1) 
	     c3(dur:39  pre:[c2] res:m4)  c4(dur:74  pre:[c3] res:m3) 
	     c5(dur:90  pre:[c4] res:m9)  c6(dur:10  pre:[c5] res:m6) 
	     c7(dur:12  pre:[c6] res:m8)  c8(dur:89  pre:[c7] res:m7) 
	     c9(dur:45  pre:[c8] res:m10) c10(dur:33 pre:[c9] res:m5) 
	     d1(dur:81  pre:[pa] res:m2)  d2(dur:95  pre:[d1] res:m3) 
	     d3(dur:71  pre:[d2] res:m1)  d4(dur:99  pre:[d3] res:m5) 
	     d5(dur: 9  pre:[d4] res:m7)  d6(dur:52  pre:[d5] res:m9) 
	     d7(dur:85  pre:[d6] res:m8)  d8(dur:98  pre:[d7] res:m4) 
	     d9(dur:22  pre:[d8] res:m10) d10(dur:43 pre:[d9] res:m6) 
	     e1(dur:14  pre:[pa] res:m3)  e2(dur: 6  pre:[e1] res:m1) 
	     e3(dur:22  pre:[e2] res:m2)  e4(dur:61  pre:[e3] res:m6) 
	     e5(dur:26  pre:[e4] res:m4)  e6(dur:69  pre:[e5] res:m5) 
	     e7(dur:21  pre:[e6] res:m9)  e8(dur:49  pre:[e7] res:m8) 
	     e9(dur:72  pre:[e8] res:m10) e10(dur:53 pre:[e9] res:m7) 
	     f1(dur:84  pre:[pa] res:m3)  f2(dur: 2  pre:[f1] res:m2) 
	     f3(dur:52  pre:[f2] res:m6)  f4(dur:95  pre:[f3] res:m4) 
	     f5(dur:48  pre:[f4] res:m9)  f6(dur:72  pre:[f5] res:m10) 
	     f7(dur:47  pre:[f6] res:m1)  f8(dur:65  pre:[f7] res:m7) 
	     f9(dur: 6  pre:[f8] res:m5)  f10(dur:25 pre:[f9] res:m8) 
	     g1(dur:46  pre:[pa] res:m2)  g2(dur:37  pre:[g1] res:m1) 
	     g3(dur:61  pre:[g2] res:m4)  g4(dur:13  pre:[g3] res:m3) 
	     g5(dur:32  pre:[g4] res:m7)  g6(dur:21  pre:[g5] res:m6) 
	     g7(dur:32  pre:[g6] res:m10) g8(dur:89  pre:[g7] res:m9) 
	     g9(dur:30  pre:[g8] res:m8)  g10(dur:55 pre:[g9] res:m5) 
	     h1(dur:31  pre:[pa] res:m3)  h2(dur:86  pre:[h1] res:m1) 
	     h3(dur:46  pre:[h2] res:m2)  h4(dur:74  pre:[h3] res:m6) 
	     h5(dur:32  pre:[h4] res:m5)  h6(dur:88  pre:[h5] res:m7) 
	     h7(dur:19  pre:[h6] res:m9)  h8(dur:48  pre:[h7] res:m10) 
	     h9(dur:36  pre:[h8] res:m8)  h10(dur:79 pre:[h9] res:m4) 
	     i1(dur:76  pre:[pa] res:m1)  i2(dur:69  pre:[i1] res:m2) 
	     i3(dur:76  pre:[i2] res:m4)  i4(dur:51  pre:[i3] res:m6) 
	     i5(dur:85  pre:[i4] res:m3)  i6(dur:11  pre:[i5] res:m10) 
	     i7(dur:40  pre:[i6] res:m7)  i8(dur:89  pre:[i7] res:m8) 
	     i9(dur:26  pre:[i8] res:m5)  i10(dur:74 pre:[i9] res:m9) 
	     j1(dur:85  pre:[pa] res:m2)  j2(dur:13  pre:[j1] res:m1) 
	     j3(dur:61  pre:[j2] res:m3)  j4(dur: 7  pre:[j3] res:m7) 
	     j5(dur:64  pre:[j4] res:m9)  j6(dur:76  pre:[j5] res:m10) 
	     j7(dur:47  pre:[j6] res:m6)  j8(dur:52  pre:[j7] res:m4) 
	     j9(dur:90  pre:[j8] res:m5)  j10(dur:45 pre:[j9] res:m8) 
	     pe(dur:0 pre:[a10 b10 c10 d10 e10 f10 g10 h10 i10 j10])])

define
   
   fun {GetDur TaskSpec}
      {List.toRecord dur {Map TaskSpec fun {$ T}
					  {Label T}#T.dur
				       end}}
   end

   fun {GetStart TaskSpec}
      MaxTime = {FoldL TaskSpec fun {$ Time T} 
				   Time+T.dur
				end 0}
      Tasks   = {Map TaskSpec Label}
   in
      {FD.record start Tasks 0#MaxTime}
   end
   
   fun {GetTasksOnResource TaskSpec}
      D={Dictionary.new}
   in
      {ForAll TaskSpec 
       proc {$ T}
	  if {HasFeature T res} then R=T.res in
	     {Dictionary.put D R {Label T}|{Dictionary.condGet D R nil}}
	  end
       end}
      {Dictionary.toRecord tor D}
   end

   fun {Compile Spec}
      TaskSpec    = Spec.tasks
      Constraints = if {HasFeature Spec constraints} then
		       Spec.constraints
		    else
		       proc {$ _ _} 
			  skip
		       end
		    end
      Dur         = {GetDur TaskSpec}
      TasksOnRes  = {GetTasksOnResource TaskSpec}
   in
      proc {$ Start}
	 Start = {GetStart TaskSpec}
	 {ForAll TaskSpec
	  proc {$ T}
	     {ForAll {CondSelect T pre nil}
	      proc {$ P}
		 Start.P + Dur.P =<: Start.{Label T}
	      end}
	  end}
	 {Constraints Start Dur}
	 {Schedule.serialized   TasksOnRes Start Dur}
	 {Schedule.firstsLastsDist TasksOnRes Start Dur}
	 {FD.assign min Start}
      end
   end

   Script = {Compile MT10}

   proc {Order Old New}
      Old.pe >: New.pe
   end
   
end

