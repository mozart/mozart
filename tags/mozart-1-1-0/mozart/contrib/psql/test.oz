local
   functor  F
   import
      Psql 
      System(show:Show)
      Application(exit)
      OS(getEnv)
   define   
      proc {RShow R} 
	{Record.forAllInd R 
	  proc {$ A B}
	    {Show A#{String.toAtom B}}
          end}
      end
 
      try
	SQL Name in

        SQL = {Psql.startSQL
 	     "host=pets.sics.se dbname=ozstuff "#
	     "user=ozuser password=mozart"}

	% Select and print stuff
        {ForAll {SQL.query "SELECT * FROM person WHERE age>10"} RShow}
      
        Name =  {OS.getEnv 'USER'}#"@"#{OS.getEnv 'HOST'}

	% Insert stuff
	_ = {SQL.insert person(age:"0" name:Name)}

      catch C then {Show caught(C)} end	
      {Application.exit 0}
   end
in
%    {{New Module.manager init} apply(url:'' F _)}
   F
end

