declare

[QTk]={Module.link ["QTk.ozf"]}
[BootName BootObject]={Module.link ["x-oz://boot/Name" "x-oz://boot/Object"]}

NewUniqueName=BootName.newUnique
GetClass=BootObject.getClass

`ooMeth`={NewUniqueName 'ooMeth'}
`ooFeat`={NewUniqueName 'ooFeat'}

fun{GetInfo Obj}
   C={GetClass Obj}
in
   r('meth':{Dictionary.keys C.`ooMeth`}
     'feat':{Arity C.`ooFeat`})
end

Parent={NewName}
Init={NewName}
Link={NewName}

L
W={QTk.build td(button(handle:L text:"Ok" action:toplevel#close))}

Data={GetInfo L}

class Control
   prop
      locking
   attr
      Con
   feat
      Toplevel
      Event
      Widgets
      Callback
      CBThId
      Desc
   meth Init(Top D)
      lock
	 S
      in
	 self.Toplevel=Top
	 Con<-unit
	 self.Widgets={NewDictionary}
	 self.Callback={NewPort S}
	 self.Event={NewDictionary}
	 self.Desc=D
	 thread
	    proc{Loop M}
	    X#Msg|Xs=M
	       W={Dictionary.condGet self.Widgets X unit}
	    in
	       if W\=unit then {W Msg} end
	       {Loop Xs}
	    end
	 in
	    self.CBThId={Thread.this}
	    {Loop S}
	 end
	 {Wait self.CBThId}
      end
   end
   meth isConnected($)
      lock
	 @Con\=unit
      end
   end
   meth connect(To)
      lock
	 if @Con\=unit then
	    {self disconnect(To)}
	 end
	 Con<-To
	 {Port.send @Con Desc}
	 
      end
   end
end

CObj={New Control Init(W)}

class Proxy
   feat
      !Parent
      BindDict
      Name
      Ctrl
   attr
      Widget
   meth !Init(P N C BuildWidget Features)
      self.Parent=P
      self.BindDict={NewDictionary}
      self.Name=N
      self.Ctrl=C
      {ForAll Features
       proc{$ F} self.F=BuildWidget.F end}
   end
   meth set(...)=M
      skip
   end
   meth get(...)=M
      skip
   end
   meth close
      skip
   end
   meth bind(evetn:E
	     action:A
	     args:LA
	     append:AP)
      skip
   end
   meth otherwise(M)
      {self.Source M}
   end
end

PClass={Class.new [Proxy] r {List.toTuple r Data.'feat'} nil}

S={New PClass Init(unit label1 CObj L Data.'feat')}

{W show}
{S set(text:"Test")}