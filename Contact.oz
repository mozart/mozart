functor
import
   Externalizable('class':XT)
   GetSlot('class':GS)
   Admin(manager:Manager)
   ContactName('class':CName)
   Entry('class':EntryClass)
   HTML_Contact('class':HTMLClass)
   MogulID(normalizeID:NormalizeID)
export
   'class' : Contact
define
   class Contact from GS XT EntryClass HTMLClass
      feat type : contact
      attr id pid url email www name name_for_index
      meth init(Msg Id Url Pid Prev)
	 {Msg check_keys(['name'])}
	 NAME = {New CName init({Msg get1('name' $)})}
      in
	 EntryClass,init(Msg)
	 id    <- {NormalizeID Id Pid}
	 pid   <- Pid
	 url   <- Url
	 email <- {Msg condGet1('email' unit $)}
	 www   <- {Msg condGet1('www' unit $)}
	 name  <- {NAME toVS($)}
	 name_for_index <- {Msg condGet1('name-for-index' unit $)}
	 if @name_for_index==unit then
	    name_for_index<-{NAME toVSIndex($)}
	 end
	 %% !!! we should copy the persistent info from Prev
      end
      meth extern_label($) 'contact' end
      meth extern_slots($)
	 [content_type body id pid url email www name name_for_index]
      end
      meth printOut(Margin Out DB)
	 {Out write(vs:Margin#' '#@id#' (contact)\n')}
      end
      meth updatePub(_)
	 {Manager trace('Skipping contact '#@id)}
      end
      meth updateProvided(_ _)
	 {Manager trace('Skipping contact '#@id)}
      end
      meth updatePkgList(_ L $)
	 {Manager trace('Skipping contact '#@id)}
	 L
      end
      %%
      meth updateAuthorList(DB L $)
	 {Manager trace('Skipping contact '#@id)}
	 L
      end
      %%
      meth getContributions($)
	 try
	    {Manager get_authors($)}.@id
	 catch _ then
	    {Manager trace(@id#" has no registered contributions in the database")}
	    nil
	 end
      end
   end
end
