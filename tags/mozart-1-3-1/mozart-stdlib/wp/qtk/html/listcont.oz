declare

[QTk]={Module.link ["x-oz://system/wp/QTk.ozf"]}

Builder={QTk.newBuilder}

{ForAll [td lr grid]
 proc{$ V}
    {Builder.setAlias {VirtualString.toAtom V#l}
     fun{$ M}
	Num Other
     in
	{Record.partitionInd M fun{$ I _} {Int.is I} end Num Other}
	{Record.adjoin
	 Other
	 {List.toTuple V
	  {List.flatten {Record.toList Num}}}}
     end}
 end}

%{{Builder.build td(tdl([label(text:"A")] [label(text:"B") label(text:"C")] label(text:"D")))} show}
