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
      mogulsecurl(single type:string)
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
			      uninstall edit config
			      %%again
			     )
	     default:build)
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
      config(           alias:action#config)
      %% again(            alias:action#again)

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
      )

   OPTLIST =
   [
    verbose        # set_verbose
    quiet          # set_quiet
    'just-print'   # set_justprint
    optlevel       # set_optlevel
    gnu            # set_gnu
    prefix         # set_prefix
    dir            # set_dir
    builddir       # set_builddir
    srcdir         # set_srcdir
    bindir         # set_bindir
    libdir         # set_libdir
    docdir         # set_docdir
    libroot        # set_libroot
    docroot        # set_docroot
    extractdir     # set_extractdir
    publishdir     # set_publishdir
    mogulpkgurl    # set_mogulpkgurl
    moguldocurl    # set_moguldocurl
    mogulsecurl    # set_mogulsecurl
    archive        # set_archive
    tmpdir         # set_tmpdir
    makefile       # set_makefile
    usemakepkg     # set_use_makepkg
    package        # set_package
    moguldatabase  # set_moguldatabase
    database       # set_database
    configfile     # set_configfile
    grade          # set_grade
    replacefiles   # set_replacefiles
    keepzombies    # set_keepzombies
    savedb         # set_savedb
    '_installdocs' # set_includedocs
    '_installlibs' # set_includelibs
    '_installbins' # set_includebins
    extendpackage  # set_extendpackage
    fullbuild      # set_fullbuild
    linewidth      # set_linewidth
    'local'        # set_local
    includedir     # set_includedirs
    librarydir     # set_librarydirs
    sysincludedirs # set_sysincludedirsok
    syslibrarydirs # set_syslibrarydirsok
   ]
      
define
   try
      Args    = {Application.getArgs OPTIONS}
      Man     = {New Manager.'class' init}
      Targets = {Map Args.1 StringToAtom}
   in
      for O#M in OPTLIST do
	 if {HasFeature Args O} then {Man M(Args.O)} end
      end
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
      [] config    then {Man config}
      %% [] again     then {Man again}
      end
      {Application.exit 0}
   catch E then
      {Wait Errors}
      {Error.printException E}
      {Application.exit 1}
   end
end
