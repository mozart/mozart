functor
export
   'class' : Database
import
   Path(make) at 'x-oz://duchier/sp/Path.ozf'
define
   fun lazy {LoadDB F}
      try {Record.toDictionary {Pickle.load F}}
      catch _ then {NewDictionary} end
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
	 try {Pickle.save {Dictionary.toRecord o @db} @file}
	 catch _ then skip end
      end
   end
end
