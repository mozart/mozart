declare

proc {Alpha Sol}
   alpha(a:A b:B c:C d:D e:E f:F g:G h:H i:I j:J k:K l:L m:M
	 n:N o:O p:P q:Q r:R s:S t:T u:U v:V w:W x:X y:Y z:Z)
   = Sol = {FD.dom 1#26}
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
   Board = {FD.list 6 1#6}
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

proc {Picture X}
   choice X=1
   [] fail
   [] choice choice fail [] fail end [] X=2 end
   end
end
	 

/*

% Test Move, Search, Nodes, and Hide
{ExploreOne Queens}

{ExploreAll Picture}

{ExploreBest Photo MaxSat}

%%
%% Test blocking
%%

declare
fun {P1 U V}
   proc {$ X}
      choice
	 if U=1 then skip
	 [] U=2 then choice X=3 [] X=4 end
	 [] U=3 then V=1
	 else fail
	 end
      [] X=2
      end
   end
end

fun {P2 U V}
   proc {$ X}
      if U=1 then skip
      [] U=2 then choice X=3 [] X=4 end
      [] U=3 then V=1
      else fail
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


%%
%% Test recomputation
%%
{Explorer option(search search:full information:full)}

declare U
{ExploreOne proc {$ X}
	       choice
		  case {IsFree U} then
		     choice X=11 [] X=12 end
		  else fail
		  end
	       []
		  case {IsFree U} then
		     choice X=11 [] X=12 end
		  else fail
		  end
	       []
		  case {IsFree U} then
		     choice X=11 [] X=12 end
		  else fail
		  end
	       end
	    end}
U=1
% access info


%%
%% Test options
%%

{Explorer option(search search:4 information:8 failed:true)}

{Explorer option(search search:0 information:1)}
{Explorer option(search search:none information:full)}
{Explorer option(search search:full information:none)}

{Explorer option(drawing hide:false scale:true update:40)}

{Explorer option(postscript color:grayscale orientation:landscape
		 size:6#i#x#7#i)}
{Explorer option(postscript color:full orientation:portrait
		 size:6#p#x#7#c)}
{Explorer option(postscript color:bw
		 size:6#m#x#7#m)}

%% wrong options
{Explorer option(gargel)}

{Explorer option(search search:a)}
{Explorer option(search information:a)}
{Explorer option(search wuff:c)}

{Explorer option(drawing hide:a)}
{Explorer option(drawing scale:a)}
{Explorer option(drawing update:a)}
{Explorer option(drawing quff:a)}

{Explorer option(postscript color:a)}
{Explorer option(postscript orientation:a)}
{Explorer option(postscript size:a)}
{Explorer option(postscript chack:a)}


  
%%
%% Test actions
%%

{Explorer add(information separator)}
{Explorer add(information proc {$ N X}
			     {Show N#X}
			  end)}
{Explorer add(information proc {$ N X}
			     {Show N#X}
			  end
	      label:test_root type:root)}
{Explorer add(information proc {$ N X}
			     {Show N#{Space.merge X}}
			  end
	      label:test_space type:space)}
{Explorer add(information proc {$ N X}
			     {Show N#{X}#{X}}
			  end
	      label:test_procedure type:procedure)}

{Explorer add(compare separator)}
{Explorer add(compare proc {$ N1 X1 N2 X2}
			 {Show N1#X1#N2#X2}
		      end)}
{Explorer add(compare proc {$ N1 X1 N2 X2}
			 {Show N1#X1#N2#X2}
		      end 
	      label:test_root type:root)}
{Explorer add(compare proc {$ N1 X1 N2 X2}
			 {Show N1#{Space.merge X1}#N2#{Space.merge X2}}
		      end 
	      label:test_space type:space)}
{Explorer add(compare proc {$ N1 X1 N2 X2}
			 {Show N1#{X1}#{X1}#N2#{X2}#{X2}}
		      end 
	      label:test_procedure type:procedure)}

declare
proc {P1 _ _} skip end
proc {P2 _ _} skip end
{Explorer add(information separator)}
{Explorer add(information P1)}
{Explorer add(information separator)}
{Explorer add(information P2)}
{Explorer add(information separator)}

{Explorer delete(information P2)}
{Explorer delete(information P1)}

{Explorer add(information separator)}
{Explorer add(information P1)}
{Explorer add(information separator)}
{Explorer add(information P2)}
{Explorer add(information separator)}

{Explorer delete(information all)}
{Explorer delete(compare all)}
{Explorer delete(statistics all)}

%%
%% Check reset, clear, or close
%%
declare
create ShowCloseInfo from BaseObject
   meth close {Show close_info} end
end
create ShowCloseCmp from BaseObject
   meth close {Show close_compare} end
end
create ShowCloseStat from BaseObject
   meth close {Show close_stat} end
end

{Explorer add(information fun {$ _ _}
			     proc {$} {Show get_rid_info} end
			  end)}
{Explorer add(information fun {$ _ _}
			     ShowCloseInfo # close
			  end)}
{Explorer add(compare fun {$ _ _ _ _}
			 proc {$} {Show get_rid_compare} end
		      end)}
{Explorer add(compare fun {$ _ _ _ _}
			 ShowCloseCmp # close
		      end)}
{Explorer add(statistics fun {$ _ _}
			    proc {$} {Show get_rid_statistics} end
			  end)}
{Explorer add(information fun {$ _ _}
			     ShowCloseStat # close
			  end)}


*/




