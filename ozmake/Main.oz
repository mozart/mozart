functor
import
   Application Error
   Manager at 'Manager.ozf'
   Help    at 'Help.ozf'
   Errors  at 'Errors.ozf'
prepare
   OPTIONS =
   record(
      verbose(multiple char:&v type:bool)
      quiet(       single char:&q type:bool)
      'just-print'(single char:&n type:bool)
      optlevel(    single         type:atom(none debug optimize))
      debug(              char:&g alias:optlevel#debug)
      optimize(           char:&O alias:optlevel#optimize)
      gnu(         single         type:bool)
      linewidth(   single         type:int)
      'local'(     single         type:bool)
      
      prefix(    single type:string)
      dir(       single type:string)
      builddir(  single type:string)
      srcdir(    single type:string)
      bindir(    single type:string)
      libdir(    single type:string)
      docdir(    single type:string)
      libroot(   single type:string)
      docroot(   single type:string)
      extractdir(single type:string)
      publishdir(single type:string)
      mogulpkgurl(single type:string)
      moguldocurl(single type:string)
      moguldburl( single type:string)
      archive(   single type:string)
      tmpdir(    single type:string)

      makefile(single char:&m type:string)
      usemakepkg( single type:bool)
      package( single char:&p type:string)
      database(single         type:string)
      moguldatabase(single    type:string)
      configdatabase(single   type:string)

      action(single type:atom(build install clean veryclean
			      create publish extract list help
			      uninstall edit mogul
			      %%again
			     ))
      build(    char:&b alias:action#build)
      install(  char:&i alias:action#install)
      fullbuild(single type:bool)
      clean(            alias:action#clean)
      veryclean(        alias:action#veryclean)
      create(   char:&c alias:action#create)
      publish(          alias:action#publish)
      extract(  char:&x alias:action#extract)
      list(     char:&l alias:action#list)
      help(     char:&h alias:action#help)
      uninstall(char:&e alias:action#uninstall)
      edit(             alias:action#edit)
      mogul(            alias:action#mogul)
      %% again(            alias:action#again)

      config(single type:atom(set unset list)
	     validate:alt(when(action false) when(true optional)))

      grade(single type:atom(none up down same any))
      upgrade(  char:&U alias:[action#install grade#up])
      downgrade(        alias:[action#install grade#down])
      anygrade( char:&A alias:[action#install grade#any])
      replacefiles(single type:bool)
      replace(  char:&R alias:[action#install grade#any replacefiles#true])
      extend(   char:&X alias:[action#install grade#any extendpackage#true])
      extendpackage(single type:bool)
      keepzombies(single type:bool)
      savedb(single type:bool)

      '_installdocs'(single type:bool)
      includedocs(alias:'_installdocs'#true)
      excludedocs(alias:'_installdocs'#false)
      '_installlibs'(single type:bool)
      includelibs(alias:'_installlibs'#true)
      excludelibs(alias:'_installlibs'#false)
      '_installbins'(single type:bool)
      includebins(alias:'_installbins'#true)
      excludebins(alias:'_installbins'#false)

      includedir(multiple char:&I type:string)
      librarydir(multiple char:&L type:string)
      sysincludedirs(single type:bool)
      syslibrarydirs(single type:bool)

      mogulpkgdir(single type:string)
      moguldocdir(single type:string)
      moguldbdir( single type:string)
      )

   OPTLIST =
   [
    %% OPTION      # SET METHOD           # CONFIGURABLE?
    verbose        # set_verbose	  # false
    quiet          # set_quiet            # false
    'just-print'   # set_justprint        # false
    optlevel       # set_optlevel         # true
    gnu            # set_gnu              # true
    prefix         # set_prefix           # true
    dir            # set_dir              # false
    builddir       # set_builddir         # false
    srcdir         # set_srcdir           # false
    bindir         # set_bindir           # false
    libdir         # set_libdir           # false
    docdir         # set_docdir           # false
    libroot        # set_libroot          # true
    docroot        # set_docroot          # true
    extractdir     # set_extractdir       # false
    publishdir     # set_publishdir       # true
    mogulpkgurl    # set_mogulpkgurl      # true
    moguldocurl    # set_moguldocurl      # true
    moguldburl     # set_moguldburl       # true
    archive        # set_archive          # true
    tmpdir         # set_tmpdir           # true
    makefile       # set_makefile         # false
    usemakepkg     # set_use_makepkg      # false
    package        # set_package          # false
    moguldatabase  # set_moguldatabase    # true
    database       # set_database         # true
    configfile     # set_configfile       # true
    grade          # set_grade            # false
    replacefiles   # set_replacefiles     # false
    keepzombies    # set_keepzombies      # false
    savedb         # set_savedb           # false
    '_installdocs' # set_includedocs      # false
    '_installlibs' # set_includelibs      # false
    '_installbins' # set_includebins      # false
    extendpackage  # set_extendpackage    # false
    fullbuild      # set_fullbuild        # false
    linewidth      # set_linewidth        # true
    'local'        # set_local            # false
    includedir     # set_includedirs      # true
    librarydir     # set_librarydirs      # true
    sysincludedirs # set_sysincludedirsok # true
    syslibrarydirs # set_syslibrarydirsok # true
    mogulpkgdir    # set_mogulpkgdir      # true
    moguldocdir    # set_moguldocdir      # true
    moguldbdir     # set_moguldbdir       # true
   ]
      
define
   try
      local Args_ = {Application.getArgs OPTIONS} in
	 Args = if {HasFeature Args_ action} then Args_
		elseif {HasFeature Args_ config} then
		   {AdjoinAt Args_ action config}
		else
		   {AdjoinAt Args_ action build}
		end
      end
      Man     = {New Manager.'class' init}
      Targets = {Map Args.1 StringToAtom}
   in
      %% process the options supplied by the user
      for Key#Set#_ in OPTLIST do
	 if {HasFeature Args Key} then {Man Set(Args.Key)} end
      end
      %% read some default configuration parameters
      {Man config_install(Args OPTLIST)}
      %% only use MAKEPKG.oz if installing from a package and not
      %% explicitly disabled by the user
      {Man set_default_use_makepkg(Args.action==install andthen
				   {Man get_package_given($)})}
      case Args.action
      of build     then {Man build(Targets)}
      [] install   then {Man install(Targets)}
      [] clean     then {Man clean}
      [] veryclean then {Man veryclean}
      [] create    then {Man create}
      [] publish   then {Man publish}
      [] extract   then {Man extract}
      [] list      then {Man list}
      [] help      then {Help.help}
      [] uninstall then {Man uninstall}
      [] edit      then {Man makefileEdit}
      [] config    then {Man config(Args OPTLIST)}
      [] mogul     then {Man mogul}
      %% [] again     then {Man again}
      end
      {Application.exit 0}
   catch E then
      {Wait Errors}
      {Error.printException E}
      {Application.exit 1}
   end
end
