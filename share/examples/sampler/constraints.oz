%%%
%%% Constraints & propagators
%%%

declare L X Y Z in L = [X Y Z]

{Browse L}

{FD.dom 1#10 L}

2 * Y =: Z

X <: Y

Z <: 7

X \=: 1





%%%
%%% Propagation & enumeration
%%%

declare
proc {Problem Sol}
   X Y Z
in
   Sol = s(x:X y:Y z:Z) = {FD.dom 1#7}
   X + Y =: 3*Z
   X - Y =: Z
   {FD.distribute naive Sol}
end

{Browse {SearchOne Problem}}

{Browse {SearchAll Problem}}

{Explorer script(Problem)}








%%%
%%% Send More Money
%%%

declare
proc {Money Sol}
   S E N D M O R Y
in
   Sol = s(s:S e:E n:N d:D m:M o:O r:R y:Y)
   Sol ::: 0#9
   {FD.distinct Sol}
   S \=: 0
   M \=: 0
                1000*S + 100*E + 10*N + D
              + 1000*M + 100*O + 10*R + E
   =: 10000*M + 1000*O + 100*N + 10*E + Y
   {FD.distribute ff Sol}
end

{Explorer script(Money)}





%%%
%%% Magic Square
%%%
declare
proc {MagicSquare ?Sol}
   Square
   [N11 N12 N13 N21 N22 N23 N31 N32 N33]
       = Square
       = {FD.distinct}
   Sum = {FD.decl}
in
   Sol=s(square:Square sum:Sum)
   Square ::: 1#9
   N11 + N12 + N13 =: Sum
   N21 + N22 + N23 =: Sum
   N31 + N32 + N33 =: Sum
   N11 + N21 + N31 =: Sum
   N12 + N22 + N32 =: Sum
   N13 + N23 + N33 =: Sum
   N11 + N22 + N33 =: Sum
   N31 + N22 + N13 =: Sum
   3 * Sum =: 9 * 10 div 2
   {FD.distribute ff Square}
end

{ExploreOne MagicSquare}





%%%
%%% A smarter solution...
%%%

declare
proc {SmartSquare ?Sol}
   thread
      9 * (9 + 1) div 2 =: 3 * Sol.sum
   end
   {MagicSquare Sol}
end

{ExploreOne SmartSquare}














%%%
%%% Aligning for a photo
%%%
declare
Persons = [alice bert chris deb evan]
Prefs   = [alice#chris bert#evan chris#deb
           chris#evan deb#alice deb#evan
           evan#alice evan#bert]
proc {PhotoConstraints Sol}
   Pos   = {FD.record pos Persons
                      1#{Length Persons}}
         = {FD.distinct}
   Sat   = {Map Prefs
            fun {$ A#B}
               (Pos.A+1 =: Pos.B) +
               (Pos.A-1 =: Pos.B) =: 1
            end}
   Total = {FD.int 0#{Length Prefs}}
         = {FD.sum Sat '=:'}
in
   Sol = s(pos:Pos total:Total sat:Sat)
end


declare
proc {PhotoNaive Sol}
   {PhotoConstraints Sol}
   {FD.distribute naive Sol.pos}
end

declare
proc {MaxSat O N}
   O.total <: N.total
end

{Explorer script(PhotoNaive MaxSat)}













%%%
%%% Graphical output
%%%
\feed sampler/constraints/draw-photo.oz

{Explorer add(information DrawPhoto)}


















%%%
%%% Use a better enumeration
%%%
declare
proc {PhotoBetter Sol}
   {PhotoConstraints Sol}
   {FD.distribute generic(order:nbSusps) Sol.pos}
end

{ExploreBest PhotoBetter MaxSat}














%%%
%%% Removing symmetries
%%%

declare
proc {Photo Sol}
   thread
      Sol.pos.alice >: Sol.pos.bert
   end
   {PhotoBetter Sol}
end

{ExploreBest Photo MaxSat}











%%%
%%% Bridge
%%%

%% bridge specification
\feed sampler/constraints/bridge.oz

%% Scheduling compiler
\feed sampler/constraints/scheduling-compiler.oz


{Explorer script({Compile Bridge}
                 proc {$ O N}
                    O.pe >: N.pe
                 end)}

%% Use Gantt chart to visualize
\feed sampler/constraints/gantt.oz

{Explorer add(information DrawGantt)}


















%%%
%%% Production scheduling
%%%
\feed sampler/constraints/machines.oz

{ExploreBest {Compile Machines} proc {$ O N}
                                   O.pe >: N.pe
                                end}












%%%
%%% Use scheduling propagators
%%%

{ExploreBest {SmartCompile Bridge}
              proc {$ O N}
                 O.pe >: N.pe
              end}

{ExploreBest {SmartCompile Machines}
              proc {$ O N}
                 O.pe >: N.pe
              end}
















%%%
%%% Show the bridge schedule
%%%

\feed sampler/constraints/animate-bridge.oz
