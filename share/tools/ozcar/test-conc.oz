%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare

\insert test-conc2

proc {Scan T G1 G2 R}
   {WaitOr T R}
   case {IsDet R} then skip else
      case T
      of green then G1=G2
      [] red then R=unit
      [] A#B then G in
	 thread {Debug.breakpoint} {Scan A G1 G R} end
	 thread {Debug.breakpoint} {Scan B G G2 R} end
      end
   end
end

{Debug.breakpoint}
{Browse {AllGreen (green#(_#green))#(red#_)}}

