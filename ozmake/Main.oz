functor
import
   Application Error
   Manager at 'Manager.ozf'
prepare
   OPTIONS =
   record(
      verbose(single char:&v type:bool)
      'just-print'(single char:&n type:bool)
      optlevel(single type:atom(none debug optimize))
      debug(char:&g alias:optlevel#debug)
      optimize(char:&O alias:optlevel#optimize)
      
      prefix(single type:string)
      dir(single type:string)
      builddir(single type:string)
      srcdir(single type:string)
      bindir(single type:string)
      libdir(single type:string)
      docdir(single type:string)
      libroot(single type:string)
      docroot(single type:string)
      extractdir(single type:string)

      makefile(single type:string)

      action(single type:atom(build fullbuild) default:build)
      build(char:&b alias:action#build)
      fullbuild(char:&B alias:action#fullbuild)

      grade(single type:atom(none up down same any))
      upgrade(alias:grade#up)
      downgrade(alias:grade#down)
      replace(alias:grade#any)
      
      )

   OPTLIST =
   [
    verbose      # set_verbose
    'just-print' # set_justprint
    optlevel     # set_optlevel
    prefix       # set_prefix
    dir          # set_dir
    builddir     # set_builddir
    srcdir       # set_srcdir
    bindir       # set_bindir
    libdir       # set_libdir
    docdir       # set_docdir
    libroot      # set_libroot
    docroot      # set_docroot
    extractdir   # set_extractdir
    makefile     # set_makefile
    grade        # set_grade
   ]
      
define
   try
      Args    = {Application.getArgs OPTIONS}
      Man     = {New Manager.'class' init}
      Targets = Args.1
   in
      for O#M in OPTLIST do
	 if {HasFeature Args O} then {Man M(Args.O)} end
      end
      case Args.action
      of build then {Man build(Targets)}
      [] fullbuild then
	 {Man set_fullbuild(true)}
	 {Man build(Targets)}
      end
      {Application.exit 0}
   catch E then
      {Error.printException E}
      {Application.exit 1}
   end
end
