\insert ri.oz

%\define DEBUG

declare
PrintVS = System.printInfo
ShowVS = System.showInfo

Problem =
problem(
  resources: resource(
		man:       r(ta: 14 npp: [1 1 1 1 1 1 1 1 1 1 1 1 1])
		material1: r(ta: 17 npp: [0 4 5 0 0 0 0 4 5 0 0 0 0])
		material2: r(ta: 20 npp: [4 0 3 0 0 0 3 0 4 0 0 0 0])
		machine1:  r(ta: 34 npp: [7 0 0 6 0 0 7 0 0 6 0 0 0])
		machine2:  r(ta: 26 npp: [0 0 0 4 5 0 0 0 0 5 4 0 0])
		machine3:  r(ta: 16 npp: [0 0 0 0 4 3 0 0 0 0 4 2 1])
		machine4:  r(ta: 16 npp: [0 3 0 0 0 5 0 3 0 0 0 3 3]))
  profit: [5 7 5 11 8 10 6 8 3 12 9 8 4])

/*
p(resources: rs(r1: r(ta: 6  npp: [1 1])
		r2: r(ta: 45 npp: [9 5])
	       )
  profit: [8 5])
*/

/*
sol(
   maxprofit:121 
   products:[0 0 3 3 0 0 0 0 0 2 1 5 0] 
   profit:[5 7 5 11 8 10 6 8 3 12 9 8 4])
5270/44/5227
*/

fun {KnapsackFD Problem}
   NumProducts = {Length Problem.profit}
   Resources   = Problem.resources
in
   proc {$ Sol}
      sol(maxprofit: MaxProfit = {FD.decl}
	  products:  Products  = {FD.list NumProducts 0#FD.sup}
	  profit:    Problem.profit) = Sol
   in
      MaxProfit = {FD.sumC Problem.profit Products '=:'}

      {ForAll {Arity Resources}
       proc {$ ResourceName}
	  Resource = Resources.ResourceName
       in
	  {FD.sumC Resource.npp Products '=<:' Resource.ta}
       end}

      {FD.distribute naive Products}
   end
end

% returns `unit#unit' if no non-integral variable is found
fun {SelectVar VsPair}
   case VsPair
   of nil#nil then unit#unit 
   [] (VH|VT)#(RVH|RVT) then
      % check for integrality
      case RVH == {Round RVH}
      then {SelectVar VT#RVT}
      else VH#RVH
      end
   else
      {Browse VsPair} unit
   end
end   

fun {DuplicateRIs Vs}
   {Map Vs fun {$ V} {RI.var.bounds
		      {RI.getLowerBound V}
		      {RI.getUpperBound V}}
	   end}	 
end

proc {DistributeKnapSackLP Vs ObjFn Constraints MaxProfit}
   choice
      DupVs = {DuplicateRIs Vs}
      DupMaxProfit V DupV
   in
      DupMaxProfit = {RI.var.bounds
		      {RI.getLowerBound MaxProfit}
		      {RI.getUpperBound MaxProfit}}
      
      {LP.solve DupVs ObjFn Constraints DupMaxProfit optimal}

      V#DupV = {SelectVar Vs#DupVs}

      case {IsDet V} then
\ifdef DEBUG
	 {ShowVS '@@@@ integral solution found '#DupMaxProfit}	 
\endif
	 DupMaxProfit = MaxProfit
	 DupVs        = Vs
      else
	 dis {RI.lessEq {Ceil DupV} V} 
	 then
\ifdef DEBUG
	    {ShowVS '@@@@ branch1'}
	    {Show V#DupV}
\endif
	    {DistributeKnapSackLP Vs ObjFn Constraints MaxProfit}
	 [] {RI.lessEq V {Floor DupV}} 
	 then
\ifdef DEBUG
	    {ShowVS '@@@@ branch2'}
	    {Show V#DupV}
\endif
	    {DistributeKnapSackLP Vs ObjFn Constraints MaxProfit}
	 end
      end     
   end
end

/*
sol(
   maxprofit:121.0 
   products:[0.0 0.0 3.0 3.0 0.0 0.0 0.0 0.0 0.0 
             2.0 1.0 5.0 0.0] 
   profit:[5 7 5 11 8 10 6 8 3 12 9 8 4])

490/12/479
*/
fun {KnapsackLP Problem}
   NumProducts = {Length Problem.profit}
   Resources   = Problem.resources
in
   proc {$ Sol}
      sol(maxprofit: MaxProfit = {RI.var.decl}
	  products:  Products  = {MakeList NumProducts}
	  profit:    Problem.profit) = Sol

      ObjFn Constraints
   in
      {ForAll Products proc {$ V} {RI.var.bounds 0.0 RI.sup V} end}

      ObjFn = objfn(row: {Map Problem.profit fun {$ I} {IntToFloat I} end}
		    opt: max)

      Constraints = 
      {Map {Arity Resources}
        fun {$ ResourceName}
	  Resource = Resources.ResourceName
       in
	  constr(row: {Map Resource.npp fun {$ I} {IntToFloat I} end}
		 type: '=<'
		 rhs: {IntToFloat Resource.ta})
       end}

      {DistributeKnapSackLP Products ObjFn Constraints MaxProfit}
   end
end

/*
sol(
   maxprofit:121 
   products:[0 0 3 3 0 0 0 0 0 2 1 5 0] 
   profit:[5 7 5 11 8 10 6 8 3 12 9 8 4])

52/8/45
*/
fun {KnapsackFDLP Problem}
   NumProducts = {Length Problem.profit}
   Resources   = Problem.resources
in
   proc {$ Sol}
      sol(maxprofit: FDMaxProfit = {FD.decl}
	  products:  FDProducts  = {FD.list NumProducts 0#FD.sup}
	  profit:    Problem.profit) = Sol

      ObjFn Constraints
      MaxProfit = {RI.var.decl}
      Products  = {MakeList NumProducts}
   in
      %
      % finite domain constraints part
      %
      FDMaxProfit = {FD.sumC Problem.profit FDProducts '=:'}

      {ForAll {Arity Resources}
       proc {$ ResourceName}
	  Resource = Resources.ResourceName
       in
	  {FD.sumC Resource.npp FDProducts '=<:' Resource.ta}
       end}

      %%
      %% linear programming part
      %%
      {ForAll Products proc {$ V} {RI.var.bounds 0.0 RI.sup V} end}

      ObjFn = objfn(row: {Map Problem.profit fun {$ I} {IntToFloat I} end}
		    opt: max)

      Constraints = 
      {Map {Arity Resources}
        fun {$ ResourceName}
	  Resource = Resources.ResourceName
       in
	  constr(row: {Map Resource.npp fun {$ I} {IntToFloat I} end}
		 type: '=<'
		 rhs: {IntToFloat Resource.ta})
       end}

      %%
      %% putting both together
      %%
      {RI.intBounds MaxProfit FDMaxProfit}
      {Map Products proc {$ R D} {RI.intBounds R D} end FDProducts}

      
      {DistributeKnapSackLP Products ObjFn Constraints MaxProfit}
   end
end

/*

{ExploreBest {KnapsackFD Problem} proc {$ O N} O.maxprofit <: N.maxprofit end}

{ExploreBest {KnapsackLP Problem} proc {$ O N} {RI.lessEq O.maxprofit+1.0 N.maxprofit} end}

{ExploreBest {KnapsackFDLP Problem} proc {$ O N} O.maxprofit <: N.maxprofit end}


{Browse {LP.config get}}

{LP.config put conf(mode: verbose)} 
{LP.config put conf(mode: quiet)}
{LP.config put conf(solver: cplex_primopt)}
{LP.config put conf(solver: cplex_dualopt)}
{LP.config put conf(solver: lpsolve)}
*/

