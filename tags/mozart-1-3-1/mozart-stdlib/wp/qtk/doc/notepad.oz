declare
[QTk]={Module.link ["http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf"]}

proc{SaveText}
   Name={QTk.dialogbox save($)}
in 
   try 
      File={New Open.file init(name:Name flags:[write create truncate])}
      Contents={TextHandle get($)}
   in 
      {File write(vs:Contents)}
      {File close}
   catch _ then skip end 
end 

proc{LoadText}
   Name={QTk.dialogbox load($)}
in 
   try 
      File={New Open.file init(name:Name)}
      Contents={File read(list:$ size:all)}
   in 
      {TextHandle set(Contents)}
      {File close}
   catch _ then skip end 
end

Toolbar=lr(glue:we
	   tbbutton(text:"Save" glue:w action:SaveText)
	   tbbutton(text:"Load" glue:w action:LoadText)
	   tbbutton(text:"Quit" glue:w action:toplevel#close))

TextHandle

Window={QTk.build td(Toolbar
		     text(glue:nswe handle:TextHandle bg:white tdscrollbar:true))}

{Window show}

