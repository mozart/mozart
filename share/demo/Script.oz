declare


proc {BinaryPartition ?S1 ?S2 S}
   %% Sets S1 and S2 are binary partition of S
   C1 C2
in
   {FS.disjoint S1 S2} {FS.union S1 S2 S}
   C1={FS.card S1} C2={FS.card S2}
   C1>:0 C2>:0 C1+C2=:{FS.card S}
end

local
   fun {Run I1 I2 Is}
      I1|if I1==I2 then Is else {Run I1+1 I2 Is} end
   end
in
   fun {UnfoldSetSpec Ss}
      case Ss of nil then nil
      [] S|Sr then
	 case S of I1#I2 then {Run I1 I2 {UnfoldSetSpec Sr}}
	 else S|{UnfoldSetSpec Sr}
	 end
      end
   end
end

fun {Compile Spec PlaceAll}

   %% Specification is as follows:
   %%  Spec.x, Spec.y: size of the target plate
   %%  Spec.squares.D=N: N squares of size D
   
   N  = {Record.foldL Spec.squares Number.'+' 0}
   DX = Spec.x
   DY = Spec.y


   %% Area covered by squares.
   proc {Covered Sqs}
      {FD.sum {Record.map Sqs proc {$ Sq ?Res}
				 Area = Sq.d * Sq.d
			      in
				 Res :: [0 Area]
				 Sq.placed =: (Area =: Res)
			      end} '=<:' DX*DY}
   end

   proc {NoOverlap Sqs}
      %% No rectangles must overlap, return variables for arrangement
      thread
	 {For 1 N 1
	  proc {$ I}
	     Sq1 = Sqs.I
	  in
	     if Sq1.placed==1 then
		X1=Sq1.x Y1=Sq1.y D1=Sq1.d
	     in
		{For 1 I-1 1
		 proc {$ J}
		    Sq2=Sqs.J
		 in
		    if Sq2.placed==1 then
		       X2=Sq2.x Y2=Sq2.y D2=Sq2.d
		    in
		       (X1 + D1 =<: X2) +  (X2 + D2 =<: X1) +
		       (Y1 + D1 =<: Y2) +  (Y2 + D2 =<: Y1) >: 0
		    end
		 end}
	     end
	  end}
      end
   end

   proc {Map3 Xs F ?Y1s ?Y2s ?Y3s}
      case Xs of nil then Y1s=nil Y2s=nil Y3s=nil
      [] X|Xr then Y1|Y1r=Y1s Y2|Y2r=Y2s Y3|Y3r=Y3s in
	 {F X ?Y1 ?Y2 ?Y3} {Map3 Xr F Y1r Y2r Y3r}
      end
   end

   fun {SwapXY XY}
      case XY of x then y [] y then x end
   end

   fun {Mix Xs Ys}
      case Xs of nil then Ys
      [] X|Xr then
	 case Ys of nil then Xs
	 [] Y|Yr then X|Y|{Mix Yr Xr}
	 end
      end
   end
	 
   fun {MakeCuts Info Sqs}
      XY  = Info.xy
      SQS = Info.sqs
   in
      cond {FS.card SQS}<:4 then
	 {FD.distribute splitMin
	  {FoldR {UnfoldSetSpec {FS.reflect.upperBound SQS}}
	   fun {$ I XYr}
	      Sqs.I.x|Sqs.I.y|XYr
	   end nil}}
	 nil
      else
	 DimXY = Info.XY
	 YX = {SwapXY XY}
	 DimYX = Info.YX
	 SQS1    = {FS.var.decl}
	 SQS2    = {FS.var.decl}
	 Cut     = {FD.decl}
	 IsLs IsRs DDs
	 DimD ={FD.decl}
	 InfoL InfoR
	 CurSqs  = {UnfoldSetSpec {FS.reflect.upperBound SQS}}
	 DimL={FD.decl}
	 DimR={FD.decl}
      in
	 DimD    =: DimYX.2 - DimYX.1
	 %% SQS1 and SQS2 are are partition of the squares
	 {BinaryPartition SQS1 SQS2 SQS}
	 %% The cut must be inside the current dimensions
	 DimXY.1<:Cut Cut<:DimXY.2
	 %% Find Left and Right, just cardinality matters
	 {Map3 CurSqs
	  proc {$ I ?IsL ?IsR ?DD}
	     Sq=Sqs.I SqXY=Sq.XY SqD=Sq.d
	  in
	     IsL = (Cut >=: SqXY + SqD)
	     IsR = (Cut =<: SqXY)
	     {FD.nega IsL IsR}
	     IsL = {FS.reified.isIn I SQS1}
	     IsR = {FS.reified.isIn I SQS2}
	     DD  = SqD*SqD
	  end ?IsLs ?IsRs ?DDs}
	 %% However, the capacities must fit (redundant)
	 DimL =: DimD * Cut - DimD * DimXY.1 
	 {FD.sumC DDs IsLs '=<:' DimL} 
	 DimR =: DimD * DimXY.2 - DimD * Cut
	 {FD.sumC DDs IsRs '=<:' DimR} 
	 %% Find the cut
%	 {FD.distribute generic(value:mid) [{FS.card SQS1}]}
	 %% Distribute squares to be either right or left
	 {FD.distribute generic(value:max) {Mix IsLs IsRs}}
%	 {FD.distribute generic(value:mid) [Cut]}
	 %% Continue recursively
	 %% Place the guys to the left
%	 {FD.distribute splitMin
%	  {FoldR {List.zip CurSqs IsLs fun {$ I IsL} I#IsL end}
%	   fun {$ I#IsL XYs}
%	      if IsL==1 then Sqs.I.XY|XYs
%	      else XYs
%	      end
%	   end nil}}
	 info(xy:YX XY:DimXY.1#Cut YX:DimYX sqs:SQS1)#
	 info(xy:YX XY:Cut#DimXY.2 YX:DimYX sqs:SQS2)
	 %% Place the guys to the right
%	 {FD.distribute splitMin
%	  {FoldR {List.zip CurSqs IsRs fun {$ I IsR} I#IsR end}
%	   fun {$ I#IsR XYs}
%	      if IsR==1 then Sqs.I.XY|XYs
%	      else XYs
%	      end
%	   end nil}}
      end
   end

   fun {DriveCuts Infos Sqs}
      if Infos==nil then nil else
	 Infos|{DriveCuts {FoldL Infos
			   fun {$ Is I}
			      NI={MakeCuts I Sqs}
			   in
			      case NI
			      of nil then Is
			      [] I1#I2 then I1|I2|Is
			      end
			   end nil} Sqs}
      end
   end
   
   proc {Distribute Sqs ?Cuts}
      %% Place all rectangles
      if PlaceAll then
	 {Record.forAll Sqs proc {$ Sq}
			       Sq.placed = 1
			    end}
      else
	 {FD.distribute generic(order:naive value:max)
	  {Record.map Sqs fun {$ Sq} Sq.placed end}}
      end
      %% Size down the unplaced sqaures
      {Record.forAll Sqs proc {$ Sq}
			    if Sq.placed==0 then
			       Sq.x=0 Sq.y=0
			    end
			 end}
      %% Now comes the real stragey:
      %%  1. Find a position for the cut, starting from the middle
      %%  2. Distribute all squares either to the left or right side
      %%     of the cut
      Cuts={DriveCuts
	    [info(xy:x x:0#DX y:0#DY sqs:{FS.value.make [1#N]})] Sqs}
      {FD.distribute naive
       {FoldL Cuts
	fun {$ XYs Infos}
	   {FoldL Infos fun {$ XYs Info}
			   Info.x.1|Info.x.2|Info.y.1|Info.y.2|XYs
			end XYs}
	end nil}}
   end

   
in
   
   proc {$ Root}
      root(squares:Sqs cuts:Cuts x:!DX y:!DY) = Root
   in

      Sqs = {MakeTuple '#' N}
      
      %% Set up problem variables
      {Record.foldRInd Spec.squares
       fun {$ D M I}
	  {For I I+M-1 1 proc {$ J}
			    Sqs.J=square(x:      {FD.int 0#{Max 0 DX-D}}
					 y:      {FD.int 0#{Max 0 DY-D}}
					 d:      D   
					 placed: {FD.bool})
			 end}
	  I+M
       end 1 _}
      
      %% The placed squares must fit the target
      {Covered Sqs}

      %% Fix some freedom for first square (wolg)
      Sqs.1.x =<: Sqs.1.y
      
      %% Remove permutations of equally-sized squares by ordering them
      {For 1 N-1 1 proc {$ I}
		      Sq1=Sqs.I Sq2=Sqs.(I+1)
		   in
		      if Sq1.d==Sq2.d then
%			 Sq1.x =<: Sq2.x
%			 (Sq1.x \=: Sq2.x) + (Sq1.y <: Sq2.y) >=: 1
			 Sq1.x*DY + Sq1.y <: Sq2.x*DY + Sq2.y
		      end
		   end}

      %% Squares must not overlap
      {NoOverlap Sqs}

      %% At any position (be it x or y) the placed squares must
      %% fit into the height/width of the rectangle
      {ForAll [x#DY y#DX]
       proc {$ XY#DXY}
	  {For 0 DXY-1 1
	   proc {$ I}
	      {FD.sum {Record.map Sqs
		       proc {$ Sq ?D}
			  D :: [0 Sq.d]
			  {FD.conj Sq.placed
			   %% this the same as:
			   %%  {FD.conj I=<:Sq.XY Sq.XY+Sq.d>=:I}
			   (Sq.XY :: {Max 0 I-Sq.d+1}#I)
			   (D=:Sq.d)}
		       end} '=<:' DXY}
	   end}
       end}

      
%      Cuts = {MakeCuts x {FS.value.make [1#N]} Sqs}

      Cuts={Distribute Sqs}

   end

end



declare
%Spec = spec(x:5 y:5 squares:s(3:1 2:2))
%Spec = spec(x:7 y:9 squares:s(2:4 5:1 1:1 3:1))
%Spec = spec(x:8 y:8 squares:s(4:4))
Spec = spec(x:16 y:14 squares:s(5:2 4:3 3:3 2:5 1:4))
%Spec = spec(x:12 y:9 squares:d(4:2 3:2 2:5 1:3))
%2*16+3*4+
{ExploreOne {Compile
	     Spec
	     true}}
/*

declare
S={Compile
	     Spec
   true}
declare R
{Browse R}

{S R}
*/

	     

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
       if {IsDet Sq.placed} andthen Sq.placed==1 andthen
	  {IsDet SX} andthen {IsDet SY}
       then
	  X=SX*DX
	  Y=SY*DY
       in
	  {C tk(create rectangle X+2 Y+2 X+Sq.d*DX-2 Y+Sq.d*DY-2
		fill:    blue
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





