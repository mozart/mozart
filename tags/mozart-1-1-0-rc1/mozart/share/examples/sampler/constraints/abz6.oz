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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

declare
ABZ6 =
abz6(
   tasks:[pa(dur: 0) 
          a1(dur:62  pre:[pa] res:m7) a2(dur:24  pre:[a1] res:m8) 
          a3(dur:25  pre:[a2] res:m5) a4(dur:84  pre:[a3] res:m3) 
          a5(dur:47  pre:[a4] res:m4) a6(dur:38  pre:[a5] res:m6) 
          a7(dur:82  pre:[a6] res:m2) a8(dur:93  pre:[a7] res:m0) 
          a9(dur:24  pre:[a8] res:m9) a10(dur:66 pre:[a9] res:m1) 
          b1(dur:47  pre:[pa] res:m5) b2(dur:97  pre:[b1] res:m2) 
          b3(dur:92  pre:[b2] res:m8) b4(dur:22  pre:[b3] res:m9) 
          b5(dur:93  pre:[b4] res:m1) b6(dur:29  pre:[b5] res:m4) 
          b7(dur:56  pre:[b6] res:m7) b8(dur:80  pre:[b7] res:m3) 
          b9(dur:78  pre:[b8] res:m0) b10(dur:67 pre:[b9] res:m6) 
          c1(dur:45  pre:[pa] res:m1) c2(dur:46  pre:[c1] res:m7) 
          c3(dur:22  pre:[c2] res:m6) c4(dur:26  pre:[c3] res:m2) 
          c5(dur:38  pre:[c4] res:m9) c6(dur:69  pre:[c5] res:m0) 
          c7(dur:40  pre:[c6] res:m4) c8(dur:33  pre:[c7] res:m3) 
          c9(dur:75  pre:[c8] res:m8) c10(dur:96 pre:[c9] res:m5) 
          d1(dur:85  pre:[pa] res:m4) d2(dur:76  pre:[d1] res:m8) 
          d3(dur:68  pre:[d2] res:m5) d4(dur:88  pre:[d3] res:m9) 
          d5(dur:36  pre:[d4] res:m3) d6(dur:75  pre:[d5] res:m6) 
          d7(dur:56  pre:[d6] res:m2) d8(dur:35  pre:[d7] res:m1) 
          d9(dur:77  pre:[d8] res:m0) d10(dur:85 pre:[d9] res:m7) 
          e1(dur:60  pre:[pa] res:m8) e2(dur:20  pre:[e1] res:m9) 
          e3(dur:25  pre:[e2] res:m7) e4(dur:63  pre:[e3] res:m3) 
          e5(dur:81  pre:[e4] res:m4) e6(dur:52  pre:[e5] res:m0) 
          e7(dur:30  pre:[e6] res:m1) e8(dur:98  pre:[e7] res:m5) 
          e9(dur:54  pre:[e8] res:m6) e10(dur:86 pre:[e9] res:m2) 
          f1(dur:87  pre:[pa] res:m3) f2(dur:73  pre:[f1] res:m9) 
          f3(dur:51  pre:[f2] res:m5) f4(dur:95  pre:[f3] res:m2) 
          f5(dur:65  pre:[f4] res:m4) f6(dur:86  pre:[f5] res:m1) 
          f7(dur:22  pre:[f6] res:m6) f8(dur:58  pre:[f7] res:m8) 
          f9(dur:80  pre:[f8] res:m0) f10(dur:65 pre:[f9] res:m7) 
          g1(dur:81  pre:[pa] res:m5) g2(dur:53  pre:[g1] res:m2) 
          g3(dur:57  pre:[g2] res:m7) g4(dur:71  pre:[g3] res:m6) 
          g5(dur:81  pre:[g4] res:m9) g6(dur:43  pre:[g5] res:m0) 
          g7(dur:26  pre:[g6] res:m4) g8(dur:54  pre:[g7] res:m8) 
          g9(dur:58  pre:[g8] res:m3) g10(dur:69 pre:[g9] res:m1) 
          h1(dur:20  pre:[pa] res:m4) h2(dur:86  pre:[h1] res:m6) 
          h3(dur:21  pre:[h2] res:m5) h4(dur:79  pre:[h3] res:m8) 
          h5(dur:62  pre:[h4] res:m9) h6(dur:34  pre:[h5] res:m2) 
          h7(dur:27  pre:[h6] res:m0) h8(dur:81  pre:[h7] res:m1) 
          h9(dur:30  pre:[h8] res:m7) h10(dur:46 pre:[h9] res:m3) 
          i1(dur:68  pre:[pa] res:m9) i2(dur:66  pre:[i1] res:m6) 
          i3(dur:98  pre:[i2] res:m5) i4(dur:86  pre:[i3] res:m8) 
          i5(dur:66  pre:[i4] res:m7) i6(dur:56  pre:[i5] res:m0) 
          i7(dur:82  pre:[i6] res:m3) i8(dur:95  pre:[i7] res:m1) 
          i9(dur:47  pre:[i8] res:m4) i10(dur:78 pre:[i9] res:m2) 
          j1(dur:30  pre:[pa] res:m0) j2(dur:50  pre:[j1] res:m3) 
          j3(dur:34  pre:[j2] res:m7) j4(dur:58  pre:[j3] res:m2) 
          j5(dur:77  pre:[j4] res:m1) j6(dur:34  pre:[j5] res:m5) 
          j7(dur:84  pre:[j6] res:m8) j8(dur:40  pre:[j7] res:m4) 
          j9(dur:46  pre:[j8] res:m9) j10(dur:44 pre:[j9] res:m6) 
          pe(dur:0 pre:[a10 b10 c10 d10 e10 f10 g10 h10 i10 j10])])
