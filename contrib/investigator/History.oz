functor

export

   HistoryClass

   
define

   class HistoryClass

      attr
	 last: ~1
	 curr: ~1
	 hist: {Dictionary.new}
		
      meth init
	 skip
      end

      meth is_prev(B)
	 B = (@curr > 0)
      end

      meth is_next(B)
	 B = (@curr < @last)
      end

      meth get_prev_action(Action Args)
	 curr <- @curr - 1
	 Action#Args = {Dictionary.get @hist @curr}	 
      end

      meth get_next_action(Action Args)
	 curr <- @curr + 1
	 Action#Args = {Dictionary.get @hist @curr}
      end

      meth add_action(Action Args)
	 OldLast = @last
      in
	 curr <- @curr + 1
	 
	 last <- @curr
	 {Dictionary.put @hist @curr Action#Args}

	 if OldLast > @last then
	    {For @last+1 OldLast 1 proc {$ I} {Dictionary.remove @hist I} end}
	 else skip end
      end

      meth insert_menu(M)
	 M =
	 if HistoryClass,is_prev($) 
	 then "menu_entry(\"prev\",\"Previous\"),"
	 else "" end
	 #if HistoryClass,is_next($) 
	 then "menu_entry(\"next\",\"Next\"),"
	 else "" end
	 #if HistoryClass,is_prev($) orelse HistoryClass,is_next($)
	  then "blank," else "" end
      end
   end

end
