declare
proc {Donald Root}
   sol(a:A b:B d:D e:E g:G l:L n:N  o:O r:R t:T) = Root
in
   Root ::: 0#9
   {FD.distinct Root}
   D\=:0  R\=:0  G\=:0
      100000*D + 10000*O + 1000*N + 100*A + 10*L + D
   +  100000*G + 10000*E + 1000*R + 100*A + 10*L + D
   =: 100000*R + 10000*O + 1000*B + 100*E + 10*R + T
   {FD.distribute ff Root}
end

{ExploreAll Donald}