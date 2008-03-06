functor
import
   Glue at 'x-oz://boot/Glue'
   DP
   Property
   DPService
   Error
export
   This
   Resolve
   DistributedURIs
   AllURIs
   AddURI
define
   {DP.init}
   fun{This}
      {Glue.getThisSite}
   end
   {DPService.register
    {This}
    'oz:URIs'
    proc{$ 'oz:URIs' S M}
       case M
       of getAll(?X) then
	  X=@URIs
       [] add(Uri)then
	  if S=={This} then
	     O in O=URIs:=Uri|O
	  else
	     {Error.printException {Exception.error dp(service localOnly 'oz:URIs' M)}}
	  end
       else
	  {Error.printException {Exception.error dp(service unknownMessage 'oz:URIs' M)}}
       end
    end}
   fun{Resolve UriVS}
      Uri={VirtualString.toString UriVS}
      Scheme={String.toAtom {List.takeWhile Uri fun{$C}C\=&:end}}
   in
      {{Property.get 'dp.resolver'}.Scheme Uri}.site
   end
   fun{DistributedURIs S}
      Info={Value.condSelect S info badInfo}
   in
      [{VirtualString.toString
	"oz-site://s("#Info.ip#";"#
	{Int.toString Info.port}#";"
	#{Int.toString Info.id}#")"}]
   end
   fun{AllURIs S}
      {DPService.send S 'oz:URIs' getAll($)}
   end
   fun{AddURI S Uri}
      {DPService.send S 'oz:URIs' add(Uri)}
   end
   URIs={NewCell {DistributedURIs {This}}}
end