fun {$ IMPORT}
   \insert '../lib/import.oz'

   Kalotan =
   proc {$ Solution}
      Vars
      [Sex Claim Truth        % Kibi
       Sex1 Truth1            % Parent 1
       Sex2 Truth2a Truth2b]  % Parent 2
      = Vars
   in
      Vars:::0#1
      (Claim=:Sex)=:Truth
      Sex+Truth>:0
      (Claim=:0)=:Truth1
      Sex1+Truth1>:0
      (Sex=:1)=:Truth2a
      (Truth=:0)=:Truth2b
      Sex2+Truth2a+Truth2b=:2
      Sex1\=:Sex2
      Solution=Sex#Sex1#Sex2
      {FD.distribute ff Vars}
   end

   KalotanSol = [1#1#0]
in

   fd([kalotan([
                one(equal(fun {$} {SearchOne Kalotan} end
                          KalotanSol)
                    keys: [fd])
               ])
      ])

end
