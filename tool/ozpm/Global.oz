functor
import
   Application(getArgs)
   URL(resolve toBase)
   Pickle(load save)
   Path(make) at 'x-ozlib://duchier/sp/Path.ozf'
   Database('class')
export
   Args
   %% file   : file name (no path)
   %% dir    : directory name (no file)
   %% path   : path name (directory and file names)
   %% (none) : Oz data structure
   %% Dft    : default
   %% Pkl    : pickle format
   %% Txt    : text format
   fileLocalDB      : FILELOCALDB
   filePkgDft       : FILEPKGDFT
   fileMftPkl       : FILEMFTPKL
   fileMftTxt       : FILEMFTTXT
   dirPrefix        : DIRPREFIX
   pathLocalDB      : PATHLOCALDB
   localDB          : LOCALDB
   mogulData        : MOGULDB
   packageMogulDB   : PACKAGEMOGULDB
   authorMogulDB    : AUTHORMOGULDB
   background       : BackgroundColor
   getParent        : GetParent
   getLabel         : GetLabel
   
define
   FILEPKGDFT       = 'ozpm.dsc'
   FILELOCALDB      = '.ozpm.info'
   FILEMFTPKL       = 'OZPMMFT.PKL'
   FILEMFTTXT       = 'OZPMMFT.TXT'
   DIRPREFIXDFT     = '~/.oz/'
   MOGUL            = 'http://www.mozart-oz.org/mogul/'
   INFO             = 'ozpm.info'
   MOGULDB          = {ByNeed fun{$}
				 try {Pickle.load {URL.resolve {URL.toBase Args.'mogul'} INFO}}
				 catch _ then nil end
			      end}

      
   Args={Application.getArgs
	 record('action'(single type:atom(install create info check interactive remove help)
			 default:interactive)
		%%
		%% aliases for actions
		%%
		'install'(    alias:['action'#install     '<install>'    #true])
		'create'(     alias:['action'#create      '<create>'     #true])
		'info'(       alias:['action'#info        '<info>'       #true])
		'list'(       alias:['action'#list        '<list>'       #true])
		'check'(      alias:['action'#check       '<check>'      #true])
		'interactive'(alias:['action'#interactive '<interactive>'#true])
		'remove'(     alias:['action'#remove      '<remove>'     #true])
		'help'(       alias:['action'#help        '<help>'       #true])
		%%
		%% corresponding flags
		%%
		'<install>'(    single type:bool)
		'<create>'(     single type:bool)
		'<info>'(       single type:bool)
		'<list>'(       single type:bool)
		'<check>'(      single type:bool)
		'<interactive>'(single type:bool)
		'<remove>'(     single type:bool)
		'<help>'(       single type:bool)
		%%
		%% arguments
		%%
		'in'(    single type:string
			 validate:alt(when('<create>' optional)
				      when(disj('<install>' '<info>' '<remove>') true)
				      when(true false)))
		'prefix'(single type:string)
		'out'(   single type:string
			 validate:alt(when('<create>' true)
				      when(true false)))
		'force'( single type:bool default:false)
		'leave'( single type:bool default:false)
		'update'(single type:bool default:false)
		%%
		%% other parameters
		%%
		'mogul'(single type:string default:MOGUL)
	       )}

   DIRPREFIX      = {{Path.make {CondSelect Args 'prefix' DIRPREFIXDFT}} toBase($)}
   PATHLOCALDB    = {DIRPREFIX resolve(FILELOCALDB $)}
   LOCALDB        = {New Database.'class' init(PATHLOCALDB)}
   PACKAGEMOGULDB = {ByNeed fun{$}
			       {New Database.'class' initFromList(MOGULDB.packages)}
			    end}
   AUTHORMOGULDB  = {ByNeed fun{$}
			       {New Database.'class' initFromList(MOGULDB.authors)}
			    end}

   BackgroundColor = c(240 250 242)

   %% two functions on mogul names
   
   fun{GetParent X}
      %%
      %% returns the parent mogul name
      %% ignore last /, my return a name with an ending /
      %%
      {VirtualString.toAtom
       {Reverse
	{List.dropWhileInd {Reverse {VirtualString.toString X}}
	 fun{$ I C} C\=&/ orelse I==1 end}}}
   end
   fun{GetLabel X}
      %%
      %% returns the last basename of a mogul name
      %% ignore last last /
      %%
      VS={VirtualString.toString
	  {Reverse
	   {List.takeWhileInd {Reverse {VirtualString.toString X}}
	    fun{$ I C} C\=&/ orelse I==1 end}}}
   in
      if {List.last VS}\=&/ then
	 VS
      else
	 {List.take VS {Length VS}-1}
      end
   end	       
   
   
end
