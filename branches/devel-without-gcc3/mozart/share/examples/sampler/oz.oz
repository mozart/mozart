%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
%%%   Gert Smolka, 1998
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

%%%%%%%%%%%%%%%%%%%%%%%%%%% Functional Programming

declare
fun {MAP L F}
   case L
   of nil then nil
   [] H|T then {F H}|{MAP T F}
   end
end


declare L F O in 
{Inspect O}
O={MAP L F}

L = _|_|_

L = _|_|7|_

F = fun {$ X} X*X end

L = [1 2 _ _ ~5]

declare
fun {MAPC L F}
   case L
   of nil then nil
   [] H|T then thread {F H} end|{MAPC T F}
   end
end


%%%%%%%%%%%%%%%%%%%%%%%%%% Finite Domains


declare L X Y Z in L=[X Y Z]
{FD.dom 1#10 L}
{Inspect L}

2*Y=:Z

X<:Y

Z<:7

X\=:1

declare
proc {Q L}
   [X Y Z] = L
in
   {FD.dom 1#19 L}
   3*X+Y =: Z
   X >: 2
   {FD.distribute ff L}
end

{Inspect {SearchOne Q}}

{Inspect {SearchAll Q}}

{ExploreOne Q}



%%%%%%%%%%%%%%%%%%%%%%%%%% Constraint Programming


% see sampler-constraints.oz


%%%%%%%%%%%%%%%%%%%%%%%%%%% Logic Programming

declare
proc {LENGTH L N}
   dis L=nil N=0
   []  H R M in
      L=H|R N=s(M)
   then
      {LENGTH R M}  
   end
end



declare L N in 
{Inspect L}
{Inspect N}


{LENGTH L N}


N = s(s(_))


L = _|_|_|_


N = s(s(s(0)))


L = 1|2|3|nil


{ExploreOne proc {$ X} 
	       L N in 
	       X=L#N 
	       {LENGTH L N} 
	    end}



%%%%%%%%%%%%%%%%%%%%%%%%%% Object-oriented Programming

%%% a sequential object

declare
class Counter
   from BaseObject
   attr val:0
   meth inc
      val <- @val + 1
   end
   meth inspect
      {Inspect counter(@val)} 
   end
end
C = {New Counter inspect}

{C inc}

{C inspect}

%%% a concurrent object

declare
class DCounter from Counter
   prop locking
   meth set(X)
      lock
	 val <- X 
	 {self inspect}
      end
   end
   meth dec
      lock 
	 val <- @val-1
	 {self inspect}
      end
   end
   meth inc
      lock
	 Counter,inc
	 {self inspect}
      end
   end
end

declare X 
D = {New DCounter set(X)} 

{D dec}

{D inc}

X=56



%%%%%%%%%%%%%%%%%% Real Time Programming


{Delay 3000}
{Inspect 'fired after 3 seconds'}


declare
proc {DoWithDelay Xs T P}
   case Xs of X|Xr
   then {P X} {Delay T} {DoWithDelay Xr T P}
   else skip
   end
end

{DoWithDelay [this is a nice list] 1000 Inspect}


%%%%%%%%%%%%%%%%%% Animation


declare
class TimeCounter from DCounter Time.repeat end
D = {New TimeCounter inspect}

{D setRepAction(inc)}

{D go}

{D setRepAction(dec)}

{D setRepDelay(200)}

{D setRepDelay(1000)}

{D stop}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% create a window with a canvas and a frame
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

declare 
Window = {New Tk.toplevel tkInit(title:'Demo Window')}
Canvas = {New Tk.canvas tkInit(parent:Window
			       relief:sunken
			       borderwidth:1
			       background:white
			       width:400 height:400)}
{Tk.send pack(Canvas)}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% deiconify very simple things
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

{Window tkWM(iconify)}

{Window tkWM(deiconify)}


declare
proc {DrawCircle X Y R}
   {Canvas tk(create oval X-R Y-R X+R Y+R)}
end

{DrawCircle 100 100 20}

{DrawCircle 200 200 10}

declare
proc {SetMouseAction A}
   {Canvas tkBind(event: '<1>' action:A
		  args:[int(x) int(y)])}
end

{SetMouseAction proc {$ X Y} {DrawCircle X Y 40} end}

{Canvas tk(delete all)}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% A text object
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

declare
BigFont = {New Tk.font tkInit(weight:bold size:18)}
class Text from Tk.canvasTag
   meth init(X Y T)
      Tk.canvasTag,tkInit(parent:Canvas)
      {Canvas tk(crea text X Y anchor:w
		 text:T tag:self font:BigFont)}
   end
end
T={New Text init(10 100 "Oz is nice")}

{T tk(move 3 3)}

{T tkClose}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% An animated text object
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

declare
class AnimatedText from Text Time.repeat
   meth up   {self setRepAction(tk(move 0 ~2))} end
   meth down {self setRepAction(tk(move 0 2))}  end
   meth fast {self setRepDelay(100)}            end
   meth slow {self setRepDelay(1000)}           end
end
A={New AnimatedText init(20 20 "Oz is parallel")}

{A down}
{A go}
{A fast}
{A slow}
{A up}
{A down}		
{A stop}
{A go}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Buttons
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

declare
Frame = {New Tk.frame tkInit(parent:Window)}
{Tk.send pack(Frame)}
proc {NewButton Object Message}
   {Tk.send pack({New Tk.button
	     tkInit(parent:Frame font:BigFont text:Message
		    action: proc {$} {Object Message} end)}
	    side:left)}
end

{NewButton A stop}
{NewButton A go}
{NewButton A up}
{NewButton A down}
{NewButton A fast}
{NewButton A slow}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Images
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

declare
Images={TkTools.images
	['http://www.mozart-oz.org/home/'#
	 'doc/demo/applets/images/trucks/truck-right.ppm']}
TruckImage = Images.'truck-right'
class Truck from Tk.canvasTag Time.repeat
   meth init(Position)
      {self tkInit(parent:Canvas)}
      {Canvas tk(crea image 1 Position
		 image:  TruckImage
		 anchor: sw
		 tags:   self)}
      {self setRepAll(delay:100 number:150)}
      {self forward}
   end
   meth forward
      {self setRepAction(tk(move 2 0))}
      {self setRepFinal(backward)}
      {self go}
   end
   meth backward
      {self setRepAction(tk(move ~2 0))}
      {self setRepFinal(forward)}
      {self go}
   end
end
T={New Truck init(400)}


{T stop}

{T go}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Open Programming
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

declare
T={New AnimatedText init(250 20 "Oz is open")}
{T down} {T go}

declare
class InternetController
   from Open.socket Open.text
   meth getCommand
      {self write(vs:'command? ')}
      case {String.toAtom {Filter {self getS($)} Char.isAlpha}}
      of quit then {self close} {T stop} {T tkClose}
      elseof M then {T M} {self getCommand}
      end
   end
end
IC={New InternetController server(port:{Inspect portNumber($)})}
{IC getCommand}


********************************************





