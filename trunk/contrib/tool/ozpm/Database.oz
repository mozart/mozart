functor
export
   'class' : Database
import
   Path(make) at 'x-ozlib://duchier/sp/Path.ozf'
   Pickle(load save)
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
	 file <- {{{Path.make File} expand($)} toString($)}
	 db   <- {LoadDB @file}
      end
      meth initFromList(L)
	 file <- unit
	 db   <- {NewDictionary}
	 {ForAll L proc{$ R} {Dictionary.put @db
			      {VirtualString.toAtom R.id}
			      R}
		   end}
      end
      meth get(K V) {Dictionary.get @db {ToKey K} V} end
      meth put(K V) {Dictionary.put @db {ToKey K} V} end
      meth condGet(K D V) {Dictionary.condGet @db {ToKey K} D V} end
      meth keys($) {Dictionary.keys @db} end
      meth items($) {Dictionary.items @db} end
      meth entries($) {Dictionary.entries @db} end
      meth remove(K) {Dictionary.remove @db {ToKey K}} end
      meth member(K $) {Dictionary.member @db {ToKey K}} end
      meth save
	 try {Pickle.save {Dictionary.toRecord '1.0' @db} @file}
	 catch _ then skip end
      end
   end
   %%
   fun {ToKey ID}
      if {IsAtom ID} then ID else
	 {VirtualString.toAtom ID}
      end
   end
end
