local
   fun{Purge L}
      case L
      of &\r|Ls then {Purge Ls}
      [] Le|Ls then Le|{Purge Ls}
      else nil end
   end
   FileList
   HomeDir
   local
      P={Application.getCmdArgs plain}
      HD=if P==nil then "./PrototyperData" else P.1 end
   in
      HomeDir=if {List.last HD}==47 then HD else HD#"/" end
   end
   try
      L={List.filter {OS.getDir HomeDir}
	 fun{$ N}
	    {List.take {Reverse N} 3}=="zo." % file ending by a .oz extension
	 end}
   in
      FileList={List.sort
		{List.map L fun{$ N} {List.take N {Length N}-3} end}
		fun{$ A B}
		   {String.toAtom A}<{String.toAtom B}
		end}
   catch _ then FileList=nil end
   ListObj
   FileNameVar
   NfoText
   CodeText
   CurName={NewCell nil}

   class MyApp
   
      meth init
	 skip
      end

      meth run
	 Code={List.append
	       "try\n"
	       {List.append
		{CodeText get($)}
		"\ncatch E then {Error.printException E} end\nunit\n"}}
      in
	 thread
	    try
	       {Compiler.evalExpression Code
		env('QTk':QTk 'OS':OS 'Compiler':Compiler 'System':System
		    'Application':Application
		    'Open':Open 'Show':System.show 'Error':Error)
		_ _}
	    catch E then
	       {Error.printException E}
	    end
	 end
      end
   
      meth loadCurFile
	 Name={Access CurName}
	 {FileNameVar set(Name)}
      in
	 try
	    HOZ={New Open.file init(url:HomeDir#Name#".oz" flags:[read])}
	    COZ={HOZ read(list:$ size:all)}
	    {HOZ close}
	 in
	    {CodeText set({Purge COZ})}
	 catch _ then {CodeText set("")}
	 end
      end
   
      meth chgFile
	 Ind={ListObj get(firstselection:$)}
      in
	 if Ind\=0 then
	    Name={List.nth FileList Ind}
	 in
	    {Assign CurName Name}
	    try
	       HOZ={New Open.file init(url:HomeDir#Name#".nfo" flags:[read])}
	       COZ={HOZ read(list:$ size:all)}
	       {HOZ close}
	    in
	       {NfoText set({Purge COZ})}
	    catch _ then
	       {NfoText set("No information available.")}
	    end
	    {self loadCurFile}
	 else
	    {Assign CurName ""}
	    {NfoText set("")}
	    {CodeText set("")}
	 end
      end
   end

   App={New MyApp init}

   Desc=td(title:"QTk Prototyper (alpha)"
	   lr(glue:nwe
	      menubutton(glue:w
			 text:"File"
			 menu:menu(command(text:"About"
					   action:proc{$}
						     {{QTk.build td(label(text:"Author : Donatien Grolaux")
								    button(glue:s padx:5 pady:5
									   text:"Close"
									   action:toplevel#close))} show(wait:true modal:true)}
						  end)
				   separator
				   command(text:"Exit"
					   action:toplevel#close)))
	      label(glue:e
		    handle:FileNameVar)
	     )
	   tdrubberframe(glue:nswe padx:2 pady:2
			 td(glue:nswe
			    lrrubberframe(glue:nswe
					  td(glue:nswe
					     listbox(glue:nswe bg:white
						     handle:ListObj
						     tdscrollbar:true
						     init:FileList
						     action:App#chgFile))
					  td(glue:nswe
					     text(glue:nswe bg:white
						  tdscrollbar:true
						  wrap:word
						  handle:NfoText))))
			 td(glue:nswe
			    text(glue:nswe bg:white
				 tdscrollbar:true
				 handle:CodeText)))
	   lr(glue:swe
	      button(glue:w padx:5 pady:5
		     text:"Run"
		     action:App#run)
	      button(glue:w padx:5 pady:5
		     text:"Revert"
		     action:App#loadCurFile)
	     ))
   Window={QTk.build Desc}

   {NfoText bind(event:"<FocusIn>"
		 action:proc{$} {ListObj getFocus} end)}
in
   {ListObj set(selection:{List.map FileList fun{$ F} F=="Click_here_to_begin" end})}
   {App chgFile}
   {Window show(wait:true)}
end
