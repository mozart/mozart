functor
export
   'class' : Uninstaller
prepare
   VSToAtom = VirtualString.toAtom
   fun {UnitToNil X}
      if X==unit then nil else X end
   end
   DictKeys = Dictionary.keys
import
   Utils at 'Utils.ozf'
   Path  at 'Path.ozf'
define
   class Uninstaller

      meth uninstall
	 MOG =
	 if {self get_package_given($)} then
	    Pkg={self get_package($)}
	 in
	    if {Utils.isMogulID Pkg} then
	       {VSToAtom {self get_package($)}}
	    elsecase {self load_extract_mogulid(Pkg $)}
	    of unit then
	       raise ozmake(uninstall:bizzarepackage(Pkg)) end
	    [] A then A end
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
	    DFILES = {NewDictionary}
	    for F in {UnitToNil {CondSelect PKG files nil}} do
	       DFILES.F := unit
	    end
	    FILES = {Sort {DictKeys DFILES} Value.'>'}
	    DZOMBIES = {NewDictionary}
	    for F in {UnitToNil {CondSelect PKG zombies nil}} do
		  DZOMBIES.F := unit
	    end
	    ZOMBIES = {Sort {DictKeys DZOMBIES} Value.'>'}
	 in
	    {self trace('uninstalling package '#MOG)}
	    {self incr}
	    try
	       {self trace('removing files')}
	       {self incr}
	       try
		  for F in FILES do {self exec_rm(F)} end
	       finally {self decr} end
	       {self trace('removing zombies')}
	       {self incr}
	       try
		  for F in ZOMBIES do {self exec_rm(F)} end
	       finally {self decr} end
	       {self trace('removing empty directories')}
	       {self incr}
	       try
		  for F in FILES   do {self rmEmptyDirs({Path.dirnameURL F})} end
		  for F in ZOMBIES do {self rmEmptyDirs({Path.dirnameURL F})} end
	       finally {self decr} end
	       {self database_remove_package(MOG)}
	       {self database_save}
	    finally {self decr} end
	 end
      end

      meth rmEmptyDirs(F)
	 if {Path.isDir F} andthen {Path.ls F}==nil then
	    {self exec_rmdir(F)}
	    {self rmEmptyDirs({Path.dirnameURL F})}
	 end
      end

   end
end
