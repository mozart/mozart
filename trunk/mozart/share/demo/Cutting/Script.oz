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
   FD Schedule

export
   compile: Compile

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

      %% Mapping: I -> Atom(Int), needed for scheduling propagators
      IntToAtom = {MakeTuple '#' N}

      {For 1 N 1 proc {$ I}
		    IntToAtom.I = {VirtualString.toAtom I}
		 end}

   in
   
      proc {$ Root}
	 SqsX0 % left coordinates
	 SqsX1 % right coordinates
	 SqsY0 % upper coordinates
	 SqsY1 % lower coordinates
	 Cuts  % cut information
	 
	 local
	    fun {FindCut1 [I] X0 X1 Y0 Y1}
	       SqsX0.I=X0 SqsY0.I=Y0 X1>=:SqsX1.I Y1>=:SqsY1.I
	       nil
	    end
	    
	    proc {FindCut2 [I1 I2] X0 X1 Y0 Y1 ?Info}
	       S1X0=SqsX0.I1 S1X1=SqsX1.I1 S1Y0=SqsY0.I1 S1Y1=SqsY1.I1
	       S2X0=SqsX0.I2 S2X1=SqsX1.I2 S2Y0=SqsY0.I2 S2Y1=SqsY1.I2
	    in
	       S1X0=X0 S1Y0=Y0 
	       dis
		  S2X0=X0   S2Y0=S1Y1 X1>=:S1X1 Y1>=:S2Y1
	       then
		  Info=info(cut:S1Y1 dir:x nil nil)
	       []
		  S2X0=S1X1 S2Y0=Y0   X1>=:S2X1 Y1>=:S1Y1
	       then
		  Info=info(cut:S1X1 dir:y nil nil)
	       end
	    end

	    proc {FindCut3 [I1 I2 I3] X0 X1 Y0 Y1 ?Info}
	       S1X0=SqsX0.I1 S1X1=SqsX1.I1 S1Y0=SqsY0.I1 S1Y1=SqsY1.I1
	       S2X0=SqsX0.I2 S2X1=SqsX1.I2 S2Y0=SqsY0.I2 S2Y1=SqsY1.I2
	       S3X0=SqsX0.I3 S3X1=SqsX1.I3 S3Y0=SqsY0.I3 S3Y1=SqsY1.I3
	    in
	       S1X0=X0 S1Y0=Y0 
	       dis
		  S2X0=X0 S2Y0=S1Y1
		  dis %% in y direction: 1,2,3
		     S3X0=X0   S3Y0=S2Y1 X1>=:S1X1 Y1>=:S3Y1
		  then
		     Info=info(cut:S1Y1 dir:x nil info(cut:S2Y1 dir:x nil nil))
		  [] %% in y direction: 1,(2,3)
		     S3X0=S1X1 S3Y0=S1Y1 Y1>=:S2Y1 X1>=:{FD.max S1X1 S3X1}
		  then
		     Info=info(cut:S1Y1 dir:x nil info(cut:S2X1 dir:y nil nil))
		  end
	       [] S2X0=S1X1 S2Y0=Y0
		  dis %% in x direction: 1,2,3
		     S3X0=S2X1 S3Y0=Y0   X1>=:S3X1 Y1>=:S1Y1
		  then
		     Info=info(cut:S1X1 dir:y nil info(cut:S2Y1 dir:x nil nil))
		  [] %% in x direction: 1,(2,3)
		     S3X0=S2X0 S3Y0=S2Y1 X1>=:S2X1 Y1>=:{FD.max S1Y1 S3Y1}
		  then
		     Info=info(cut:S1X1 dir:y nil info(cut:S2X1 dir:y nil nil))
		  end
	       end
	    end

	    local
	       proc {Get Is Ps S N ?M ?SPs ?NIs ?NPs}
		  case Is of nil then
		     M=N SPs=nil NIs=nil NPs=nil
		  [] I|Ir then
		     if SqsSize.I==S then P|Pr=Ps in
			SPs=P|{Get Ir Pr S N+1 ?M $ ?NIs ?NPs}
		     else
			M=N SPs=nil NIs=Is NPs=Ps
		     end
		  end
	       end
	    in
	       proc {GetSameSize Is Ps ?M ?SPs ?NIs ?NPs}
		  case Is of nil then
		     SPs=nil M=0 NIs=nil NPs=nil
		  [] I|Ir then P|Pr=Ps in
		     SPs=P|{Get Ir Pr SqsSize.I 1 ?M $ ?NIs ?NPs}
		  end
	       end
	    end

	    local
	       proc {AssignZero Ps}
		  case Ps of nil then skip
		  [] P|Pr then P=0 {AssignZero Pr}
		  end
	       end
	       proc {AssignOne Ps M}
		  if M>0 then 1|Pr=Ps in {AssignOne Pr M-1}
		  else {AssignZero Ps}
		  end
	       end
	    in
	       proc {Arrange Is Ps}
		  if Is\=nil then SPs NIs NPs N M in
		     {GetSameSize Is Ps ?N ?SPs ?NIs ?NPs}
		     M::0#N
		     {FD.distribute generic(value:mid) '#'(M)}
		     {AssignOne SPs M}
		     {Arrange NIs NPs}
		  end
	       end
	    end

	    proc {Arrangement Ps Is ?Ls ?Rs}
	       case Ps of nil then
		  Ls=nil Rs=nil
	       [] P|Pr then I|Ir=Is in
		  case P
		  of 0 then Rs=I|{Arrangement Pr Ir Ls $}
		  [] 1 then Ls=I|{Arrangement Pr Ir $ Rs}
		  end
	       end
	    end
	    
	    fun {GetPos Is SqsXY0 SqsXY1 Cut}
	       case Is of nil then nil
	       [] I|Ir then
		  ((SqsXY1.I=<:Cut) = (SqsXY0.I<:Cut))
		  |{GetPos Ir SqsXY0 SqsXY1 Cut}
	       end
	    end

	    proc {FindCutM Is X0 X1 Y0 Y1 ?Info}
	       %% More than three squares
	       Cut      = {FD.decl}
	       I|Ir     = Is
	       As       = {Map Is fun {$ I} SqsArea.I end}
	       Area     = {FoldL As Number.'+' 0}
	       MinSize  = {FoldL Ir fun {$ M I}
				       {Min SqsSize.I M}
				    end SqsSize.I}
	    in
	       %% Arrange first square
	       SqsX0.I = X0
	       SqsY0.I = Y0
	       choice
		  Ls Rs InfoL InfoR
		  Ps
		  DL={FD.decl}
		  PL={FD.decl}
	       in
		  Cut >=: X0+SqsSize.I
		  Cut + MinSize =<: X1
		  DL =: (Cut - X0) * (Y1 - Y0)
		  Ps = {GetPos Is SqsX0 SqsX1 Cut}
		  {FD.sumC As Ps '=:' PL}
		  PL <: Area
		  PL =<: DL
		  Info = info(dir:y cut:Cut InfoL InfoR)
		  {FD.distribute generic(value:mid) [Cut]}
		  {Arrange Is Ps}
		  {Arrangement Ps Is ?Ls ?Rs}
		  InfoL={FindCut Ls X0 Cut Y0 Y1}
		  InfoR={FindCut Rs Cut X1 Y0 Y1}
	       []
		  Ls Rs InfoL InfoR
		  Ps
		  DL={FD.decl}
		  PL={FD.decl}
	       in
		  Cut >=: Y0+SqsSize.I
		  Cut + MinSize =<: Y1
		  DL =: (Cut - Y0) * (X1 - X0)
		  Ps = {GetPos Is SqsY0 SqsY1 Cut}
		  {FD.sumC As Ps '=:' PL}
		  PL <: Area
		  PL =<: DL
		  Info = info(dir:x cut:Cut InfoL InfoR)
		  {FD.distribute generic(value:mid) [Cut]}
		  {Arrange Is Ps}
		  {Arrangement Ps Is ?Ls ?Rs}
		  InfoL = {FindCut Ls X0 X1 Y0 Cut}
		  InfoR = {FindCut Rs X0 X1 Cut Y1}
	       end
	    end
	    
	 in
	    fun {FindCut Is X0 X1 Y0 Y1}
	       case {Length Is}
	       of 0 then fail nil
	       [] 1 then {FindCut1 Is X0 X1 Y0 Y1}
	       [] 2 then {FindCut2 Is X0 X1 Y0 Y1}
	       [] 3 then {FindCut3 Is X0 X1 Y0 Y1}
	       else      {FindCutM Is X0 X1 Y0 Y1}
	       end
	    end
	 end

	 proc {Fit SqsXY Cap}
	    Tasks = IntToAtom
	    As    = {Record.foldR Tasks fun {$ A Ar} A|Ar end nil}
	    Dur   = {Record.make dur   As}
	    Start = {Record.make start As}
	    Use   = Dur
	 in
	    {For 1 N 1
	     proc {$ I}
		A = Tasks.I
	     in
		Dur.A=SqsSize.I Start.A=SqsXY.I
	     end}
	    {Schedule.cumulative [Tasks] Start Dur Use [Cap]}
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
			 I1 = I+1
		      in
			 if SqsSize.I==SqsSize.I1 then
			    %% This is respected by the no overlap
			    SqsX0.I * DY + SqsY0.I =<:
			    SqsX0.I1 * DY + SqsY0.I1
			 end
		      end}

	 %% No Overlaps allowed
	 {FD.distinct2 SqsX0 SqsSize SqsY0 SqsSize}
	 
	 %% In any direction (be it x or y) the squares must
	 %% fit into the height/width of the rectangle
	 {Fit SqsX0 DY}
	 {Fit SqsY0 DX}
      
	 Cuts = {FindCut {List.number 1 N 1} 0 DX 0 DY}

      end

   end

end

