functor
export
   'class' : Database
import
   Path(make) at 'x-oz://duchier/sp/Path.ozf'
define
   fun lazy {LoadDB F}
      D = {NewDictionary}
      L = try {Pickle.load F}
	  catch _ then nil end
   in
      for X in L do {Dictionary.put D X.id X} end
      D
   end
   %%
   class Database
      prop final
      attr db:unit file:unit
      meth init(File)
	 file <- {{Path.make File} expand($)}
	 db   <- {LoadDB}
      end
      meth get(K V) {Dictionary.get @db K V} end
      meth put(K V) {Dictionary.put @db K V} end
      meth condGet(K D V) {Dictionary.condGet @db K D V} end
      meth keys($) {Dictionary.keys @db} end
      meth items($) {Dictionary.items @db} end
      meth entries($) {Dictionary.entries @db} end
      meth remove(K) {Dictionary.remove @db K} end
      meth member(K $) {Dictionary.member @db K} end
      meth save
	 try {Pickle.save Database,items($) @file}
	 catch _ then skip end
      end
   end
end
