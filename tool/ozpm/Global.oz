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
   ozpminfo         : OZPMINFO
   ozpmmanifest     : OZPMMANIFEST
   ozpmmanifesttext : OZPMMANIFESTTEXT
   ozpmpkg          : OZPMPKG
   mogul            : MOGUL
   info             : INFO
   prefix           : PREFIX
   ozpmInfo         : OzpmInfo
   ozpminfofile     : OzpmInfoFile
define
   
   OZPMINFO         = {Expand ".ozpm.info"}
   OZPMMANIFEST     = 'OZPMMFT.PKL'
   OZPMMANIFESTTEXT = 'OZPMMFT.TXT'
   OZPMPKG          = {Expand '~/.oz/'}
   MOGUL            = 'http://www.mozart-oz.org/mogul/' %"./"
   INFO             = 'ozpm.info'
   MOGULINFO        = try {Pickle.load {URL.resolve {URL.toBase MOGUL} INFO}}
		      catch _ then nil end

      
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

   PREFIX       = {CondSelect Args prefix OZPMPKG}
   OzpmInfoFile = {AddToPath PREFIX OZPMINFO}
   OzpmInfo     = {ByNeed
		   fun {$}
		      Ret
		   in
		      try
			 Ret={Pickle.load OzpmInfoFile}
		      catch error(url(load ...) ...) then
			 {CreatePath {Dirname OzpmInfoFile}}
			 {Pickle.save nil OzpmInfoFile}
			 Ret=nil
		      end
		      Ret
		   end}
end
