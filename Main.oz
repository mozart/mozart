functor
import
   Application(getArgs exit)
   System(showError)
   Admin(manager:Manager)
   Except('raise':Raise)
   Property(put)
define
   {Property.put errors errors(width:100 depth:100)}
   Args
   try
      Args = {Application.getArgs
	      list('verbose'(      type:bool char:&v)
		   'quiet'(        type:bool char:&q)
		   %%
		   'rood-id'(      type:string)
		   'root-url'(     type:string)
		   %%
		   'open-db'(      type:string)
		   'save-db'(      type:bool)
		   'close-db'(     type:bool)
		   'export-db'(    type:string)
		   'import-db'(    type:string)
		   %%
		   'update-info'(  type:bool)
		   'print-db'(     type:bool)
		   %%
		   'print-reports'(type:bool)
		   %%
		   'wget'(         type:string)
		   'update-pub'(   type:bool)
		   %%
		   'css'(          type:string)
		   'update-html'(  type:bool)
		   %%
		   'mogul-dir'(    type:string)
		   'mogul-url'(    type:string)
		   %%
		   'update-provided'(type:bool)
		   'print-provided'(type:bool)
		   %%
		   'update-categories'(type:bool)
		   'categories-url'(type:string)
		   'update-categories-html'(type:bool)
		   'update-package-list'(type:bool)
		   'update-package-list-html'(type:bool)
		   %%
		   'update-author-list'(type:bool)
		   'update-author-list-html'(type:bool)
		   %%
		   'update-ozpm-info'(type:bool)
		   %%
		   'mogul-top'(type:string)
		   %%
		   'ignore-id'(type:list(string) default:nil)
		   'ignore-url'(type:list(string) default:nil)
		   %%
		   'update-ozmake'(type:bool)
		  )}
   catch error(ap(usage Msg) ...) then
      {System.showError Msg}
      {Application.exit 1}
   end

   %% first check that all arguments are ok
   
   for A in Args do
      case A of _#_ then skip else
	 {Raise mogul(argument_not_understood(A))}
      end
   end

   %% actually process them
   
   for A in Args do
      case A of K#V then
	 {Manager trace('Processing option --'#K)}
	 try
	    {Manager K(V)}
	 catch mogul(...)=E then
	    {Manager addReport(processing_option(K V) E)}
	 end
      end
   end

   {Manager exit}
end
