functor
prepare
   ToTuple = List.toTuple
   VS2A = VirtualString.toAtom
   VS2S = VirtualString.toString
   VSLength = VirtualString.length

   %%-----------------------------------------------------------------
   %% functions to nicely format headers
   %%-----------------------------------------------------------------
   
   fun {Spaces N}
      if N>0 then & |{Spaces N-1} else nil end
   end

   INDENT = 16

   fun {Header Label Value}
      N = {VSLength Label}
   in
      Label#':'#{Spaces INDENT-N-1}#Value#'\n'
   end

   %% omit header if value is unit
   
   fun {MaybeHeader Label Value}
      if Value==unit then nil else {Header Label Value} end
   end

   %% format a header with possibly embedded newlines - we have to be
   %% careful that we produce continuation lines appropriate for header
   
   fun {ExtendedHeader Label Value}
      {Header Label {ExtendedValue {VS2S Value}}}
   end

   fun {ExtendedValue S}
      case S of nil then nil
      [] H|T then
	 if H==&\n
	 then {Append &\n|{Spaces INDENT} {ExtendedValue T}}
	 else H|{ExtendedValue T} end
      end
   end

   fun {MaybeExtendedHeader Label Value}
      if Value==unit then nil else
	 {ExtendedHeader Label Value}
      end
   end

   %%-----------------------------------------------------------------
   %% compare instances in order to select a representative. this is
   %% the instance which we will use to create the advertising filler
   %% for MOGUL.
   %%-----------------------------------------------------------------

   fun {ComparePlatforms I1 I2}
      P1 = if I1==unit then unit else {I1 get(platform $)} end
      P2 = if I2==unit then unit else {I2 get(platform $)} end
   in
      if P1==P2 then '='
      elseif P1==source then '<'
      elseif P2==source then '>'
      else '?' end
   end

   fun {NewUniqQueue}
      Q={NewQueue}
      T={NewDictionary}
      proc {Put K}
	 if {HasFeature T K} then skip else
	    T.K := unit
	    {Q.put K}
	 end
      end
      fun {ToList} {Q.toList} end
   in
      uniqQueue(put:Put toList:ToList)
   end

import
   Utils at 'Utils.ozf'
define

   fun {CompareVersions I1 I2}
      V1 = if I1==unit then unit else {I1 get(version $)} end
      V2 = if I2==unit then unit else {I2 get(version $)} end
   in
      if V1==V2 then '='
      elseif V1==unit then '>'
      elseif V2==unit then '<'
      elsecase {Utils.versionCompare V1 V2}
      of eq then '='
      [] lt then '<'
      [] gt then '>'
      end
   end

   fun {CompareDates I1 I2}
      D1 = if I1==unit then unit else {I1 get(released $)} end
      D2 = if I2==unit then unit else {I2 get(released $)} end
   in
      if D1==D2 then '='
      elseif D1==unit then '<'
      elseif D2==unit then '>'
      else
	 if {Date.less
	     {Utils.dateParse D1}
	     {Utils.dateParse D2}}
	 then '<' else '>' end
      end
   end

   fun {BetterRepresentative I1 I2}
      case {ComparePlatforms I1 I2}
      of '<' then true
      [] '>' then false
      elsecase {CompareVersions I1 I2}
      of '<' then true
      [] '>' then false
      elsecase {CompareDates I1 I2}
      of '>' then true
      else false
      end
   end

   local
      fun {Loop L B}
	 case L of nil then B
	 [] H|T then
	    {Loop T
	     if {BetterRepresentative B H}
	     then B else H end
	     T}
	 end
      end
   in
      fun {ChooseBestRepresentative L}
	 {Loop L unit}
      end
   end

   %%-----------------------------------------------------------------
   %% MogulContact
   %%
   %% represents the info for a contact (typically then used to
   %% describe an author)
   %%-----------------------------------------------------------------

   class MogulContact
      attr
	 manager        : unit
	 mogul          : unit
	 name           : unit
	 name_for_index : unit
	 email          : unit
	 www            : unit

      meth toVS($)
	 {Header type contact}#
	 {Header id @mogul}#
	 {Header name @name}#
	 {MaybeHeader 'name-for-index' @name_for_index}#
	 {MaybeHeader email @email}#
	 {MaybeHeader www @www}
      end

      meth init(R Manager)
	 manager        <- Manager
	 mogul          <- R.mogul
	 name           <- R.name
	 name_for_index <- {CondSelect R 'name-for-index' unit}
	 email          <- {CondSelect R email            unit}
	 www            <- {CondSelect R www              unit}
      end
   end

   %%-----------------------------------------------------------------
   %% MogulPackageBase
   %%
   %% holds functionality that is common to both packages and
   %% instances - mostly this commonality is due to the fact that we
   %% need to support old and new regime - in the old regime, there
   %% were no (multiple) instances, only the (single) package itself.
   %%-----------------------------------------------------------------

   class MogulPackageBase

      attr
	 Changed:unit
	 old_value
	 manager

      meth init(R Manager)
	 old_value <- R
	 manager <- Manager
      end

      meth get(A $) @A end
      meth set(A V)
	 if @A\=V then
	    A<-V
	    Changed.A := unit
	 end
      end

      meth getChanged($)
	 {Dictionary.keys @Changed}
      end

      meth to_pkgbasename($)
	 {Utils.mogulToFilename @mogul}#
	 if @format  ==unit then '__1.2.5'  else '__'#@format   end#
	 if @platform==unit then '__source' else '__'#@platform end#
	 if @version ==unit then nil        else '__'#@version  end
      end

      meth to_pkgname($)
	 {self to_pkgbasename($)}#'.pkg'
      end

      meth to_pkgurl($)
	 {Path.resolve
	  {@manager get_mogulpkgurl($)}
	  {self to_pkgname($)}}
      end
      
      meth url_pkg_list($)
	 if @tar==nil then
	    [{self to_pkgurl($)}]
	 else
	    %% this is for _very_ oldstyle tar-based packages etc...
	    {Map @tar
	     fun {$ EXT}
		{Path.resolve
		 {@manager get_mogulpkgurl($)}
		 {Utils.ToFilename @id}#'.'#EXT}
	     end}
	 end
      end
      
      meth url_doc($)
	 if @doc==unit then unit else
	    {Path.resolve
	     {@manager get_moguldocurl($)}
	     {Utils.mogulToFilename @id}#'/'#@doc}
	 end
      end

      meth toProvides(R Uri Uniq)
	 if {HasFeature R provides} then
	    for E in R.provides do A={VS2A E} in
	       %% this sucks!!! it needs to be resolved
	       %% by the makefile object, not approximated here
	       %% in this fashion.
	       if {Member A {CondSelect R bin nil}} then
		  {Uniq.put A}
	       else
		  {Uniq.put {Path.resolveAtom Uri A}}
	       end
	    end
	 else
	    for E in {CondSelect R bin nil} do
	       {Uniq.put {VS2A E}}
	    end
	    for E in {CondSelect R lib nil} do
	       {Uniq.put {Path.resolveAtom Uri E}}
	    end
	 end
	 for D#M in {Record.toListInd {CondSelect R submakefiles o}} do
	    {self toProvides(M {Path.resolve Uri D} Uniq)}
	 end
      end

      %% 
      meth toRecord($)
	 L={self getChanged($)}
      in
	 if L==nil then @old_value else
	    Table = {Record.toDictionary @old_table}
	 in
	    for A in L do
	       Table.A := {self get(A $)}
	    end
	    {Dictionary.toRecord {Label @old_value} Table}
	 end
      end
   end

   %%-----------------------------------------------------------------
   %% MogulInstance
   %%
   %% Each package maybe released as a number of instances.  Obviously,
   %% we could have an instance for each version (if we want to preserve
   %% publication history - sometimes convenient when a newer release
   %% turns out to have a bug that makes it unsuable on some systems
   %% or for certain applications).  We may also supply an instance
   %% in the old 1.2.5 format and one in the new 1.3.0 format.  We may
   %% want to additionally supply platform-specific binary versions of
   %% the package, e.g. a Windows versions so that Windows users may
   %% enjoy our wonderful native functors eventhough they probably do
   %% not have a cygwin installation to compile them.
   %%
   %% a instance object has all the attributes that used to be in a
   %% package
   %%-----------------------------------------------------------------

   class MogulInstance from MogulPackageBase
      attr
	 mogul      : unit
	 uri        : unit
	 format     : unit
	 platform   : unit
	 version    : unit
	 released   : unit
	 author     : nil
	 blurb      : unit
	 tar        : nil
	 provides   : nil
	 requires   : nil
	 info_html  : unit
	 info_text  : unit

      meth init(R Manager)
	 MogulPackageBase,init(R Manager)
	 mogul     <- R.mogul
	 uri       <- {CondSelect R uri          nil}
	 format    <- {CondSelect R format      unit}
	 platform  <- {CondSelect R platform    unit}
	 version   <- {CondSelect R version     unit}
	 released  <- {CondSelect R released    unit}
	 author    <- {CondSelect R author       nil}
	 blurb     <- {CondSelect R blurb       unit}
	 tar       <- {CondSelect R tar          nil}
	 provides  <- {CondSelect R provides     nil}
	 requires  <- {CondSelect R requires     nil}
	 info_html <- {CondSelect R 'info-html' unit}
	 info_text <- {CondSelect R 'info-text' unit}
      end

      %% we are going to store the MOGUL information about an instance
      %% in a file SECTION-PACKAGE__FORMAT__PLATFORM__VERSION.mogul
      %% the basename is the id

      meth get_id($)
	 {VS2A {self toInstanceName($)}}
      end

      meth toInstanceName($)
	 %% we don't allow 2 instances with the same characteristics
	 %% if they only differ in "time", then the later one simply
	 %% overwrites the earlier one
	 {self to_pkgbasename($)}
      end

      meth toInstanceFilename($)
	 {self toInstanceName($)}#'.mogul'
      end

      %% toRepresentative($) is invoked by the package to produce the
      %% advertising filler that it needs.

      meth toRepresentativeVS($)
	 {MaybeHeader format @format}#
	 {MaybeHeader platform @platform}#
	 {MaybeHeader version @version}#
	 {MaybeHeader released @released}#
	 {ToTuple '#'
	  {Map @author fun {$ A} {Header author A} end}}#
	 {MaybeExtendedHeader blurb @blurb}#
	 {ToTuple '#'
	  {Map @categories fun {$ C} {Header category C} end}}#
	 {ToTuple '#'
	  {Map {self url_pkg_list($)}
	   fun {$ U} {Header 'url-pkg' U} end}}#
	 {MaybeHeader 'url-doc' {self url_doc($)}}#
	 {ToTuple '#'
	  {Map @requires fun {$ R} {Header requires R} end}}#
	 {ToTuple '#'
	  {Map @provides fun {$ P} {Header provides P} end}}#
	 if @info_html\=unit then
	    {Header 'content-type' 'text/html'}#
	    '\n'#
	    @info_html
	 else
	    {Header 'content-type' 'text/plain'}#
	    '\n'#
	    @info_text
	 end
      end

      %% toVS($) is almost as the above, except it additionally
      %% supplies its own type and id

      meth toVS($)
	 {Header type instance}#
	 {Header id {self get_id($)}}#
	 {self toRepresentativeVS($)}
      end
   end

   %%-----------------------------------------------------------------
   %% MogulPackage
   %%
   %% Previously, a package was just one thing - so all the info
   %% concerning the unique corresponding instance lived in the
   %% package object itself.  In the new regime, there may be
   %% many instances for one package: these instances are stored
   %% in the "instances" attribute.
   %%
   %% Note that we need to support simultaneously the old and the
   %% new regime.  In the old regime, there are no instances and
   %% all the info concerning the unique instance is stored in
   %% the package itself.
   %%
   %% In the new regime, we still need information that we will
   %% use to advertise the package.  The idea is to choose a
   %% representative from among the instances and to borrow all
   %% the necessary information from it.
   %%-----------------------------------------------------------------

   class MogulPackage from MogulPackageBase
      attr
	 mogul     : unit
	 instances : nil
	 representative : unit

      meth init(R Manager)
	 MogulPackageBase,init(R Manager)
	 if {HasFeature R instances} then
	    mogul <- R.mogul
	    instances <- {Map R.instance
			  fun {$ I}
			     {New MogulInstance init(I Manager)}
			  end}
	 else
	    %% this is am old-style package - we just turn it into
	    %% an instance
	    mogul <- R.mogul
	    instances <- [{New MogulInstance init(R Manager)}]
	 end
      end

      meth choose_representative($)
	 if @representative==unit andthen @instance\=nil then
	    representative <- {ChooseBestRepresentative @instances}
	 end
      end

      meth toVS($)
	 {self choose_representative($)}
	 {Header type package}#
	 {Header id @mogul}#
	 {ToTuple '#'
	  {Map @instances
	   fun {$ I} {Header instance {I toInstanceName($)}} end}}
	 {@representative toRepresentativeVS($)}
      end

      meth SetRepresentative(R)
	 %% R is expected to be a member of @instances
	 %% we set the format+platform only when there is only one instance
	 if {Length @instances}<2 then
	    format   <- {R get(format    $)}
	    platform <- {R get(platform  $)}
	 else
	    format   <- unit
	    platform <- unit
	 end
	 version    <- {R get(version    $)}
	 author     <- {R get(author     $)}
	 blurb      <- {R get(blurb      $)}
	 categories <- {R get(categories $)}
	 provides   <- {R get(provides   $)}
	 requires   <- {R get(requires   $)}
	 info_html  <- {R get(info_html  $)}
	 info_text  <- {R get(info_text  $)}
      end

      meth ChooseRepresentative($)
	 %% if a representative is not set, we'll pick the most recent
	 %% source version, else the most recent version in any form,
	 %% else the most recent instance
	 representative <-
	 {ChooseBestRepresentative @instances}
      end
   end

   class MogulDatabase
      attr
	 DB
      %% we initialize the database object using the database
      %% read from the filesystem
      meth init(D Manager)
	 DB <- {NewDictionary}
	 for Key#Val in {Dictionary.keys D} do
	    case Val.type
	    of 'contact' then @DB.Key := {New MogulContact init(Val Manager)}
	    [] 'package' then @DB.Key := {New MogulPackage init(Val Manager)}
	    end
	 end
      end

      meth toTable($)
	 Table = {NewDictionary}
      in
	 for Key#Val in {Dictionary.keys @DB} do
	    Table.Key := {Val toRecord($)}
	 end
	 Table
      end
   end
end
