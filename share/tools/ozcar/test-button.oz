%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare
T={New Tk.toplevel tkInit}
B={New Tk.button tkInit(parent:T action:proc {$}
					   {Debug.breakpoint}
					   {Browse a}
					end)}
{Tk.send pack(B)}
