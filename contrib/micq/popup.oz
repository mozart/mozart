%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1998
%%%   Simon Lindblom, 1998
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
   Tk
%   System
export
   popup:Popup
   
define
   proc{Popup List T}
      SubInFocus={NewCell 0}
      InFocus={NewCell true}
      WinList={NewCell nil}
      
      proc{MakeMenu Parent List}
	 case List of nil then
	    skip
	 elseof A#B | Xs then
	    if {IsList B} then O N 
	       M = {New Tk.menu tkInit(parent:Parent tearoff:false)}
	    in
	       {Exchange WinList O N} N=M|O
	       {New Tk.menuentry.cascade tkInit(parent:Parent label:A menu:M) _}
	       {MakeMenu M B}

 	       {M tkBind(event:'<Enter>'
			 action:proc{$} O N in
				   {Exchange SubInFocus O N}
				   N=O+1
%				   {System.show em1(N)}
				end)}
 	       {M tkBind(event:'<Leave>'
			 action:proc{$} O N in
				   {Delay 200}
				   {Exchange SubInFocus O N}
				   N=O-1
%				   {System.show lm1(N)}
				 end)}
	       
	       {MakeMenu Parent Xs}
	    else
	       {New Tk.menuentry.command tkInit(parent:Parent label:A action:B) _}
	       {MakeMenu Parent Xs}
	    end
	 elseof separator | Xs then
	    {New Tk.menuentry.separator tkInit(parent:Parent) _}
	    {MakeMenu Parent Xs}
	 elseof ignore | Xs then
	    {MakeMenu Parent Xs}
	 else
	    raise popupError(List) end
	 end
      end
      MouseX = {Tk.returnInt winfo(pointerx T)}
      MouseY = {Tk.returnInt winfo(pointery T)}
      MainMenu = {New Tk.menu tkInit(parent:T tearoff:false)}
   in
%      {System.show '--'}
      {MakeMenu MainMenu List}
      {MainMenu tk(post MouseX MouseY)}
      {MainMenu tkBind(event:'<Enter>'
		       action: proc{$} O N in
				  {Exchange SubInFocus O N}
				  N=O+1
				  {Assign InFocus true}
%				  {System.show em(N)}
			       end)}
      {MainMenu tkBind(event:'<Leave>'
		       action: proc{$} O N in
				  {Delay 200}
				  {Exchange SubInFocus O N}
				  N=O-1
				  {Assign InFocus false}
%				  {System.show lm(N)}
			       end)}
      
      thread
	 {Delay 200}
 	 proc{CheckInFocus}
 	    {Delay 125}
	    if {Access SubInFocus}=<0 andthen {Access InFocus}==false then
	       {Tk.send wm(withdraw MainMenu)}
	       {ForAll {Access WinList} proc{$ X} {Tk.send wm(withdraw X)} end}
	    else
	       {CheckInFocus}
 	    end
 	 end
       in
	 {CheckInFocus}
       end
   end
in
   skip
end


/* Example

{Popup ["Subentry"#["Hi"#proc{$} {Show hi} end]
	separator
	"Entry"#proc{$} skip end] {New Tk.toplevel tkInit(title:"popup menu")}}

*/

