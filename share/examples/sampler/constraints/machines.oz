%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Joerg Wuertz
%%%  Email: wuertz@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

declare

Machines =
machines(tasks:
            [
              pa # 0 # nil # noResource
              a1 #29#[pa] #m1  a2 #78#[a1]#m2  a3# 9#[a2]# m3
              a4 #36#[a3]#m4  a5 #49#[a4]#m5  a6#11#[a5]# m6
              a7 #62#[a6]#m7  a8 #56#[a7]#m8  a9#44#[a8]# m9
              a10#21#[a9]#m10
              b1 #43#[pa] #m1  b2#90#[b1]#m3 b3#75#[b2]#m5
              b4 #11#[b3]#m10 b5#69#[b4]#m4 b6#28#[b5]#m2
              b7 #46#[b6]#m7  b8#46#[b7]#m6 b9#72#[b8]#m8
              b10#30#[b9]#m9
              c1 #91#[pa]#m2   c2#85#[c1]#m1 c3#39#[c2]#m4
              c4 #74#[c3]#m3  c5#90#[c4]#m9 c6#10#[c5]#m6
              c7 #12#[c6]#m8  c8#89#[c7]#m7 c9#45#[c8]#m10
              c10#33#[c9]#m5
              d1 #81#[pa]#m2   d2#95#[d1]#m3 d3#71#[d2]#m1
              d4 #99#[d3]#m5  d5#9#[d4]#m7 d6#52#[d5]#m9
              d7 #85#[d6]#m8  d8#98#[d7]#m4 d9#22#[d8]#m10
              d10#43#[d9]#m6
              e1 #14#[pa] # m3  e2 # 6#[e1]# m1  e3 #22#[e2]# m2
              e4 #61#[e3]# m6  e5 #26#[e4]# m4  e6 #69#[e5]# m5
              e7 #21#[e6]# m9  e8 #49#[e7]# m8  e9 #72#[e8]# m10
              e10#53#[e9]# m7
              pe #0 #[a10 b10 c10 d10 e10]#noResource]
         constraints:
            proc {$ Start Dur}
               skip
            end)
