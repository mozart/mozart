functor

export
   MakeVarTable
   GetVarId
   GetVar
   GetVarAllIds

   MakePropTable
   GetPropId
   GetProp
   GetPropAllIds

import
   Misc(counterClass varEq propEq)

define   
   proc {MakeItemTable ItemEq ?Table}
      Key    = {NewName}
      MakeId = {NewName}

      fun {GetItemId ItemTable Item}
	 Entries = {Dictionary.entries ItemTable.Key}
      in
	 {FoldL Entries
	  fun {$ L Id#V}
	     if L == nil then
		if {ItemEq V Item} then Id else nil end
	     else L end
	  end nil}
      end
   in
      Table = table(make:
		       fun {$}
			  table(Key:    {NewDictionary}
				MakeId: {New Misc.counterClass init})
		       end
		    getItemId:
		       fun {$ ItemTable Item}
			  Id = {GetItemId ItemTable Item}
		       in
			  if Id == nil then % id not found
			     NewId = {ItemTable.MakeId next($)}
			  in
			     {Dictionary.put ItemTable.Key NewId Item}
			     NewId
			  else Id end	% id found 
		       end
		    getItem:
		       fun {$ ItemTable Id}
			  {Dictionary.condGet ItemTable.Key Id nil}
		       end
		    getAllIds:
		       fun {$ ItemTable}
			  {Dictionary.keys ItemTable.Key}
		       end
		   )
   end

   table(make:      MakeVarTable
	 getItemId: GetVarId
	 getItem:   GetVar
	 getAllIds: GetVarAllIds) = {MakeItemTable Misc.varEq}

   table(make:      MakePropTable
	 getItemId: GetPropId
	 getItem:   GetProp
	 getAllIds: GetPropAllIds) = {MakeItemTable Misc.propEq}
end
