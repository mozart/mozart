%%%
%%% Authors:
%%%   Christian Schulte <schulte@dfki.de>
%%%
%%% Contributor:
%%%   Jörg Würtz
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor

import
   FD
   Schedule

export
   compile: Compile

define

   local
      fun {SkipDet Is}
	 case Is of nil then nil
	 [] I|Ir then
	    if {FD.reflect.size I}>1 then Is else {SkipDet Ir} end
	 end
      end
   in
      proc {AssignMin Is}
	 choice skip end
	 case {SkipDet Is}
	 of nil then skip
	 [] I|Ir then
	    I={FD.reflect.min I} {AssignMin Ir}
	 end
      end
   end
   
   proc {Map3 Xs F ?Y1s ?Y2s ?Y3s}
      case Xs of nil then Y1s=nil Y2s=nil Y3s=nil
      [] X|Xr then Y1|Y1r=Y1s Y2|Y2r=Y2s Y3|Y3r=Y3s in
	 {F X ?Y1 ?Y2 ?Y3} {Map3 Xr F Y1r Y2r Y3r}
      end
   end
   
   fun {SwapDir Dir}
      case Dir of x then y [] y then x end
   end
   
   local
      proc {Do Bs N}
	 case Bs of nil then skip
	 [] B|Br then NN=1-N in
	    choice skip end
	    if {Not {IsDet B}} then
	       choice B=N [] B=NN end
	    end
	    {Do Br NN}
	 end
      end
   in
      proc {Alternate Bs}
	 {Do Bs 0}
      end
   end
   
   local
      fun {Insert Ts N P}
	 case Ts of nil then [N#P]
	 [] T|Tr then
	    if N<T.1 then N#P|Ts else T|{Insert Tr N P} end
	 end
      end
   in
      class DistributionManager
	 prop final
	 attr
	    Tasks: nil
	    Done
	    
	 meth init
	    thread DistributionManager,Drive end
	 end
	 
	 meth add(N P)
	    NewTs OldTs
	 in
	    OldTs = (Tasks <- NewTs)
	    NewTs={Insert OldTs N P}
	 end
	 
	 meth wait
	    {Wait @Done}
	 end
	 
	 meth Drive
	    choice skip end
	    case @Tasks of nil then
	       @Done = unit
	    [] T|Tr then
	       Tasks <- Tr
	       {T.2}
	       DistributionManager,Drive
	    end
	 end
      end
   end
   
   fun {Compile Spec}
      
      %% Specification is as follows:
      %%  Spec.x, Spec.y: size of the target plate
      %%  Spec.squares.D=N: N squares of size D

      %% Number of all squares
      N  = {Record.foldL Spec.squares Number.'+' 0}
      %% Dimension of X and Y
      DX = Spec.x
      DY = Spec.y

   in
   
      proc {$ Root}
	 Sqs
	 Cuts

	 DM = {New DistributionManager init}
      
	 proc {MakeCuts N SQS Dir RectDir RectRid ?Info}
	    thread
	       Rid  = {SwapDir Dir}
	       Card = {FS.card SQS}
	    in
	       Card >: 0
	       cond Card<:3 then
		  %% Just go an place the squares
		  {DM add(N
			  proc {$}
			     Is = {FS.reflect.upperBoundList SQS}
			  in
			     case Card
			     of 1 then [I]=Is Sq=Sqs.I in
				Info   = nil
				Sq.Dir = RectDir.1
				Sq.Rid = RectRid.1
				Sq.Dir + Sq.d =<: RectDir.2
				Sq.Rid + Sq.d =<: RectRid.2
			     [] 2 then [I1 I2]=Is Sq1=Sqs.I1 Sq2=Sqs.I2 in
				case Dir
				of x then
				   Sq1.x = RectDir.1
				   Sq1.y = RectRid.1
				   Sq2.x =: Sq1.x + Sq1.d
				   Sq2.y = RectRid.1
				   Sq2.x + Sq2.d =<: RectDir.2
				   Sq2.y         =<: RectRid.2
				   Info  = info(cut:Sq2.x nil nil) 
				[] y then
				   Sq1.y = RectDir.1
				   Sq1.x = RectRid.1
				   Sq2.y =: Sq1.y + Sq1.d
				   Sq2.x = RectRid.1
				   Sq2.y + Sq2.d =<: RectDir.2
				   Sq2.x         =<: RectRid.2
				   Info  = info(cut:Sq2.y nil nil) 
				end
			     end
			  end)}
	       else
		  %% Partition the squares into two disjoint sets
		  SQS1 = {FS.var.decl}
		  SQS2 = {FS.var.decl}
		  %% The other direction (direction of the next cut)
		  DimD = {FD.decl}
		  DimL = {FD.decl}
		  DimR = {FD.decl}
		  IsLs IsRs DDs
		  info(cut:Cut InfoL InfoR) = Info
	       in
		  Cut = {FD.decl}
		  DimD =: RectRid.2 - RectRid.1
		  %% SQS1 and SQS2 are partition of the squares
		  {FS.union SQS1 SQS2 SQS}
		  %% The cut must be inside the current dimensions, and
		  %% of course the squares we consider here have at least size 2!
		  RectDir.1+2=<:Cut Cut+2=<:RectDir.2
		  %% Find Left and Right, just cardinality matters
		  {Map3 {FS.reflect.upperBoundList SQS}
		   proc {$ I ?IsL ?IsR ?DD}
		      Sq=Sqs.I SqXY=Sq.Dir SqD=Sq.d
		   in
		      IsL = (Cut >=: SqXY + SqD)
		      IsR = (Cut =<: SqXY)
		      {FD.nega IsL IsR}
		      IsL = {FS.reified.isIn I SQS1}
		      IsR = {FS.reified.isIn I SQS2}
		      DD  = SqD*SqD
		   end ?IsLs ?IsRs ?DDs}
		  %% However, the capacities must fit (redundant)
		  DimL =: DimD * Cut - DimD * RectDir.1 
		  {FD.sumC DDs IsLs '=<:' DimL} 
		  DimR =: DimD * RectDir.2 - DimD * Cut
		  {FD.sumC DDs IsRs '=<:' DimR} 
		  %% Distribute squares to be either right or left
		  {DM add(N proc {$}
			       {Alternate IsLs}
			    end)}
		  %% These are the next rectangles to be cut
		  {MakeCuts N+1 SQS1 Rid RectRid RectDir.1#Cut InfoL}
		  {MakeCuts N+1 SQS2 Rid RectRid Cut#RectDir.2 InfoR}
	       end
	    end
	 end

	 fun {GetCuts Cut Cs}
	    if Cut==nil then
	       Cs
	    else
	       Cut.cut|{GetCuts Cut.1 {GetCuts Cut.2 Cs}}
	    end
	 end

	 proc {Fit Dir Cap}
	    Tasks = {Tuple.make tasks N}
	    {For 1 N 1
	     proc {$ I}
		Tasks.I = {VirtualString.toAtom I}
	     end}
	    As    = {Record.foldR Tasks fun {$ A Ar} A|Ar end nil}
	    Dur   = {Record.make dur   As}
	    Start = {Record.make start As}
	    Use   = Dur
	 in
	    {For 1 N 1
	     proc {$ I}
		A = Tasks.I
	     in
		Dur.A=Sqs.I.d Start.A=Sqs.I.Dir
	     end}
	    {Schedule.cumulative [Tasks] Start Dur Use [Cap]}
	 end
      
      in
      
	 Root = root(squares:Sqs cuts:Cuts x:DX y:DY)

	 Sqs  = {MakeTuple '#' N}
      

	 %% Set up problem variables
	 {Record.foldRInd Spec.squares
	  fun {$ D M I}
	     {For I I+M-1 1 proc {$ J}
			       Sqs.J=square(x: {FD.int 0#{Max 0 DX-D}}
					    y: {FD.int 0#{Max 0 DY-D}}
					    d: D)
			    end}
	     I+M
	  end 1 _}
      

	 %% The squares must fit the target
	 {Record.foldL Sqs fun {$ N Sq}
			      N+Sq.d*Sq.d
			   end 0} =<: DX*DY
      

	 %% Fix some freedom for first square (wolg)
	 Sqs.1.x=0 Sqs.1.y=0


	 %% Remove permutations of equally-sized squares by ordering them
	 {For 1 N-1 1 proc {$ I}
			 Sq1=Sqs.I Sq2=Sqs.(I+1)
		      in
			 if Sq1.d==Sq2.d then
			    %% This is respected by the no overlap
			    Sq1.x * DY + Sq1.y <: Sq2.x * DY + Sq2.y
			 end
		      end}

	 %% No Overlaps allowed
	 local
	    Xsqs = {Record.map Sqs fun {$ Sq} Sq.x end}
	    Ysqs = {Record.map Sqs fun {$ Sq} Sq.y end}
	    Dsqs = {Record.map Sqs fun {$ Sq} Sq.d end}
	 in
	    {FD.distinct2 Xsqs Dsqs Ysqs Dsqs}
	 end

	 %% In any direction (be it x or y) the squares must
	 %% fit into the height/width of the rectangle
	 {Fit x DY}
	 {Fit y DX}
      
	 %%  1. Find a position for the cut, starting from the middle
	 %%  2. Distribute all squares either to the left or right side
	 %%     of the cut
	 %%  3. Only consider squares with size graeter than 1
	 Cuts={MakeCuts 0 {FS.value.make [1#N]} x 0#DX 0#DY} 

	 {DM wait}
	 %% The rest is deterministic
	 {AssignMin {GetCuts Cuts nil}}

	 {AssignMin {Record.foldL Sqs fun {$ XYs Sq}
					 Sq.x|Sq.y|XYs
				      end nil}}

      end

   end

end

   /*
declare
%Spec = spec(x:5 y:5 squares:s(3:1 2:2))
%Spec = spec(x:7 y:9 squares:s(2:4 5:1 1:1 3:1))
%Spec = spec(x:8 y:8 squares:s(4:4 1:1))
%Spec = spec(x:7 y:7 squares:s(5:1 3:1 2:3))
%Spec = spec(x:14 y:14 squares:s(5:2 4:4 3:3 2:5 1:35))
%Spec = spec(x:14 y:14 squares:s(5:2 4:4 3:3 2:10 1:15))
Spec = spec(x:17 y:20 squares:s(5:2 4:3 3:3 2:5 1:5))
%Spec = spec(x:10 y:12 squares:d(4:2 3:2 2:7 1:2))
%2*16+3*4+
{ExploreOne {Compile Spec}}

declare
fun {DrawSquares N Sol}
   DX=20 DY=20
   HX=10 HY=10
   T = {New Tk.toplevel tkInit(title:'Node '#N)}
   C = {New Tk.canvas tkInit(parent: T
			     bg:     ivory
			     width:  Sol.x * DX
			     height: Sol.y * DY)}
in
   {For 1 {Width Sol.squares} 1
    proc {$ I}
       Sq = Sol.squares.I
       SX = Sq.x
       SY = Sq.y
    in
       if {IsDet SX} andthen {IsDet SY} then
	  X=SX*DX
	  Y=SY*DY
       in
	  {C tk(create rectangle X+2 Y+2 X+Sq.d*DX-2 Y+Sq.d*DY-2
		fill:    c(60 179 113)
		width:   1
		outline: black)}
	  {C tk(create text X+Sq.d*HX Y+Sq.d*HY
		text:I)}
       end
    end}
   {Tk.send pack(C)}
   {Browse Sol}
   T#tkClose
end

{Explorer.object add(information fun {$ N Sol}
				    {DrawSquares N Sol}
				 end)}








*/