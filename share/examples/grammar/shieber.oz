%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Gert Smolka
%%%  Email: smolka@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$


\feed 'examples/grammar/shieber/formalism.oz'
\feed 'examples/grammar/shieber/grammar.oz'

/*

% parse 

{GrammarAgent query(proc {$ F} 
		       F^lexin = [uther persuades cornwall to sleep] 
		    end)}

% parse 

{GrammarAgent query(proc {$ F} 
		       F^lexin = [uther _ cornwall _ _ _]
		    end)}


% generate all sentences

{GrammarAgent query(proc {$ F} 
		       skip
		    end)}

% generate from logical form

{GrammarAgent query(proc {$ F} 
		       F^head^lf = [persuade uther kuno [beat kuno cornwall]]
		    end)}

{Browse {GrammarAgent next($)}}


% generate all sentences in perfect tense

{GrammarAgent query(proc {$ F} 
		       F^head^lf = [perfective _]
		    end)}

{GrammarAgent next}

% generate all sentences in perfect tense with subject knights

{GrammarAgent query(proc {$ F} 
		       F^head^lf = [perfective (_|knights|_)]
		    end)}

% generate all sentences with main verb storm

{GrammarAgent query(proc {$ F} 
		       LF = F^head^lf
		    in
		       dis LF = storm|_
		       [] LF = perfective|(storm|_)|_
		       end
		    end)}

% generate all sentences with at most 3 words

declare
proc {LengthLE L N}
   dis L=nil
   [] LL in L=_|LL N>:0 then {LengthLE LL N-1}
   end
end

{GrammarAgent query(proc {$ F} 
		       {LengthLE F^lexin 6}
		    end)}

*/
