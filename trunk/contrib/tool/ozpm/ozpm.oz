functor

   Application

import Archive QTk
   System(show:Show
	  showInfo:ShowInfo)

define

   Platform={Property.get 'platfrom.os'}
   
   OZPMINFO="~/.oz/ozpm/ozpm.info"

   fun{ExtractPath P}
      {Reverse
       {List.takeWhile
	{Reverse P}
	
   
   class ArchiveManagerClass

      meth init skip end

      meth list()
	 skip
      end

      meth install()
	 skip
      end
      
      meth info()
	 skip
      end

      meth create()
	 skip
      end

      meth check()
	 skip
      end
      
   end
   
   class InteractiveManager

      meth init()
	 skip
      end

   end
   
   
   Args={Application.getArgs
	 record('zip'( single type:string optional:false)
		'action'(single type:atom(list extract create interactive) default:interactive)
		'list'(char:&l alias:'action'#list)
		'extract'(char:&x alias:'action'#extract)
		'create'(char:&c alias:'action'#create)
		'from'(single type:string)
		'to'(  single type:string)
		'files'(multiple type:list(string))
	       )}

   InteractiveMode=Args.'action'==interactive
   
   OzpmInfo=fun lazy {$}
	       try {Pickle.load OZPMINFO}
	       catch error(url(load OZPMINFO) ...) then
		  if InteractiveMode then
		     %% application not properly installed, try to recover
		     %% interactively
		     R
		     {{QTk.build td(title:"Oz Package Manager"
				    action:toplevel#close
				    label(text:"First time installation (or recovery)\n\nContinue installation ?"
					  padx:10 pady:10)
				    lr(glue:swe
				       button(text:"Ok"
					      action:toplevel#close
					      return:R)
				       button(text:"Cancel"
					      action:toplevel#close)))} show(wait:true)}
		  in
		     if R then
			
		     else {Application.exit 0} end
		  else
		     {ShowInfo "ozpm not installed properly, please start it in interactive mode or reinstall"}
		  end
	       end
	    end

   ArchiveManager={New ArchiManagerClass init}

   case Args.'action'
   of list then % list all installed packages
   [] install then % install/update a specified package
   [] create then % create a new package
   [] info then % displays information about an installed package
   [] check then % check installed packages integrity and rebuilds if necessary
   [] interactive then % start the application in interactive mode
      _={New InteractiveManager init}
   end

end
