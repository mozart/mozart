%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Gert Smolka
%%%  Email: smolka@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

declare

GrammarAgent = {New class $ from BaseObject
                       prop final
                       attr gram:nil
                       meth grammar(type:_ gram:_) = G
                          gram <- G
                       end
                       meth query(P)
                          Type = @gram.type
                          Gram = @gram.gram
                       in
                          {ExploreOne proc {$ F}
                                         {Type s F}
                                         {P F} {Gram F}
                                      end}
                       end
                    end noop}
