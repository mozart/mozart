functor
import
   OS(system stat getDir)
   Admin(manager:Manager)
   Resolve(expand)
   URL(make resolve)
export
   MkDir MkDirForFile ListRegularFiles
define
   proc {CreateDir Path}
      {Manager trace('Creating dir '#Path)}
      if {OS.system 'mkdir "'#Path#'"'}\=0 then
	 raise mogul(cannot_create_dir(Path)) end
      end
   end
   %%
   local
      proc {Loop Prefix Suffix}
	 case Suffix of nil then skip
	 [] H|T then
	    Path = Prefix#'/'#H
	 in
	    try {OS.stat Path _} catch _ then {CreateDir Path} end
	    {Loop Path T}
	 end
      end
   in
      proc {MkDir DIR}
	 {Loop '' {Resolve.expand {URL.resolve './' DIR}}.path}
      end
   end
   %%
   proc {MkDirForFile File}
      U = {URL.make File}
   in
      case {Reverse U.path}
      of _|T then {MkDir {AdjoinAt U path {Reverse T}}}
      else skip end
   end
   %%
   fun {ListRegularFiles Path}
      try
	 for F in {OS.getDir Path} collect:COL do
	    if F\="." andthen F\=".." andthen {OS.stat Path#'/'#F}.type=='reg'
	    then {COL F} end
	 end
      catch _ then nil end
   end
end
