functor
  
export
   Observe
   ObserveGen
   Blackbox
   GetEventStream
   WatchEvent
   IgnoreEvent
   InjectEvent
   
require
   BootName at 'x-oz://boot/Name'
   BootObject at 'x-oz://boot/Object'

prepare
   Init={NewName}
   NewUniqueName=BootName.newUnique
   GetClass=BootObject.getClass
   BB={NewName}
   `ooFeat`={NewUniqueName 'ooFeat'}

define
   
   fun{GetFeats Obj}
      C={GetClass Obj}
   in
      {Record.map C.`ooFeat` fun{$ _} _ end}
   end

   class Blackbox
      prop locking
      attr evt
      feat port out obj

      meth init(O)
	 self.obj=O
	 self.port={NewPort self.out}
	 evt<-r
      end

      meth getEventStream($)
	 self.out
      end

      meth watch(Evt)
	 lock
	    evt<-{Record.adjoinAt @evt {Label Evt} Evt}
	 end
      end

      meth ignore(Evt)
	 lock
	    evt<-{Record.subtract @evt {Label Evt}}
	 end
      end

      meth inject(Evt)
	 lock
	    {self.obj Evt}
	 end
      end
   
      meth notifyMeth(Evt)
	 lock
	    if {HasFeature @evt {Label Evt}} then
	       {Port.send self.port Evt}
	    end
	 end
      end

   end

   fun{ObserveGen O Blackbox}
      Return
   in
      thread
	 Return=if {Object.is O} then
		   class RedirectorClass
		      feat Obj !BB
		      meth !Init(O)
			 self.Obj=O
			 self.BB={New Blackbox init(self.Obj)}
		      end
		      meth otherwise(M)
			 {self.Obj M}
			 {self.BB notifyMeth(M)}
		      end
		   end
		   Feats={GetFeats O}
		   R={New {Class.new [RedirectorClass] q Feats nil}
		      Init(O)}
		   {ForAll {Arity Feats} % copy all features
		    proc{$ F} R.F=O.F end}
		in
		   R     
		elseif {Class.is O} then
		   class Dummy from O
		      meth dummyinit skip end
		   end
		   Feats={GetFeats {New Dummy dummyinit}}
		in
		   {Class.new
		    [class $
			feat Obj !BB Inited
			meth otherwise(M)
			   if {IsFree self.Inited} then
			      self.Inited=unit
			      self.Obj={New O M}
			      self.BB={New Blackbox init(self.Obj)}
			      {ForAll {Arity Feats} % copy all features
			       proc{$ F} self.F=self.Obj.F end}
			      {self.BB notifyMeth(M)}
			   else
			      {self.Obj M}
			      {self.BB notifyMeth(M)}
			   end
			end
		     end]
		    q Feats nil}
		else
		   O
		end
      end
      if {IsDet O} then {Wait Return} end
      Return
   end

   proc{WatchEvent Entity Event}
      if {HasFeature Entity BB} then
	 {Entity.BB watch(Event)}
      end
   end

   proc{IgnoreEvent Entity Event}
      if {HasFeature Entity BB} then
	 {Entity.BB ignore(Event)}
      end
   end

   proc{InjectEvent Entity Event}
      if {HasFeature Entity BB} then
	 {Entity.BB inject(Event)}
      end
   end

   fun{GetEventStream Entity}
      if {HasFeature Entity BB} then
	 {Entity.BB getEventStream($)}
      else !!_
      end  
   end

   fun{Observe O}
      thread
	 {ObserveGen O Blackbox}
      end
   end
   
end
