functor
require
   Open
prepare
   Data =
   for F in ['default.htm' 'GJoz.class' 'OzLink.class']
      collect : Collect
   do
      O={New Open.file init(url:'html/'#F)}
      Vs={O read(list:$ size:all)}
   in
      {O close}
      {Collect F#Vs}
   end
export Data   
end
