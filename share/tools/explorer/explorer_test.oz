declare

proc {Alpha Sol}
   !Sol = alpha(a:A b:B c:C d:D e:E f:F g:G h:H i:I j:J k:K l:L m:M
		n:N o:O p:P q:Q r:R s:S t:T u:U v:V w:W x:X y:Y z:Z)
   = {FD.dom 1#26}
in
   {FD.distinct Sol}
   
   B+A+L+L+E+T       =: 45
   C+E+L+L+O         =: 43
   C+O+N+C+E+R+T     =: 74
   F+L+U+T+E         =: 30
   F+U+G+U+E         =: 50
   G+L+E+E           =: 66
   J+A+Z+Z           =: 58
   L+Y+R+E           =: 47
   O+B+O+E           =: 53
   O+P+E+R+A         =: 65
   P+O+L+K+A         =: 59
   Q+U+A+R+T+E+T     =: 50
   S+A+X+O+P+H+O+N+E =: 134
   S+C+A+L+E         =: 51
   S+O+L+O           =: 37
   S+O+N+G           =: 61
   S+O+P+R+A+N+O     =: 82
   T+H+E+M+E         =: 72
   V+I+O+L+I+N       =: 100
   W+A+L+T+Z         =: 34
   
   {FD.distribute ff Sol}
end 


proc {Queens Board}
   Board = {FD.list 8 1#8}
   {List.forAllTail Board 
    proc{$ Q|Qs}
       {List.forAllInd Qs
	proc{$ I R}
	   Q\=:R  Q\=:R+I  Q\=:R-I
	end}
    end}
   {FD.distribute ff Board}
end


local
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
   
in

   proc {Photo Sol}
      {PhotoConstraints Sol}
      {FD.distribute naive Sol.pos}
   end

   proc {MaxSat O N}
      O.total <: N.total
   end

end


/*

% Test Move, Search, Nodes, and Hide
{ExploreOne Queens}

{ExploreBest Photo MaxSat}


declare
fun {P1 U V}
   proc {$ X}
      choice
	 if U=1 then true
	 [] U=2 then choice X=3 [] X=4 end
	 [] U=3 then V=1
	 else false
	 end
      [] X=2
      end
   end
end

fun {P2 U V}
   proc {$ X}
      if U=1 then true
      [] U=2 then choice X=3 [] X=4 end
      [] U=3 then V=1
      else false
      end
   end
end

declare
fun {P3 U V}
   proc {$ X}
      choice X=1
      [] if U=1 then choice X=5 [] X=4 end end
      [] choice X=5 [] X=5 end
      [] if V=1 then choice X=5 [] X=8 end end
      end
   end
end

declare U V in {ExploreOne {P1 U V}}
U=1
declare U V in {ExploreOne {P1 U V}}
U=2
declare U V in {ExploreOne {P1 U V}}
U=3
V=1
declare U V in {ExploreOne {P1 U V}}
U=4

declare U V in {ExploreOne {P2 U V}}
U=1
declare U V in {ExploreOne {P2 U V}}
U=2
declare U V in {ExploreOne {P2 U V}}
U=3
V=1
declare U V in {ExploreOne {P2 U V}}
U=4

declare U V in {ExploreBest {P3 U V} proc {$ O N} O <: N end}
U=1
V=1
declare U V in {ExploreBest {P3 U V} proc {$ O N} O <: N end}
U=1
V=1


*/
