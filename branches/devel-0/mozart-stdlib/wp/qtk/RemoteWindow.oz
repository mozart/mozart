functor

import

   QTk at 'QTkBare.ozf'
   Connection
   Pickle

define

   Ticket={Pickle.load "ticket"}
   Con={Connection.take Ticket}
   Rec
   {{QTk.build td(title:"Remote window" receiver(handle:Rec))} show}
   {Rec set(Con)}
   
end
