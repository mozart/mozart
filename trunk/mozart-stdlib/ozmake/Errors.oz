functor
import Error
prepare
   TITLE           = 'ozmake error'
   TITLE_CREATE    = 'ozmake [create] error'
   TITLE_DATABASE  = 'ozmake [database] error'
   TITLE_BUILD     = 'ozmake [build] error'
   TITLE_EXTRACT   = 'ozmake [extract] error'
   TITLE_INSTALL   = 'ozmake [install] error'
   TITLE_MAKEFILE  = 'ozmake [makefile] error'
   TITLE_UNINSTALL = 'ozmake [uninstall] error'
   TITLE_MOGUL     = 'ozmake [mogul] error'
   fun {OzMakeErrorFormatter E}
      case E
      of ozmake(get_uri) then
	 error(kind : TITLE
	       msg  : 'package has no uri'
	       items: [line('add a `uri\' feature to the makefile')])
      [] ozmake(get_mogul) then
	 error(kind : TITLE
	       msg  : 'package has no mogul id'
	       items: [line('add a `mogul\' feature to the makefile')])
      [] ozmake(get_package) then
	 error(kind : TITLE
	       msg  : 'no package file'
	       items: [line('use -p FILE or --package=FILE in your command')])
      [] ozmake(create:hasscheme(PKG S)) then
	 error(kind : TITLE_CREATE
	       msg  : 'package to be created should be a filename not a URL'
	       items: [hint(l:'Package' m:PKG)
		       line('it should not have a scheme')
		       hint(l:'Scheme' m:S)])
      [] ozmake(create:write(PKG)) then
	 error(kind : TITLE_CREATE
	       msg  : 'error saving pickled package to '#PKG)
      [] ozmake(database:filenotfound(S)) then
	 error(kind : TITLE_DATABASE
	       msg  : 'package database not found'
	       items: [hint(l:'Database' m:S)])
      [] ozmake(database:nograde(Old New Cmp)) then
	 error(kind : TITLE_DATABASE
	       msg  : 'package already installed'
	       hint : [hint(l:'Release date of installed version' m:Old)
		       hint(l:'Release date of this package' m:New)
		       case Cmp
		       of eq then
			  line('use --grade=(same|up|down|any) in your command')
		       [] gt then
			  line('use --grade=(up|any) in your command')
		       [] lt then
			  line('use --grade=(down|any) un your command')
		       end])
      [] ozmake(database:samegrade(Old New Cmp)) then
	 error(kind : TITLE_DATABASE
	       msg  : 'package already installed with different release date'
	       items: [hint(l:'Release date of installed version' m:Old)
		       hint(l:'Release date of this package' m:New)
		       case Cmp
		       of gt then
			  line('use --grade=(up|any) in your command')
		       [] lt then
			  line('use --grade=(down|any) in your command')
		       end])
      [] ozmake(database:upgrade(Old New)) then
	 error(kind : TITLE_DATABASE
	       msg  : 'package already installed with newer release date'
	       items: [hint(l:'Release date of installed version' m:Old)
		       hint(l:'Release date of this package' m:New)
		       line('use --grade=(down|any) in your command')])
      [] ozmake(database:downgrade(Old New)) then
	 error(kind : TITLE_DATABASE
	       msg  : 'package already installed with older release date'
	       items: [hint(l:'Release date of installed version' m:Old)
		       hint(l:'Release date of this package' m:New)
		       line('use --grade=(up|any) in your command')])
      [] ozmake(filenotfound:F) then
	 error(kind : TITLE
	       msg  : 'file not found'
	       items: [hint(l:'File' m:F)])
      [] ozmake(build:outdated(T)) then
	 error(kind : TITLE_BUILD
	       msg  : 'target still outdated'
	       items: [hint(l:'Target' m:T)])
      [] ozmake(build:unknowntool(T)) then
	 error(kind : TITLE_BUILD
	       msg  : 'unknown tool'
	       items: [hint(l:'Tool' m:T)])
      [] ozmake(build:shell(CMD)) then
	 error(kind : TITLE_BUILD
	       msg  : 'shell command failed'
	       items: [hint(l:'Shell command' m:CMD)])
      [] ozmake(mkdir:S) then
	 error(kind : TITLE
	       msg  : 'cannot create directory'
	       items: [hint(l:'Directory' m:S)])
      [] ozmake(rmdir:S) then
	 error(kind : TITLE
	       msg  : 'cannot remove directory'
	       items: [hint(l:'Directory' m:S)])
      [] ozmake(rm:S) then
	 error(kind : TITLE
	       msg  : 'cannot remove file'
	       items: [hint(l:'File' m:S)])
      [] ozmake(mkexec:S) then
	 error(kind : TITLE
	       msg  : 'cannot make file executable'
	       items: [hint(l:'File' m:S)])
      [] ozmake(extract:badrecord(R)) then
	 error(kind : TITLE_EXTRACT
	       msg  : 'package value is not a record'
	       items: [hint(l:'Value' m:oz(R))])
      [] ozmake(extract:noinfo(R)) then
	 error(kind : TITLE_EXTRACT
	       msg  : 'package value is missing feature `info\''
	       items: [hint(l:'Value' m:oz(R))])
      [] ozmake(extract:nodata(R)) then
	 error(kind : TITLE_EXTRACT
	       msg  : 'package value is missing feature `data\''
	       items: [hint(l:'Value' m:oz(R))])
      [] ozmake(extract:baddata(D)) then
	 error(kind : TITLE_EXTRACT
	       msg  : 'data feature of package value should be a list'
	       items: [hint(l:'Data value' m:oz(D))])
      [] ozmake(extract:filenotvs(F)) then
	 error(kind : TITLE_EXTRACT
	       msg  : 'filename of packed file is not a virtual string'
	       items: [hint(l:'Filename' m:oz(F))])
      [] ozmake(extract:filenotrelative(F)) then
	 error(kind : TITLE_EXTRACT
	       msg  : 'filename of packed file is not relative'
	       items: [hint(l:'Filename' m:F)])
      [] ozmake(extract:filedatanotvs(F D)) then
	 error(kind : TITLE_EXTRACT
	       msg  : 'contents of packed file is not a virtual string'
	       items: [hint(l:'Filename' m:F)
		       hint(l:'Contents' m:oz(D))])
      [] ozmake(extract:baddatapair(X)) then
	 error(kind : TITLE_EXTRACT
	       msg  : 'expected FILENAME#CONTENTS in list on package value `data\' feature'
	       items: [hint(l:'Found' m:oz(X))])
      [] ozmake(extract:load(P)) then
	 error(kind : TITLE_EXTRACT
	       msg  : 'cannot read or download package'
	       items: [hint(l:'Package' m:P)])
      [] ozmake(extract:dir(D)) then
	 error(kind : TITLE_EXTRACT
	       msg  : 'default directory for extraction already exists'
	       items: [hint(l:'Directory' m:D)])
      [] ozmake(install:overwriting(L)) then
	 error(kind : TITLE_INSTALL
	       msg  : 'need to overwrite files owned by other packages'
	       items: hint(l:'Files' m:L.1)
	       |{Append {Map L.2 fun {$ F} hint(m:F) end}
		 [line('use --replacefiles in your command')]})
      [] ozmake(makefile:notrecord(R)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'makefile value is not a record'
	       items: [hint(l:'Value' m:oz(R))])
      [] ozmake(makefile:illegalfeature(F L)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'illegal feature in makefile'
	       items: [hint(l:'Feature' m:F)
		       hint(l:'Valid features' m:list(L ' '))])
      [] ozmake(makefile:baduri(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'illegal value for `uri\' feature'
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:urinoscheme(S)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'given uri has no scheme'
	       items: [hint(l:'URI' m:S)])
      [] ozmake(makefile:badmogul(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'illegal value for `mogul\' feature'
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:mogulbadscheme(S)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected mogul id on `mogul\' feature'
	       items: [hint(l:'Value' m:S)
		       line('it should begin with mogul:/')])
      [] ozmake(makefile:badreleased(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'unparsable released date'
	       items: [hint(l:'Value' m:oz(V))
		       line('this should be a string of the form "YYYY-MM-DD-HH:MM:SS"')])
      [] ozmake(makefile:badclean(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected a list of virtual strings on feature `clean\''
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:badveryclean(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected a list of virtual strings on feature `veryclean\''
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:badauthor(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected a virtual string or a list of virtual strings on feature `author\''
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:badblurb(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected a virtual string on feature `blurb\''
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:badinfotext(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected a virtual string on feature `info_text\''
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:badinfohtml(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected a virtual string on feature `info_html\''
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:badrequires(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected a virtual string or a list of virtual strings on feature \'requires\''
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:badsectionvalue(F V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected a list of targets on feature `'#F#'\''
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:badsectionentry(F E)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'found non-virtual string target on feature `'#F#'\''
	       items: [hint(l:'Value' m:oz(E))])
      [] ozmake(makefile:badsectiontarget(F T)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'found non-relative target on feature `'#F#'\''
	       items: [hint(l:'Target' m:T)])
      [] ozmake(makefile:badbinextension(F)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : '`bin\' target should have `.exe\' extension'
	       items: [hint(l:'Target' m:F)])
      [] ozmake(makefile:duplicate(F)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'duplicate target'
	       items: [hint(l:'Target' m:F)])
      [] ozmake(makefile:baddepends(D)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected a record on `depends\' feature'
	       items: [hint(l:'Value' m:oz(D))])
      [] ozmake(makefile:baddependstarget(F)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'non relative target on `depends\' record'
	       items: [hint(l:'Target' m:F)])
      [] ozmake(makefile:baddependslist(F L)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'illegal dependency list in `depends\' record'
	       items: [hint(l:'Target' m:F)
		       hint(l:'Dependencies' m:oz(L))])
      [] ozmake(makefile:baddependsvalue(A D)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected virtual string in dependency list of target'
	       items: [hint(l:'Target' m:A)
		       hint(l:'Bad dependency' m:oz(D))])
      [] ozmake(makefile:baddependsentry(A D)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'found non-relative dependency'
	       items: [hint(l:'Target' m:A)
		       hint(l:'Dependency' m:D)])
      [] ozmake(makefile:badrules(R)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected a record on feature `rules\''
	       items: [hint(l:'Value' m:oz(R))])
      [] ozmake(makefile:badrulestarget(F)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'found non-relative target of rule'
	       items: [hint(l:'Target' m:F)])
      [] ozmake(makefile:badrule(F R)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'illegal rule in makefile'
	       items: [hint(l:'Target' m:F)
		       hint(l:'Rule' m:oz(R))
		       line('it should be of the form TOOL(FILE) or TOOL(FILE OPTIONS)')
		       line('where TOOL is one of {ozc,ozl,cc,ld}')])
      [] ozmake(makefile:badrulefile(F)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'found non-relative file argument in rule'
	       items: [hint(l:'File' m:F)])
      [] ozmake(makefile:illegaltooloption(T O)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'illegal option tool for tool'
	       items: [hint(l:'Tool' m:T)
		       hint(l:'Option' m:oz(O))])
      [] ozmake(makefile:badtooloption(T O)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'incorrect value of valid tool option'
	       items: [hint(l:'Tool' m:T)
		       hint(l:'Option' m:oz(O))])
      [] ozmake(makefile:filenotfound(F)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'makefile not found'
	       items: [hint(l:'Filename' m:F)])
      [] ozmake(makefile:subdirnotvs(D)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'non virtual string in subdirs list'
	       items: [hint(l:'Value' m:oz(D))])
      [] ozmake(makefile:subdirnotbasename(D)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'element of subdirs list should be a simple filename (no slash)'
	       items: [hint(l:'Value' m:D)])
      [] ozmake(makefile:badsubdirs(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'feature `subdirs\' should be a list of filenames'
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:badsubmakefiles(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'feature `submakefiles\' should be a record of makefile records'
	       items: [hint(l:'Value' m:oz(V))])
      [] ozmake(makefile:submakefilesnotallowed) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'feature `submakefiles\' not allowed in user makefile')
      [] ozmake(makefile:badtopics(V)) then
	 error(kind : TITLE_MAKEFILE
	       msg  : 'expected a virtual string or a list of virtual string on feature \'topics\''
	       items: [hint('Value' m:oz(V))])
      [] ozmake(uninstall:missingpackageormogul) then
	 error(kind : TITLE_UNINSTALL
	       msg  : 'no package or makefile'
	       items: [line('use either --package=PKG or --makefile=FILE in your command')])
      [] ozmake(uninstall:bizarrepackage(MOG)) then
	 error(kind : TITLE_UNINSTALL
	       msg  : '--package argument is neither a mogul id nor an existing file or URL'
	       items: [hint(l:'Package' m:MOG)])
      [] ozmake(uninstall:packagenotfound(MOG)) then
	 error(kind : TITLE_UNINSTALL
	       msg  : 'package not found in database'
	       items: [hint(l:'Package mogul id' m:MOG)
		       line('perhaps this package is not installed')])
      [] ozmake(compiling:F VS) then
	 error(kind : TITLE
	       msg  : 'error while compiling Oz file '#F#VS
	       footer:false)
      [] ozmake(mogul:nopkgurl) then
	 error(kind : TITLE_MOGUL
	       msg  : 'your MOGUL url for packages is not known'
	       items: [line('either supply it with --mogulpkgurl=URL')
		       line('or setup a default using:')
		       line('')
		       line('    ozmake --config=set --mogulpkgurl=URL')
		       line('')
		       line('this is the URL corresponding to the directory')
		       line('specified by --mogulpkgdir=DIR and for which')
		       line('you can also setup a default')])
      [] ozmake(mogul:nodocurl) then
	 error(kind : TITLE_MOGUL
	       msg  : 'your MOGUL url for documentation is not known'
	       items: [line('either supply it with --moguldocurl=URL')
		       line('or setup a default using:')
		       line('')
		       line('    ozmake --config=set --moguldocurl=URL')
		       line('')
		       line('this is the URL corresponding to the directory')
		       line('specified by --moguldocdir=DIR and for which')
		       line('you can also setup a default')])
      [] ozmake(mogul:nodburl) then
	 error(kind : TITLE_MOGUL
	       msg  : 'your MOGUL base url for your part of the MOGUL database is not known'
	       items: [line('either supply it with --moguldburl=URL')
		       line('or setup a default using:')
		       line('')
		       line('    ozmake --config=set --moguldburl=URL')
		       line('')
		       line('this is the URL corresponding to the directory')
		       line('specified by --moguldbdir=DIR and for which')
		       line('you can also setup a default')])
      end
   end
define
   {Error.registerFormatter ozmake OzMakeErrorFormatter}
end
