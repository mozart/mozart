functor
import
   Application(getArgs)
   FileUtils(expand     : Expand
	     createPath : CreatePath
	     dirname    : Dirname
	     addToPath  : AddToPath)
   URL(resolve toBase)
   Pickle(load save)
   
export
   Args
   %% file   : file name (no path)
   %% dir    : directory name (no file)
   %% path   : path name (directory and file names)
   %% (none) : Oz data structure
   fileLocalDB      : FILELOCALDB
   filePkgDft       : FILEPKGDFT
   fileMftPkl       : FILEMFTPKL
   fileMftTxt       : FILEMFTTXT
   dirPrefixDft     : DIRPREFIXDFT
   dirPrefix        : DIRPREFIX
   pathLocalDB      : PATHLOCALDB
   localDB          : LOCALDB
   mogulDB          : MOGULDB
   readDB           : READDB
   
define
   FILEPKGDFT       = 'ozpm.dsc'
   FILELOCALDB      = '.ozpm.info'
   FILEMFTPKL       = 'OZPMMFT.PKL'
   FILEMFTTXT       = 'OZPMMFT.TXT'
   DIRPREFIXDFT     = {Expand '~/.oz/'}
   MOGUL            = 'http://www.mozart-oz.org/mogul/' %"./"
   INFO             = 'ozpm.info'
   MOGULDB          = {ByNeed fun{$}
				 try {Pickle.load {URL.resolve {URL.toBase MOGUL} INFO}}
				 catch _ then nil end
			      end}

      
   Args={Application.getArgs
	 record('action'(single type:atom(install create info check interactive remove help) default:interactive)
		%%
		%% aliases for actions
		%%
		'install'(alias:['action'#install '<install>'#true])
		'create'( alias:['action'#create  '<create>' #true])
		'info'(   alias:['action'#info    '<info>'   #true])
		'list'(   alias:['action'#list    '<list>'   #true])
		'check'(  alias:['action'#check   '<check>'  #true])
		'interactive'(alias:['action'#interactive '<interactive>'#true])
		'remove'( alias:['action'#remove  '<remove>' #true])
		'help'(   alias:['action'#help    '<help>'   #true])
		%%
		%% corresponding flags
		%%
		'<install>'(single type:bool)
		'<create>'( single type:bool)
		'<info>'(   single type:bool)
		'<list>'(   single type:bool)
		'<check>'(  single type:bool)
		'<interactive>'(single type:bool)
		'<remove>'( single type:bool)
		'<help>'(   single type:bool)
		%%
		%% arguments
		%%
		'in'(    single type:string
			 validate:alt(when('<create>' optional)
				      when(disj('<install>' '<info>' '<remove>') true)
				      when(true false)))
		'prefix'(single type:string optional:true)
		'out'(   single type:string
			 validate:alt(when('<create>' true)
				      when(true false)))
		'force'( single type:bool default:false)
		'update'(single type:bool default:false)
	       )}

   DIRPREFIX   = {CondSelect Args prefix DIRPREFIXDFT}
   PATHLOCALDB = {AddToPath DIRPREFIX FILELOCALDB}
   READDB      = fun {$}
		    Ret
		 in
		    try
		       Ret={Pickle.load PATHLOCALDB}
		    catch error(url(load ...) ...) then
		       {CreatePath {Dirname PATHLOCALDB}}
		       {Pickle.save nil PATHLOCALDB}
		       Ret=nil
		    end
		    Ret
		 end
   LOCALDB     = {ByNeed READDB}

end
