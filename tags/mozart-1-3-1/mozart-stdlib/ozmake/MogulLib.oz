functor
prepare
   fun {Spaces N}
      if N>0 then & |{Spaces N-1} else nil end
   end

   INDENT = 16

   fun {Header Label Value}
      N = {VirtualString.length Label}
   in
      Label#':'#{Spaces INDENT-N-1}#Value#'\n'
   end

   fun {MaybeHeader Label Value}
      if Value==unit then nil else {Header Label Value} end
   end

   fun {ExtendedHeader Label Value}
      {Header Label {ExtendedValue {VirtualString.toString Value}}}
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

   ToTuple = List.toTuple

   fun {ComparePlatforms I1 I2}
      P1 = if I1==unit then unit else {I1 get(platform $)} end
      P2 = if I2==unit then unit else {I2 get(platform $)} end
   in
      if P1==P2 then '='
      elseif P1==source then '<'
      elseif P2==source then '>'
      else '?' end
   end

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

   class MogulContact
      attr
	 id             : unit
	 name           : unit
	 name_for_index : unit
	 email          : unit
	 www            : unit

      meth toVS($)
	 {Header type contact}#
	 {Header id @id}#
	 {Header name @name}#
	 {MaybeHeader 'name-for-index' @name_for_index}#
	 {MaybeHeader email @email}#
	 {MaybeHeader www @www}
      end

      meth init(R)
	 id <- R.id
	 name <- R.name
	 name_for_index <- {CondSelect R 'name-for-index' unit}
	 email <- {CondSelect R email unit}
	 www <- {CondSelect R www unit}
      end
   end

   class MogulPackageBase
      meth get(A $) @A end
      meth url_pkg_list($)
	 if @tar==nil then
	    [{Path.resolve
	      {@manager get_mogulpkgurl($)}
	      {Utils.mogulToPackagename @id}}]
	 else
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
   end

   class MogulInstance from MogulPackageBase
      attr
	 manager    : unit
	 id         : unit
	 format     : unit
	 platform   : unit
	 version    : unit
	 released   : unit
	 author     : nil
	 blurb      : unit
	 tar        : nil
	 provides   : unit
	 requires   : unit
	 info_html  : unit
	 info_text  : unit

      meth toLabel($) @id end

      meth toVS($)
	 {Header type instance}#
	 {Header id @id}#
	 {Header format @format}#
	 {MaybeHeader platform @platform}#
	 {MaybeHeader version @version}#
	 {MaybeHeader released @released}#
	 {ToTuple '#'
	  {Map @author fun {$ A} {Header author A} end}}#
	 {MaybeExtendedHeader blurb @blurb}#
	 {ToTuple '#'
	  {Map {self url_pkg_list($)}
	   fun {$ U} {Header 'url-pkg' U} end}}
      end
   end

   class MogulPackage from MogulPackageBase
      attr
	 manager    : unit
	 id         : unit
	 format     : unit
	 platform   : unit
	 version    : unit
	 released   : unit
	 author     : nil
	 blurb      : unit
	 categories : nil
	 tar        : nil
	 doc        : unit
	 provides   : nil
	 requires   : nil
	 info_html  : unit
	 info_text  : unit
	 instances  : nil

      meth toVS($)
	 {Header type package}#
	 {Header id @id}#
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
	 {ToTuple '#'
	  {Map @instances fun {$ I} {Header instance {I toLabel($)}} end}}#
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
	 {ChooseBestRepresentative @instance}
      end
   end
end
