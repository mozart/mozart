functor
export
   'class' : Uninstaller
prepare
   VSToAtom = VirtualString.toAtom
define
   class Uninstaller

      meth uninstall
	 MOG =
	 if {self get_package_given($)} then
	    {VSToAtom {self get_package($)}}
	 else
	    {self makefile_read}
	    if {self get_no_makefile($)} then
	       raise ozmake(uninstall:missingpackageormogul) end
	    else
	       {self get_mogul($)}
	    end
	 end
	 {self database_read}
	 PKG = {self database_get_package(MOG $)}
      in
	 if PKG==unit then
	    raise ozmake(uninstall:packagenotfound(MOG)) end
	 else
	    FILES   = {CondSelect PKG files nil}
	    ZOMBIES = {CondSelect PKG zombies nil}
	 in
	    {self trace('uninstalling package '#MOG)}
	    {self incr}
	    try
	       {self trace('removing files')}
	       {self incr}
	       try
		  if FILES\=unit then
		     for F in FILES do {self exec_rm(F)} end
		  end
	       finally {self decr} end
	       {self trace('removing zombies')}
	       {self incr}
	       try
		  if ZOMBIES\=unit then
		     for F in ZOMBIES do {self exec_rm(F)} end
		  end
	       finally {self decr} end
	       {self database_remove_package(MOG)}
	       {self database_save}
	    finally {self decr} end
	 end
      end

   end
end
