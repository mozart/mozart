functor

export
   Observe
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

   fun{Observe O}
      if {Object.is O} then
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
	      prop locking
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
      else _
      end  
   end

end

   
% class MyClass
%    feat n
%    meth init(N)
%       self.n=N
%    end
%    meth show(M)
%       {System.showInfo self.n#"#"#M}
%    end
% end

% O1={New {Observe MyClass} init(1)}
% O2={Observe {New MyClass init(2)}}

% {Browse {GetEventStream O1}}
% {Browse {GetEventStream O2}}

% {O1 show("hello")}
% {O2 show("hello")}

% {WatchEvent O1 show}
% {WatchEvent O2 show}
% thread
%    {ForAll {GetEventStream O1}
%     proc{$ Evt}
%        {InjectEvent O2 Evt}
%     end}
% end
% thread
%    {ForAll {GetEventStream O2}
%     proc{$ Evt}
%        {InjectEvent O1 Evt}
%     end}
% end


% {O1 show("world")}
% {O2 show("bummer")}



