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
      archive(   single type:string)
      tmpdir(    single type:string)

      makefile(single char:&m type:string)
      contactfile(single      type:string)
      usemakepkg( single type:bool)
      package( single char:&p type:string)
      database(single         type:string)
      moguldatabase(single    type:string)
      configdatabase(single   type:string)
      packageversion(single char:&V type:string)

      action(single type:atom(build install clean veryclean
			      create publish extract list help
			      uninstall edit checkneeded
			      %%mogul
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
      checkneeded(      alias:[action#checkneeded grade#freshen])
      %% again(            alias:action#again)

      config(single type:string
	     validate:alt(when(action false) when(true optional)))

      grade(single type:atom(none up down same any freshen))
      upgrade(  char:&U alias:[action#install grade#up])
      downgrade(        alias:[action#install grade#down])
      anygrade( char:&A alias:[action#install grade#any])
      freshen(  char:&F alias:[action#install grade#freshen])
      replacefiles(single type:bool)
      replace(  char:&R alias:[action#install grade#any replacefiles#true])
      extend(   char:&X alias:[action#install grade#any extendpackage#true])
      extendpackage(single type:bool)
      keepzombies(single type:bool)
      savedb(single type:bool)
      requires(single type:bool)

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

      mogul(      single type:string)
      mogulpkgurl(single type:string)
      moguldocurl(single type:string)
      moguldburl( single type:string)
      mogulpkgdir(single type:string)
      moguldocdir(single type:string)
      moguldbdir( single type:string)
      mogulrootid(single type:string)
      moguldir(   single type:string)
      mogulurl(   single type:string)

      exe(single type:atom(default no yes both multi))
      makepkgfile(single type:string)

      '_speed'(single type:bool)
      fast(alias:['_speed'#true])
      slow(alias:['_speed'#false])

      autodepend(single type:bool)
      )

   OPTLIST =
   [
    %% OPTION      # SET METHOD           # CONFIGURABLE?	# ON-FORK?
    verbose        # set_verbose	  # false		# true
    quiet          # set_quiet            # false		# true
    'just-print'   # set_justprint        # false		# true
    optlevel       # set_optlevel         # true		# false
    gnu            # set_gnu              # true		# true
    prefix         # set_prefix           # true		# true
    dir            # set_dir              # false		# false
    builddir       # set_builddir         # false		# false
    srcdir         # set_srcdir           # false		# false
    bindir         # set_bindir           # false		# true
    libdir         # set_libdir           # false		# false
    docdir         # set_docdir           # false		# false
    libroot        # set_libroot          # true		# true
    docroot        # set_docroot          # true		# true
    extractdir     # set_extractdir       # false		# false
    mogulpkgurl    # set_mogulpkgurl      # true		# false
    moguldocurl    # set_moguldocurl      # true		# false
    moguldburl     # set_moguldburl       # true		# false
    archive        # set_archive          # true		# true
    tmpdir         # set_tmpdir           # true		# true
    makefile       # set_makefile         # false		# false
    contactfile    # set_contactfile      # false		# false
    usemakepkg     # set_use_makepkg      # false		# false
    package        # set_package          # false		# false
    moguldatabase  # set_moguldatabase    # true		# true
    database       # set_database         # true		# true
    configfile     # set_configfile       # true		# true
    grade          # set_grade            # false		# false
    replacefiles   # set_replacefiles     # false		# true
    keepzombies    # set_keepzombies      # false		# true
    savedb         # set_savedb           # false		# true
    '_installdocs' # set_includedocs      # false		# false
    '_installlibs' # set_includelibs      # false		# false
    '_installbins' # set_includebins      # false		# false
    extendpackage  # set_extendpackage    # false		# false
    fullbuild      # set_fullbuild        # false		# false
    linewidth      # set_linewidth        # true		# true
    'local'        # set_local            # false		# false
    includedir     # set_includedirs      # true		# false
    librarydir     # set_librarydirs      # true		# false
    sysincludedirs # set_sysincludedirsok # true		# false
    syslibrarydirs # set_syslibrarydirsok # true		# false
    mogulpkgdir    # set_mogulpkgdir      # true		# true
    moguldocdir    # set_moguldocdir      # true		# true
    moguldbdir     # set_moguldbdir       # true		# true
    exe            # set_exe              # true		# true
    makepkgfile    # set_makepkgfile      # true		# false
    mogul          # set_mogul_action     # false		# false
    mogulrootid    # set_mogulrootid      # true		# true
    moguldir       # set_moguldir         # true		# true
    mogulurl       # set_mogulurl         # true		# true
    config         # set_config_action    # false		# false
    '_speed'       # set_fast             # true		# true
    packageversion # set_want_version     # false		# false
    autodepend     # set_autodepend       # true		# true
    requires       # set_dorequires       # false               # true
   ]
      
define
   try
      local Args_ = {Application.getArgs OPTIONS} in
	 Args = if {HasFeature Args_ action} then Args_
		elseif {HasFeature Args_ config} then
		   {AdjoinAt Args_ action config}
		elseif {HasFeature Args_ mogul} then
		   {AdjoinAt Args_ action mogul}
		else
		   {AdjoinAt Args_ action build}
		end
      end
      Man     = {New Manager.'class' init}
      Targets = {Map Args.1 StringToAtom}
   in
      %% we may need this info for processing "requires"
      {Man set_args(Args)}
      {Man set_optlist(OPTLIST)}
      %% process the options supplied by the user
      for Key#Set#_#_ in OPTLIST do
	 if {HasFeature Args Key} then {Man Set(Args.Key)} end
      end
      %% read some default configuration parameters
      {Man config_install(Args OPTLIST)}
      %% only use MAKEPKG.oz if installing from a package and not
      %% explicitly disabled by the user
      {Man set_default_use_makepkg(Args.action==install andthen
				   {Man get_package_given($)})}
      case Args.action
      of build       then {Man build(Targets)}
      [] install     then {Man install(Targets)}
      [] clean       then {Man clean}
      [] veryclean   then {Man veryclean}
      [] create      then {Man create}
      [] publish     then {Man publish}
      [] extract     then {Man extract}
      [] list        then {Man list}
      [] help        then {Help.help}
      [] uninstall   then {Man uninstall}
      [] edit        then {Man makefileEdit}
      [] config      then {Man config(Args OPTLIST)}
      [] mogul       then {Man mogul(Targets)}
      [] checkneeded then {Application.exit {Man checkneeded($)}}
      %% [] again     then {Man again}
      end
      {Application.exit 0}
   catch E then
      {Wait Errors}
      {Error.printException E}
      {Application.exit 1}
   end
end
