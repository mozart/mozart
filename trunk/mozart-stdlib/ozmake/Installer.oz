functor
export
   'class' : Installer
import
   OS
   Path  at 'Path.ozf'
   Utils at 'Utils.ozf'
define
   class Installer

      meth install_from_package
	 DIR={OS.tmpnam}
      in
	 try
	    {self exec_mkdir(DIR)}
	    {self set_srcdir(DIR)}
	    {self set_builddir(DIR)}
	    {self set_extractdir(DIR)}
	    {self extract(writeMakefile:false)}
	    {self install_all}
	 finally
	    {self exec_rmdir(DIR)}
	 end
      end

      %% return all install targets according to both makefile
      %% and command line options

      meth get_install_targets($)
	 Stack = {Utils.newStack}
      in
	 if {self get_includedocs($)} then
	    for T in {self get_doc_targets($)} do {Stack.push T} end
	 end
	 if {self get_includelibs($)} then
	    for T in {self get_lib_targets($)} do {Stack.push T} end
	 end
	 if {self get_includebins($)} then
	    for T in {self get_bin_targets($)} do {Stack.push T} end
	 end
	 {Reverse {Stack.toList}}
      end

      meth target_to_installed_file(T $)
	 %% we add the platform suffix if necessary
	 %% so that we can check directly presence or absence
	 %% in the database.  This is also why we return atoms.
	 TT = {Path.maybeAddPlatform T}
      in
	 case {self target_to_section(T $)}
	 of bin then {Path.resolveAtom {self get_bindir($)} TT}
	 [] doc then {Path.resolveAtom {self get_docdir($)} TT}
	 else % lib or defaulting to lib
	    {Path.resolveAtom {self get_libdir($)} TT}
	 end
      end

      meth targets_to_installation_pairs(L $)
	 {Map L fun {$ T}
		   %% at this point the file to be installed should
		   %% have been built
		   {self make_src(T $)}
		   %% and we pair it with its destination
		   #Installer,target_to_installed_file(T $)
		end}
      end

      meth install(Targets)
	 if {self get_package_given($)} then Installer,install_from_package
	 elseif Targets==nil then Installer,install_all
	 else
	    {self build_targets(Targets)}
	    IPairs = Installer,targets_to_installation_pairs(Targets $)
	 in
	    Installer,install_ipairs(IPairs)
	 end
      end

      meth get_installation_pairs($)
	 ITargets = Installer,get_install_targets($)
	 IPairs   = Installer,targets_to_installation_pairs(ITargets $)
	 Stack = {Utils.newStackFromList {Reverse IPairs}}
      in
	 if {self get_local($)} then skip else
	    for M in {self get_submans($)} do
	       for P in {M get_installation_pairs($)} do {Stack.push P} end
	    end
	 end
	 {Reverse {Stack.toList}}
      end

      meth install_all
	 %% need to read the makefile before computing install targets
	 {self makefile_read}
	 {self build_all}
	 IPairs = {self get_installation_pairs($)}
	 %%!!! Targets = {self get_install_targets($)}
      in
	 Installer,install_ipairs(IPairs)
      end

      meth install_ipairs(IPairs) %installation pairs
	 %% need to read the makefile before building targets
	 {self makefile_read}
	 %% --grade=GRADE
	 {self database_read}
	 {self database_check_grade}
	 %%!!! %% make sure all targets have been built
	 %%!!! for T in Targets do {self build_target(T)} end
	 %% compute the files that will be written
	 %%!!! IPairs    = Installer,targets_to_installation_pairs(Targets $)
	 MOG       = {self get_mogul($)}
	 PKG       = {self database_mutable_entry(MOG $)}
	 LostStack = {Utils.newStack} LostFiles
	 IFiles    = {Map IPairs fun {$ _#F} F end}
      in
	 %% compute the files that will be overwritten and
	 %% belong to other packages
	 for _#F in IPairs do
	    FMog = {self file_to_package(F $)}
	 in
	    if FMog\=unit andthen FMog\=MOG then {LostStack.push F} end
	 end
	 LostFiles={LostStack.toList}
	 %% complain about overwriting unless --replacefiles
	 if LostFiles\=nil andthen {Not {self get_replacefiles($)}} then
	    raise ozmake(install:overwriting(LostFiles)) end
	 end
	 %% actually install the targets
	 {self trace('installing targets')}
	 {self incr}
	 try
	    for From#To in IPairs do
	       case {Path.extensionAtom From}
	       of 'exe' then
		  {self exec_install_exec(From To)}
		  %% we should use a symbolic link
		  {self exec_install_exec(From {Path.dropExtension To})}
	       else
		  {self exec_install_file(From To)}
	       end
	    end
	 finally {self decr} end
	 %% update database
	 %% move each lost file to its package lost list
	 for F in LostFiles do
	    Mog = {self file_to_package(F $)}
	    Pkg = {self database_mutable_entry(Mog $)}
	 in
	    Pkg.files := {Utils.diff  {CondSelect Pkg files nil} [F]}
	    Pkg.lost  := {Utils.union {CondSelect Pkg lost  nil} [F]}
	 end
	 %% take care of the previous installation
	 if {self get_extendpackage($)} then
	    %% if we are extending the package
	    %% add the files that were installed
	    PKG.files   := {Utils.union {CondSelect PKG files   nil} IFiles}
	    PKG.zombies := {Utils.diff  {CondSelect PKG zombies nil} IFiles}
	 else
	    %% else compute the left overs from the previous installation
	    %% i.e. every file in this package which is not being overwritten
	    LeftFiles = {Utils.diff {CondSelect PKG files nil} IFiles}
	 in
	    if {self get_keepzombies($)} then
	       %% move them to the zombie list
	       PKG.zombies := {Utils.union LeftFiles {CondSelect PKG zombies nil}}
	       PKG.lost    := {Utils.diff {CondSelect PKG lost nil} IFiles}
	       PKG.files   := IFiles
	    else
	       PKG.files := IFiles
	       %% forget that anything was ever lost
	       PKG.lost  := nil
	       %% remove all zombies and left overs
	       {self trace('removing zombies')}
	       {self incr}
	       try
		  for F in {CondSelect PKG zombies nil} do {self exec_rm(F)} end
		  PKG.zombies := nil
		  for F in LeftFiles do {self exec_rm(F)} end
	       finally {self decr} end
	    end
	 end
	 %% now update all the other features of the package
	 PKG.uri := {self get_uri($)}
	 local L={self get_author($)} in
	    if L\=unit then PKG.author := L end
	 end
	 local D={self get_released($)} in
	    if D\=unit then PKG.released := D end
	 end
	 PKG.installed := {Utils.dateCurrent}
	 local B={self get_blurb($)} in
	    if B\=unit then PKG.blurb := B end
	 end
	 local I={self get_info_text($)} in
	    if I\=unit then PKG.info_text := I end
	 end
	 local I={self get_info_html($)} in
	    if I\=unit then PKG.info_html := I end
	 end
	 %% finally update the database
	 {self database_save}
      end
      
   end
end