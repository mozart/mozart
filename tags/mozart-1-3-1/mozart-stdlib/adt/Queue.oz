functor
export
   New NewFromList
prepare
   Take=List.take
   proc {Take2 L N L2 L3}
      if N<1 then L2=L3
      elsecase L of H|T then LL2 in
	 L2=H|LL2
	 {Take2 T N-1 LL2 L3}
      end
   end
   fun {QueuePack Q}
      fun {QueueGet} Old New in
	 {Exchange Q Old New}
	 case Old of N#L1#L2 then
	    %% note that we leave the queue in a consistent state
	    %% even if the operation raises an exception
	    if N==0 then New=Old raise empty end
	    elsecase L1 of H|T then New=N-1#T#L2 H end
	 end
      end
      proc {QueuePut X} New in
	 case {Exchange Q $ New}
	 of N#L1#L2 then L3 in L2=X|L3 New=N+1#L1#L3 end
      end
      proc {QueueGetPut X Y} Old New in
	 {Exchange Q Old New}
	 case Old of N#L1#L2 then
	    %% if the queue is empty we get back what we put
	    if N==0 then New=Old X=Y
	    elsecase L1 of H|T then L3 in
	       L2=Y|L3 New=N#T#L3 X=H
	    end
	 end
      end
      fun {QueueTop}
	 case {Access Q}
	 of N#L1#_ then
	    case N of 0 then raise empty end
	    elsecase L1 of X|_ then X end
	 end
      end
      fun {QueueToList}
	 case {Access Q} of N#L1#_ then {Take L1 N} end
      end
      fun {QueueIsEmpty} {Access Q}.1==0 end
      proc {QueueReset} L in {Assign Q 0#L#L} end
      fun {QueueToListKill}
	 case {Exchange Q unit} of _#H#T then T=nil H end
      end
      fun {QueueClone}
	 case {Access Q} of N#L#_ then L2 L3 in
	    {Take2 N L L2 L3}
	    {QueuePack {NewCell N#L2#L3}}
	 end
      end
   in
      queue(
	 get         : QueueGet
	 put         : QueuePut
	 getPut      : QueueGetPut
	 top         : QueueTop
	 toList      : QueueToList
	 toListKill  : QueueToListKill
	 isEmpty     : QueueIsEmpty
	 reset       : QueueReset
	 clone       : QueueClone
	 )
   end
   fun {New} L in {QueuePack {NewCell 0#L#L}} end
   fun {NewFromList L1} L2 in
      {QueuePack {NewCell {Length L1}#{Append L1 L2}#L2}}
   end
end