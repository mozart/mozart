%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   A = proc{$ X Y Z}
	  AA = proc{$}
		  {Debug.breakpoint}
		  {Show 'X'#X}
	       end
       in
	  Z = X + Y
	  {Show 'Z'#Z}
	  thread
	     {AA}
	  end
	  {B Z}
       end
   B = proc{$ X}
	  {Debug.breakpoint}
	  Y = X + 1
       in
	  {Show 'Y'#Y}
       end
in
   local
      D = 4711
   in
      {Browse {A 47 11 $}}
   end
end
