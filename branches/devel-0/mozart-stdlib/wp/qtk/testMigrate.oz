declare

% [GS]={Module.link ["GetSignature.ozf"]}
% class T feat a b C meth init skip end end
% {Browse {GS.getClassInfo T}}

[QTk QTkMigratable]={Module.link ["QTkBare.ozf" "QTkMigratable.ozf"]}

%{Browse assert#QTkMigratable.assertStream}

{Show 100}
MB={QTk.newBuilder}
{Show 101}

{MB.defaultLook.set button(bg:red)}
{Show 102}

F={QTk.newFont font(family:'Arial' size:16)}
{Show 103}
R
B
P
L
G

% M={MB.buildMigratable td(placeholder(handle:P))}
% Win={QTk.build td(title:"1" receiver(handle:R))}
% {R set({M get(ref:$)})}
% {P set(label(text:"Hello" handle:L))}
% {Win show}
C

% local
%    P
%    M={QTk.buildMigratable td(placeholder(handle:P))}
%    L
%    R
%    Win={QTk.build td(receiver(handle:R))}
%    {R set({M get(ref:$)})}
%    {P set(label(text:1 handle:L))}
% in
%    {Browse L}
%    {Wait L}
%    {Show ok}
% end



Look={QTk.newLook}
{Look.set label(bg:red)}
{Show 1}
M={QTk.buildMigratable td(button(text:"Migratable Button" font:F
				 handle:B
				 action:proc{$} {Show hi} end)
			  label(text:"Hello" look:Look)
			  label(text:"Hello2" look:Look)
			  td(canvas(handle:C bg:green))
			  placeholder(handle:P)
			  grid(handle:G)
			  button(text:"b2")
			 )}
{Show M}
{Show 2}

Win={QTk.build td(title:"1" receiver(handle:R))}
{Show 3}
{Win show}
{Show 4}
{Show ref#{M get(ref:$)}}
{R set({M get(ref:$)})}

{Delay 1000}
{Show '++++++++++++++'}
%{R set({M get(ref:$)})}
%{Delay 2000}
{Show 5}
%local Tag={C newTag($)} in {C create(line 20 30 40 50 tags:Tag)} end
proc{Turn Tag D}
    thread
       proc{Loop}
	  {Tag move(10 0)}
	  {Delay D}
	  {Tag move(0 10)}
	  {Delay D}
	  {Tag move(~10 0)}
	  {Delay D}
	  {Tag move(0 ~10)}
	  {Delay D}
	  {Loop}
       end
    in
       {Loop}
    end
end
{C create(rect 20 20 100 100)}

Tag={C newTag($)}
{Show 6}
{C create(line 10 10 100 100 tags:Tag)}
{Show 7}
{Turn Tag 200}
{Show 8}
Ov
{C create(oval 10 10 100 100 handle:Ov)}
{Browse chandle#Ov}
{Turn Ov 500}

{Show 5}
P1 P2 GL
{B bind(event:"<Enter>" action:proc{$} {Show enter} end)}
{P set(label(text:"Placeholder 1" handle:P1))}
{P set(label(text:"Placeholder 2" handle:P2))}
{G configure(label(text:"1" handle:GL) td(label(text:"2") label(text:"2a")) label(text:"3") label(text:"4") label(text:"5") label(text:"6"))}
{G configure(label(text:"1") label(text:"2") label(text:"3") columnspan:2)}
{Show a5}
{Show GL#G#P#P1#P2}
{G configure(GL row:3 column:3)}
{Show b5}
{P set(P1)}
{Show 6}
{Delay 2000}
{Show 7}
{B set(bg:blue)}
{B set(underline:8)}
{Show 8}
R2
{{QTk.build td(title:"2" receiver(handle:R2))} show}
%{R set(empty)}
{R2 set({M get(ref:$)})}
%{Delay 1000}
{B set(bg:blue)}
{P1 set(bg:red)}
{Show 10}
{Delay 2000}
{P set(P2)}
{C set(bg:yellow)}
{F set(size:18)}
F2={QTk.newFont font(family:"Times" size:5 underline:true)}
{B set(font:F2)}

{F2 set(size:15)}

% P
% Win={MB.build td(placeholder(handle:P))}
% %{Win show}
% B

% fun{Pop}
%    {B get(text:$)}
% end

% proc{Push S}
%    {B set(text:S)}
% end

% MObj
% {Show 1}
% {Win buildMigratable(button(text:"Migratable Button" handle:B action:proc{$} {Show hi} end)
% 		     Pop
% 		     Push
% 		     MObj)}
% {Show 2}
% {Browse {MObj dump($)}}
% R
% {Show 3}
% Win2={QTk.build td(receiver(handle:R))}
% {Show 4}
% {B set(fg:yellow)}
% {B bind(event:"<Enter>" action:proc{$} {Show enter} end)}
% {Show 5}
% {R set({MObj getRef($)})}
% {Win2 show}
% {Show 6}
% {Show B}
% {B set(bg:blue)}
% {Show 7}
% {B bind(event:"<Leave>" action:proc{$} {Show leave} end)}



% C
% Remote={QTk.buildMigratable td(label(text:"Hello")
% 			       button(text:"Hello" action:proc{$} {Show 'hello'} end)
% 			       canvas(handle:C))}

% Win={QTk.build td(receiver(handle:R))}
% {Win show}
% {R set({Remote get(ref:$)})}
% proc{Redraw Canvas}
%    {Canvas create(circle 10 10 100 100)}
% end

% %% default push & pop for labels in Remote => nothing to specify
% %% can be set back by {Remote transfertState(label unit unit)}

% %% override default push & pop for buttons in Remote => only transfert the button text
% %% other parameters are ignored
% {Remote transfertState(button
% 		       fun{$ H} {H get(text:$)} end
% 		       proc{$ H S} {H set(text:S)} end)}

% %% alternatively the pop and push procedures can have 3 parameters instead of 2
% %% in such case, the first parameter is the previous pop and push procedures.
% %% note that this parameters is always a 2 parameters procedure
% %% override specific push & pop for canvas in Remote
% {C transfertState(fun{$ Old C} {Old C} end
% 		  proc{$ C _}
% 		     {Redraw C}
% 		  end)}

% %% force the Remote interface to store its state
% {Remote updateState}
% %% force the canvas to store its state
% {C updateState} % alternatively one can also ask the result of the POP : {Show {C updateState($)}}

% R2
% Win2={QTk.build td(receiver(handle:R2))}
% {Win2 show}
% {R set(unit)} %% disconnects the UI from the first window
% thread
%    %% this will block as the UI is not connected to a window
%    {C create(rectangle 10 10 100 100)}
%    {Show blocked}
% end
% {Delay 1000}
% {Show here}
% {Delay 1000}
% {R2 set({Remote get(ref:$)})}
