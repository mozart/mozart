functor
export
   NewPrefixMap
   ProcessElement
   ProcessName
   Fast
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
   %% and the local part, which are returned as atoms.  If Name does
   %% not contain a prefix, unit is returned for it to indicate that
   %% it has the default prefix.
   %% ================================================================

   StringToken = String.token
   proc {Split Name NameSpacePrefix LocalPart}
      Prefix Suffix
   in
      {StringToken Name &: Prefix Suffix}
      if Suffix==nil then
	 NameSpacePrefix = unit
	 LocalPart       = {StringToAtom Prefix}
      else
	 NameSpacePrefix = {StringToAtom Prefix}
	 LocalPart       = {StringToAtom Suffix}
      end
   end

   %% ================================================================
   %% ADT for a namespace prefix map, i.e. a mapping from prefixes to
   %% namespaces.  A namespace prefix map M exports 3 methods:
   %%
   %% {M.declarePrefix +Prefix +URI}
   %%     registers that Prefix (a symbol) refers to the namespace
   %% identified by URI (also a symbol).
   %%
   %% {M.intern +Prefix +LocalPart ?Name}
   %%     returns the unique representation of the XML name Name with
   %% namespace prefix Prefix (a symbol) and local part LocalPart (a
   %% symbol).
   %%
   %% {M.clone ?M2}
   %%     returns a clone of the prefix map.  The namespaces are of
   %% course shared, but the mapping from prefix to namespace can be
   %% modified independently.
   %% ================================================================

   fun {NewPrefixMap}
      {PrefixMapWrap {NewDictionary} {NewDictionary}}
   end
   DictClone = Dictionary.clone
   fun {PrefixMapWrap PrefixMap Collection}
      proc {DeclarePrefix Prefix URI}
	 NS={CondSelect Collection URI false}
      in
	 if NS==false then
	    NS=namespace(
		  table  : {NewDictionary}
		  prefix : Prefix
		  uri    : URI)
	 in
	    Collection.URI   := NS
	    PrefixMap.Prefix := NS
	 else
	    PrefixMap.Prefix := NS
	 end
      end
      fun {Intern Prefix LocalPart}
	 NS = {CondSelect PrefixMap Prefix false}
      in
	 if NS==false then
	    %% if the Prefix is unknown, we declare it to identify
	    %% a namespace whose URI is also Prefix
	    {DeclarePrefix Prefix Prefix}
	    {Intern        Prefix LocalPart}
	 else
	    NAM = {CondSelect NS.table LocalPart false}
	 in
	    if NAM==false then
	       NAM = name(
			ns  : NS.uri
			id  : LocalPart
			key : {NewName})
	    in
	       NS.table.LocalPart := NAM
	       NAM
	    else NAM end
	 end
      end
      fun {Clone}
	 {PrefixMapWrap {DictClone PrefixMap} Collection}
      end
   in
      namespacePrefixMap(
	 declarePrefix : DeclarePrefix
	 intern        : Intern
	 clone         : Clone)
   end

   %% ================================================================
   %% {ProcessElement +Tag1 +Alist1 +Map1 ?Tag2 ?Alist2 ?Map2}
   %%
   %%     Tag1 is an atom representing the tag as written, Alist1 is
   %% the list of attributes where each attribute identifier is also
   %% an atom representing it as written, Map1 is the current prefix
   %% map.
   %%
   %% The outputs: Tag2 is an XML name for the element identifier,
   %% Alist2 is a list of attributes where each attribute identifier
   %% is now an XML name, Map2 is a new prefix map which extends Map1
   %% with the local namespace declarations that appeared in the
   %% attributes (note that these declarations are not included in
   %% Alist2).
   %%
   %% {ProcessName +Tag1 +Map ?Tag2}
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
      proc {DeclareNameSpaces PreAlist PrefixMap}
	 for Attr in PreAlist do
	    if {IsNameSpaceDeclarator Attr} then
	       case Attr of (NameSpacePrefix|LocalPart)|Value then
		  {PrefixMap.declarePrefix
		   if NameSpacePrefix==unit
		   then unit
		   else LocalPart end
		   %% note that the value is assumed to be an atom
		   Value}
	       end
	    end
	 end
      end
      fun {PostProcessName NameSpacePrefix|LocalPart PrefixMap}
	 {PrefixMap.intern NameSpacePrefix LocalPart}
      end
      fun {PostProcessAlist PreAlist PrefixMap}
	 case PreAlist
	 of nil then nil
	 [] Attr|PreAlist then
	    if {IsNameSpaceDeclarator Attr} then
	       {PostProcessAlist PreAlist PrefixMap}
	    elsecase Attr of PreName|Value then
	       ({PostProcessName PreName  PrefixMap}|Value)|
	       {PostProcessAlist PreAlist PrefixMap}
	    end
	 end
      end
   in
      proc {!ProcessElement Tag1 Alist1 Map1 Tag2 Alist2 Map2}
	 PreTag   = {PreProcessName Tag1}
	 PreAlist = {PreProcessAlist Alist1}
      in
	 if {HasNameSpaceDeclaration PreAlist} then
	    Map2 = {Map1.clone}
	    {DeclareNameSpaces PreAlist Map2}
	 else
	    Map2 = Map1
	 end
	 Tag2   = {PostProcessName  PreTag   Map2}
	 Alist2 = {PostProcessAlist PreAlist Map2}
      end

      fun {!ProcessName Name PrefixMap}
	 {PostProcessName {PreProcessName Name} PrefixMap}
      end
   end

   %% Here we define a `fast' version that simply does not
   %% handle namespaces at all.  This is intended for very
   %% simple applications that don't want to pay the price

   local
      proc {FastProcessElement Name Alist Map1 Tag2 Alist2 Map2}
	 Map2 = unit
	 {StringToAtom Name Tag2}
	 {FastProcessAlist Alist Alist2}
      end
      fun {FastProcessAlist L}
	 case L
	 of nil then nil
	 [] (Name|Value)|L then
	    ({StringToAtom Name}|Value)|{FastProcessAlist L}
	 end
      end
      fun {FastProcessName S _} {StringToAtom S} end
      fun {FastNewPrefixMap} unit end
   in
      Fast = fast(
		newPrefixMap   : FastNewPrefixMap
		processElement : FastProcessElement
		processName    : FastProcessName)
   end
end
