%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
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
   FD Schedule

   
export
   compile: Compile

   
prepare

   local
      fun {Expand N I Is}
	 if N>0 then I|{Expand N-1 I+1 Is} else Is end
      end
      fun {Get J IN Is}
	 if J>0 then I#N=IN.J in {Expand N I {Get J-1 IN Is}}
	 else Is
	 end
      end
   in
      fun {IN2Is IN}
	 {Get {Width IN} IN nil}
      end
   end

   local
      fun {Get J IN}
	 N=IN.J.2
      in
	 if N>0 then J else {Get J+1 IN} end
      end
   in
      fun {GetMinSizeIN IN}
	 {Get 1 IN}
      end
   end

   local
      fun {Get J IN}
	 I#N=IN.J
      in
	 if N>0 then I else {Get J-1 IN} end
      end
   in
      fun {GetFirstIN IN}
	 {Get {Width IN} IN}
      end
   end

   local
      fun {Get J IN A}
	 if J>0 then {Get J-1 IN IN.J.2*J*J+A} else A end
      end
   in
      fun {GetAreaIN IN}
	 {Get {Width IN} IN 0}
      end
   end
   
define

   fun {Compile Spec}
      
      %% Specification is as follows:
      %%  Spec.x, Spec.y: size of the target plate
      %%  Spec.squares.D=N: N squares of size D

      %% Number of all squares
      N  = {Record.foldL Spec.squares Number.'+' 0}
      %% Dimension of X and Y
      DX = Spec.x
      DY = Spec.y
      %%
      MaxSize = {List.last {Arity Spec.squares}}
      SizeToArea = {MakeTuple '#' MaxSize}

      {For 1 MaxSize 1 proc {$ I}
			  SizeToArea.I=I*I
		       end}
      
      %% Mapping: Sqs -> Size
      SqsSize = {MakeTuple '#' N}

      %% Mapping: Sqs -> Area
      SqsArea = {MakeTuple '#' N}

      %% Initialize area and size
      {Record.foldRInd Spec.squares
       fun {$ D M I}
	  {For I I+M-1 1 proc {$ J}
			    SqsSize.J=D SqsArea.J=D*D
			 end}
	  I+M
       end 1 _}

   in
   
      proc {$ Root}
	 SqsX0 % left coordinates
	 SqsX1 % right coordinates
	 SqsY0 % upper coordinates
	 SqsY1 % lower coordinates
	 Cuts  % cut information
	 
	 local
	    
	    local
	       proc {Do J IN MN SqsXY0 SqsXY1 Cut}
		  if J>0 then I#N=IN.J M=MN.J in
		     {FD.distribute generic(value:mid) [M]}
		     {For I I+M-1 1 proc {$ I}
				       SqsXY1.I =<: Cut
				    end}
		     {For I+M I+N-1 1 proc {$ I}
					 SqsXY0.I >=: Cut
				      end}
		     {Do J-1 IN MN SqsXY0 SqsXY1 Cut}
		  end
	       end
	    in
	       proc {Position IN MN SqsXY0 SqsXY1 Cut}
		  {Do MaxSize IN MN SqsXY0 SqsXY1 Cut}
	       end
	    end

	    local
	       fun {Do J IN MN ?LIN ?RIN LN}
		  if J>0 then
		     I#N=IN.J M=MN.J
		  in
		     LIN.J = I    #M
		     RIN.J = (I+M)#N-M
		     {Do J-1 IN MN LIN RIN LN+M}
		  else
		     LN
		  end
	       end
	    in
	       fun {MakeLRIN IN MN ?LIN ?RIN}
		  LIN = {MakeTuple '#' MaxSize}
		  RIN = {MakeTuple '#' MaxSize}
		  {Do {Width IN} IN MN LIN RIN 0} 
	       end
	    end

	    proc {FindXY IN MN I MinSize Area XY0 XY1 DYX SqsXY0 SqsXY1
		  ?Cut}
	       AreaL = {FD.decl}
	    in
	       Cut = {FD.decl}
	       %% cut must be right of first square
	       Cut >=: XY0 + SqsSize.I
	       %% cut must be left of at least the smallest square
	       Cut + MinSize =<: XY1
	       %% numbers of squares for left side
	       %% they occupy AreaL
	       {FD.sumC SizeToArea MN '=:' AreaL}
	       %% which must fit the left of the cut
	       AreaL =<: (Cut-XY0) * DYX
	       %% not all squares must go to the left
	       AreaL  <: Area
	       %% position the squares
	       {FD.distribute mid [Cut]}
	       {Position IN MN SqsXY0 SqsXY1 Cut}
	    end
	    
	    fun {FindN N IN X0 X1 Y0 Y1}
	       I       = {GetFirstIN IN}
	       Area    = {GetAreaIN IN}
	       MinSize = {GetMinSizeIN IN}
	       MN      = {Record.map IN fun {$ _#N}
					   {FD.int 0#N} 
					end}
	    in
	       %% place first square
	       SqsX0.I=X0 SqsY0.I =Y0
	       %% mapping of sizes to number of squares on left side
	       choice
		  Cut={FindXY IN MN I MinSize Area X0 X1 Y1-Y0 SqsX0 SqsX1}
		  LIN RIN
		  LN={MakeLRIN IN MN ?LIN ?RIN}
	       in
		  info(cut:Cut dir:y
		       {Find LN   LIN X0 Cut Y0 Y1}
		       {Find N-LN RIN Cut X1 Y0 Y1})
	       []
		  Cut={FindXY IN MN I MinSize Area Y0 Y1 X1-X0 SqsY0 SqsY1}
		  LIN RIN
		  LN={MakeLRIN IN MN ?LIN ?RIN}
	       in
		  info(cut:Cut dir:x
		       {Find LN   LIN X0 X1 Y0 Cut}
		       {Find N-LN RIN X0 X1 Cut Y1})
	       end
	    end

	    fun {Find N IN X0 X1 Y0 Y1}
	       case N
	       of 0 then fail nil
	       [] 1 then [I]={IN2Is IN} in
		  SqsX0.I=X0 SqsY0.I=Y0
		  nil
	       [] 2 then
		  [I1 I2]={IN2Is IN}
		  S1X0=SqsX0.I1 S1X1=SqsX1.I1 S1Y0=SqsY0.I1 S1Y1=SqsY1.I1
		  S2X0=SqsX0.I2 S2Y0=SqsY0.I2
	       in
		  S1X0=X0 S1Y0=Y0 
		  dis S2X0=X0   S2Y0=S1Y1 then info(cut:S1Y1 dir:x nil nil)
		  []  S2X0=S1X1 S2Y0=Y0   then info(cut:S1X1 dir:y nil nil)
		  end
	       [] 3 then
		  [I1 I2 I3]={IN2Is IN}
		  S1X0=SqsX0.I1 S1X1=SqsX1.I1 S1Y0=SqsY0.I1 S1Y1=SqsY1.I1
		  S2X0=SqsX0.I2 S2X1=SqsX1.I2 S2Y0=SqsY0.I2 S2Y1=SqsY1.I2
		  S3X0=SqsX0.I3 S3Y0=SqsY0.I3
	       in
		  S1X0=X0 S1Y0=Y0 
		  dis S2X0=X0  S2Y0=S1Y1 S3X0=X0   S3Y0=S2Y1 then
		     info(cut:S1Y1 dir:x nil info(cut:S2Y1 dir:x nil nil))
		  []  S2X0=X0  S2Y0=S1Y1 S3X0=S1X1 S3Y0=S1Y1 then
		     info(cut:S1Y1 dir:x nil info(cut:S2X1 dir:y nil nil))
		  [] S2X0=S1X1 S2Y0=Y0   S3X0=S2X1 S3Y0=Y0   then
		     info(cut:S1X1 dir:y nil info(cut:S2X1 dir:y nil nil))
		  [] S2X0=S1X1 S2Y0=Y0   S3X0=S2X0 S3Y0=S2Y1 then
		     info(cut:S1X1 dir:y nil info(cut:S2Y1 dir:x nil nil))
		  end
	       else
		  {FindN N IN X0 X1 Y0 Y1}
	       end
	    end
	 in
	    fun {FindCuts}
	       IN={MakeTuple '#' MaxSize}
	    in
	       {ForThread MaxSize 1 ~1
		fun {$ I Size}
		   N={CondSelect Spec.squares Size 0}
		in
		   IN.Size = I#N
		   I+N
		end 1 _}
	       {Find N IN 0 DX 0 DY}
	    end
	 end

	 proc {Fit SqsXY Cap}
	    {Schedule.cumulative [{Arity SqsXY}] SqsXY SqsSize SqsSize [Cap]}
	 end

      in
      
	 SqsX0 = {MakeTuple '#' N}
	 SqsX1 = {MakeTuple '#' N}
	 SqsY0 = {MakeTuple '#' N}
	 SqsY1 = {MakeTuple '#' N}

	 Root = root(x:SqsX0 y:SqsY0 d:SqsSize dx:DX dy:DY cuts:Cuts)


	 %% Set up problem variables
	 {For 1 N 1 proc {$ I}
		       Size = SqsSize.I
		       X0   = {FD.int 0#{Max 0 DX-Size}}
		       Y0   = {FD.int 0#{Max 0 DY-Size}}
		       X1   = {FD.decl}
		       Y1   = {FD.decl}
		    in
		       X1 =: X0 + Size  Y1 =: Y0 + Size
		       SqsX0.I = X0  SqsY0.I = Y0  
		       SqsX1.I = X1  SqsY1.I = Y1  
		    end}
	 

	 %% The squares must fit the target
	 {FD.sum SqsArea '=<:' DX*DY}
      

	 %% Fix some freedom for first square (wolg)
	 SqsX0.1=0 SqsY0.1=0

	 %% Remove permutations of equally-sized squares by ordering them
	 {For 1 N-1 1 proc {$ I}
			 I1=I+1
		      in
			 if SqsSize.I==SqsSize.I1 then
			    %% This is respected by the no overlap
			    SqsX0.I =<: SqsX0.I1
			 end
		      end}

	 %% No Overlaps allowed
	 {FD.distinct2 SqsX0 SqsSize SqsY0 SqsSize}
	 
	 %% In any direction (be it x or y) the squares must
	 %% fit into the height/width of the rectangle
	 {Fit SqsX0 DY} {Fit SqsY0 DX}

	 %% The diffuclt part: find the cuts
	 Cuts = {FindCuts}

      end

   end

end
