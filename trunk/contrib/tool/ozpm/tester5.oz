functor
import
   Application Archive System
define
   Args = {Application.getArgs
	   record('zip'( single type:string optional:false)
		  'action'(single type:atom(list extract create) default:list)
		  'list'(char:&l alias:'action'#list)
		  'extract'(char:&x alias:'action'#extract)
		  'create'(char:&c alias:'action'#create)
		  'from'(single type:string)
		  'to'(  single type:string)
		  'home'(single type:string)
		  'files'(multiple type:list(string))
		 )}
   case Args.action
   of 'list' then
      A = {New Archive.'class' init(Args.zip)}
   in
      for X in {A ls($)} do
	 {System.showInfo X}
      end
      {A close}
   [] 'create' then
      if {HasFeature Args 'home'} then
	 {Archive.makeFrom Args.zip Args.files Args.home}
      else
	 {Archive.make Args.zip Args.files}
      end
   [] 'extract' then
      A = {New Archive.'class' init(Args.zip)}
   in
      {A extract(Args.'from' Args.'to')}
      {A close}
   end
   {Application.exit 0}
end
