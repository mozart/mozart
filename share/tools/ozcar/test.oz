%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   A = proc{$ X Y Z}
	  AA = proc{$}
		  {Show 'X'#X}
	       end
       in
	  Z = X + Y
	  {Show 'Z'#Z}
	  {Debug.breakpoint}
	  {AA}
	  {B Z}
       end
   B = proc{$ X}
	  {Debug.breakpoint}
	  Y = X + 1
       in
	  {Show 'Y'#Y}
       end
in
   {Browse {A 47 11 $}}
end
