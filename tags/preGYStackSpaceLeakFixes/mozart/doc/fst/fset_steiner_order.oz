local
   N1 = N+1  N1N1 = N1*N1
in
   {ForAllTail {Map Ss fun {$ S}
			  {FD.list 3 1#N} = {FS.int.match S}
		       end}
    proc {$ T}
       case T of [X1 X2 X3]|[Y1 Y2 Y3]|_ then
	  N1N1*X1 + N1*X2 + X3 <: N1N1*Y1 + N1*Y2 + Y3
       else skip end
    end}
end

