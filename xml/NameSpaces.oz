functor
export
   NewNameSpaceCollection
   NewNameSpacePrefixMap
   ProcessElement
   ProcessName
prepare
   
   %% ================================================================
   %% an XML name is represented by a record name(ns:NS id:ID key:KEY)
   %% where NS is a symbol representing the namespace URI, ID is a
   %% symbol representing the local part of the name, and KEY is an
   %% Oz name which uniquely identifies the XML name and can be used
   %% as an index into e.g. dictionaries.
   %%
   %% {Split +Name ?NameSpacePrefix ?LocalPart}
   %%     Name is an atom representing the written identifier.  This
   %% is analyzed into its 2 constituent parts: the namespace prefix
   %% and the local part, which are returned as atoms.  If Names does
   %% not contain a prefix, unit is returned for it to indicate that
   %% it has the default prefix.
   %% ================================================================

   StringToken = String.token
   proc {Split Name NameSpacePrefix LocalPart}
      Prefix Suffix
   in
      {StringToken {AtomToString Name} &: Prefix Suffix}
      if Suffix==nil then
	 NameSpacePrefix = unit
	 LocalPart       = {StringToAtom Prefix}
      else
	 NameSpacePrefix = {StringToAtom Prefix}
	 LocalPart       = {StringToAtom Suffix}
      end
   end

   %% ================================================================
   %% ADT for an XML namespace.  An XML namespace is identified by its
   %% URI.  The ADT exports the function `get' which takes as argument
   %% an atom representing the local part of the name and returns the
   %% corresponding XML name in this namespace.  KeyURI is an atom.
   %% the exported `toList' function returns the list of all XML names
   %% currently in the namespace.
   %% ================================================================

   DictItems = Dictionary.items
   fun {NewNameSpace KeyURI}
      Table = {NewDictionary}
      fun {Get LocalPart}
	 Name = {CondSelect Table LocalPart unit}
      in
	 if Name==unit then
	    Name=name(
		    ns : KeyURI
		    id : LocalPart
		    key: {NewName})
	 in
	    Table.LocalPart := Name
	    Name
	 else Name end
      end
      fun {ToList} {DictItems Table} end
   in
      namespace(
	 uri    : KeyURI
	 get    : Get
	 toList : ToList)
   end

   %% ================================================================
   %% ADT for the XML namespaces of a document.  We call this a name
   %% space collection.  Its purpose is to map namespace URIs to
   %% namespace ADTs (see above).  It exports a `get' function which
   %% takes an atom KeyURI as argument and returns the corresponding
   %% namespace ADT, creating it if necessary.  The exported `toList'
   %% function returns a list of all namespaces currently in the
   %% collection.
   %% ================================================================

   fun {NewNameSpaceCollection}
      Table = {NewDictionary}
      fun {Get KeyURI}
	 NameSpace = {CondSelect Table KeyURI unit}
      in
	 if NameSpace==unit then
	    NameSpace = {NewNameSpace KeyURI}
	 in
	    Table.KeyURI := NameSpace
	    NameSpace
	 else NameSpace end
      end
      fun {ToList} {DictItems Table} end
   in
      namespaceCollection(
	 get    : Get
	 toList : ToList)
   end

   %% ================================================================
   %% ADT for a namespace prefix map, i.e. a mapping from prefixes to
   %% namespace URIs.  It exports `get' to map a prefix to its URI,
   %% `condGet', `put' to install a mapping from a specific prefix to
   %% a specific uri, and `clone' to obtain a copy of the prefix map.
   %% the latter is useful for installing new `local' namespace
   %% declarations.
   %% ================================================================

   fun {NewNameSpacePrefixMap}
      {NameSpacePrefixMapWrap {NewDictionary}}
   end
   DictionaryClone = Dictionary.clone
   fun {NameSpacePrefixMapWrap Table}
      fun {Get Prefix}
	 %% if a prefix is unknown, we just use it as its own URI
	 {CondSelect Table Prefix Prefix}
      end
      fun {CondGet Prefix Default}
	 {CondSelect Table Prefix Default}
      end
      proc {Put Prefix KeyURI}
	 Table.Prefix := KeyURI
      end
      fun {Clone}
	 {NameSpacePrefixMapWrap
	  {DictionaryClone Table}}
      end
   in
      namespacePrefixMap(
	 get     : Get
	 condGet : CondGet
	 put     : Put
	 clone   : Clone)
   end

   %% ================================================================
   %% {ProcessElement +Tag1 +Alist1 +Map1
   %%                 ?Tag2 ?Alist2 ?Map2
   %%                 +Collection}
   %%
   %%     Tag1 is an atom representing the tag as written, Alist1 is
   %% the list of attributes where each attribute identifier is also
   %% an atom representing it as written, Map1 is the current prefix
   %% map mapping namespace prefixes to namespace URIs, Collection is
   %% the namespace collection mapping namespace URIs to ADTs.
   %%
   %% The outputs: Tag2 is an XML name for the element identifier,
   %% Alist2 is a list of attributes where each attribute identifier
   %% is now an XML name, Map2 is a new prefix map which extends Map1
   %% with the local namespace declarations that appeared in the
   %% attributes (note that these declarations are not included in
   %% Alist2).
   %%
   %% {ProcessName +Tag1 +Map +Collection ?Tag2}
   %%     Tag1 is an atom representing an XML name as written, and
   %% Tag2 is the corresponding XML name representation.
   %% ================================================================

   ProcessElement
   ProcessName
   local
      fun {PreProcessName Name}
	 NameSpacePrefix LocalPart
      in
	 {Split Name NameSpacePrefix LocalPart}
	 NameSpacePrefix|LocalPart
      end
      fun {PreProcessAttr Name|Value}
	 {PreProcessName Name}|Value
      end
      fun {PreProcessAlist Alist}
	 {Map Alist PreProcessAttr}
      end
      XMLNS =
      o(
	 'xmlns' : unit
	 'Xmlns' : unit
	 'xMlns' : unit
	 'xmLns' : unit
	 'xmlNs' : unit
	 'xmlnS' : unit
	 'XMlns' : unit
	 'XmLns' : unit
	 'XmlNs' : unit
	 'XmlnS' : unit
	 'xMLns' : unit
	 'xMlNs' : unit
	 'xMlnS' : unit
	 'xmLNs' : unit
	 'xmLnS' : unit
	 'xmlNS' : unit
	 'XMLns' : unit
	 'XMlNs' : unit
	 'XMlnS' : unit
	 'XmLNs' : unit
	 'XmLnS' : unit
	 'XmlNS' : unit
	 'xMLNs' : unit
	 'xMlNS' : unit
	 'xMLnS' : unit
	 'xmLNS' : unit
	 'XMLNs' : unit
	 'XmLNS' : unit
	 'XMlNS' : unit
	 'XMLnS' : unit
	 'xMLNS' : unit
	 'XMLNS' : unit
	 )
      fun {IsNameSpaceDeclarator (NameSpacePrefix|LocalPart)|_}
	 if NameSpacePrefix==unit
	 then {HasFeature XMLNS LocalPart}
	 else {HasFeature XMLNS NameSpacePrefix} end
      end
      fun {HasNameSpaceDeclaration PreAlist}
	 {Some PreAlist IsNameSpaceDeclarator}
      end
      proc {DeclareNameSpaces PreAlist Map}
	 for Attr in PreAlist do
	    if {IsNameSpaceDeclarator Attr} then
	       case Attr of (NameSpacePrefix|LocalPart)|Value then
		  {Map.put
		   if NameSpacePrefix==unit
		   then unit
		   else LocalPart end
		   %% note that the value is assumed to be an atom
		   Value}
	       end
	    end
	 end
      end
      fun {PostProcessName NameSpacePrefix|LocalPart Map Collection}
	 {{Collection.get {Map.get NameSpacePrefix}}.get LocalPart}
      end
      fun {PostProcessAlist PreAlist Map Collection}
	 case PreAlist
	 of nil then nil
	 [] Attr|PreAlist then
	    if {IsNameSpaceDeclarator Attr} then
	       {PostProcessAlist PreAlist Map Collection}
	    elsecase Attr of PreName|Value then
	       ({PostProcessName PreName  Map Collection}|Value)|
	       {PostProcessAlist PreAlist Map Collection}
	    end
	 end
      end
   in
      proc {!ProcessElement Tag1 Alist1 Map1 Tag2 Alist2 Map2 Collection}
	 PreTag   = {PreProcessName Tag1}
	 PreAlist = {PreProcessAlist Alist1}
      in
	 if {HasNameSpaceDeclaration PreAlist} then
	    Map2 = {Map1.clone}
	    {DeclareNameSpaces PreAlist Map2}
	 else
	    Map2 = Map1
	 end
	 Tag2   = {PostProcessName  PreTag   Map2 Collection}
	 Alist2 = {PostProcessAlist PreAlist Map2 Collection}
      end

      fun {!ProcessName Name Map Collection}
	 {PostProcessName
	  {PreProcessName Name} Map Collection}
      end
   end
end
