declare
proc {Money Root}
   S E N D M O R Y
in
   Root = sol(s:S e:E n:N d:D m:M o:O r:R y:Y)
   Root ::: 0#9
   {FD.distinct Root}
   S \=: 0 
   M \=: 0
                1000*S + 100*E + 10*N + D
   +            1000*M + 100*O + 10*R + E
   =: 10000*M + 1000*O + 100*N + 10*E + Y
   {FD.distribute ff Root}
end

{ExploreAll Money}

/*
{Browse {SearchOne Money}}

{Browse {SearchAll Money}}

*/