local
   
   % QTk builder
   
   MyBuilder={QTk.newBuilder}

   % Alias as a record specifying default parameters
   
   {MyBuilder.setAlias ilabel label(bg:black fg:white)}
   {{MyBuilder.build td(ilabel(text:"Inverted label")  
			ilabel(text:"Inverted label with overriden foreground color" 
			       fg:red))} show}

   % Alias as a function processed at construction time

   {MyBuilder.setAlias tdl
    fun{$ M}
       Num Other
    in
       {Record.partitionInd M fun{$ I _} {Int.is I} end Num Other}
       {Record.adjoin
	Other
	{List.toTuple td
	 {List.flatten {Record.toList Num}}}}
    end}

   {{MyBuilder.build td(tdl([label(text:"A")]
			    [label(text:"B") label(text:"C")]
			    label(text:"D")))} show}

   % Alias as a class definition

   {MyBuilder.setAlias labelddlb
    class $ 
       feat label ddlb
       meth labelddlb(...)=M
	  M.(QTk.qTkDesc)=lr(glue:we
			     label(handle:self.label glue:we)
			     dropdownlistbox(handle:self.ddlb
					     action:self#select
					     init:M.init))
	  thread
	     % waits for the widget to be built
	     {Wait self.ddlb}
	     % actions to do when the widget is first built
	     if M.init\=nil then
		{self.label set(M.init.1)}
	     end
	  end
       end
       meth set(...)=M
	  self.ddlb,M
	  {self select}
       end
       meth get(...)=M
	  self.ddlb,M
       end 
       meth select
	  {self.label set({self.ddlb get(firstselection:$)})}
       end
    end}
   {{MyBuilder.build lr(labelddlb(init:[1 2 3 4]))} show}
in skip
end