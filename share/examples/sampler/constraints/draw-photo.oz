%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

declare DrawPhoto in

local
   Mid            = 80
   Dist           = 120
   FontFamily     = '-*-helvetica-medium-r-normal--*-'
   BoldFontFamily = '-*-helvetica-bold-r-normal--*-'
   FontMatch      = '-*-*-*-*-*-*'

   Font           = BoldFontFamily#240#FontMatch
   SmallFont      = BoldFontFamily#140#FontMatch
   FullWidth      = Dist * ({Length Persons} + 1)
   FullHeight     = 280
   HalfHeight     = 140

   fun {ComputeDis N Ful}
      {FoldL {List.zip Prefs Ful fun {$ X Y} X#Y end}
       fun {$ I (A#B)#F}
	  case A==N andthen {FD.reflect.size F}==1 andthen F==0 then I+1
	  else I
	  end
       end 0}
   end

in
   
   fun {DrawPhoto Node Sol}
      NotPos = {Record.filter Sol.pos
		fun {$ P} {FD.reflect.size P}>1 end}

      %% Windowing part
      W = {New Tk.toplevel tkInit(title:'Photo Alignment ('#Node#')')}
      C = {New Tk.canvas   tkInit(parent: W
				  width:  FullWidth
				  height: case {Width NotPos}==0 then
					     HalfHeight
					  else FullHeight
					  end
				  bg:     white)}
      {Tk.send pack(C)}

   in
   
      {Record.forAllInd Sol.pos
       proc {$ N P}
	  case {FD.reflect.size P}==1 then Dis={ComputeDis N Sol.sat} in
	     {C tk(crea text P*Dist Mid o(font:Font text:N))}
	     case Dis>0 then
		{C tk(crea text P*Dist Mid-30
		      font:SmallFont text:~Dis fill:red)}
		{C tk(crea oval P*Dist - 15 Mid-45  P*Dist + 15 Mid-15
		      outline:red width:2)}
	     else skip end
	  else skip
	  end
       end}

      {Record.foldLInd NotPos
       fun {$ N I P}
	  Dis={ComputeDis N Sol.sat}
       in
	  {C tk(crea text I 2*Mid font:Font text:N fill:gray)}
	  case Dis>0 then 
	     {C tk(crea text I*Dist 2*Mid+30
		   font:SmallFont text:~Dis fill:red)}
	     {C tk(crea oval I*Dist - 15 Mid+45  I*Dist + 15 Mid+15
		   outline:red width:1)}
	  else skip end
	  {ForAll {FD.reflect.domList P}
	   proc {$ To}
	      {C tk(crea line I 2*Mid - 20 To*Dist Mid + 20
		    arrow:last width:1 fill:gray)}
	   end}
	  I+Dist
       end
       (FullWidth - {Width NotPos} * Dist + Dist) div 2
       _}

      {ForAll {List.zip Prefs Sol.sat fun {$ X Y} X#Y end}
       proc {$ (NA#NB)#F}
	  case {FD.reflect.size F}>1 then skip
	  elsecase F==1 then
	     L = {Min Sol.pos.NA Sol.pos.NB}
	     R = {Max Sol.pos.NA Sol.pos.NB}
	  in
	     {C tk(crea line
		   L*Dist + 40           Mid
		   R*Dist - 40           Mid
		   arrow: case {Member NB#NA Prefs} andthen
			     (F2={Nth Sol.sat {List.foldLInd Prefs
					       fun {$ I P A#B}
						  case A#B==NB#NA then I
						  else P
						  end
					       end 0}}
			    in {FD.reflect.size F2}==1 andthen F2==1)
			  then both
			  elsecase Sol.pos.NA < Sol.pos.NB then last
			  else first
			  end
		   width: 3
		   fill:  blue)}
	  else skip
	  end
       end}
      
      W # tkClose
   end

end


	  
