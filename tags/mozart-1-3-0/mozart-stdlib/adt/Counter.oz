functor
export
   New NewFromInt
prepare
   fun {CounterPack C}
      fun  {CounterGet  } {Access C  } end
      proc {CounterPut I} {Assign C I} end
      proc {CounterGetPut I1 I2}
	 {Exchange C I1 I2}
      end
      fun  {CounterNext} I1 I2 in
	 {Exchange C I1 I2}
	 I2=I1+1
	 I1
      end
      proc {CounterPlus I} I1 I2 in
	 {Exchange C I1 I2}
	 I2=I1+I
      end
      proc {CounterTimes I} I1 I2 in
	 {Exchange C I1 I2}
	 I2=I1*I
      end
      fun {CounterClone}
	 {NewFromInt {Access C}}
      end
   in
      counter(
	 get      : CounterGet
	 put      : CounterPut
	 getPut   : CounterGetPut
	 next     : CounterNext
	 plus     : CounterPlus
	 times    : CounterTimes
	 clone    : CounterClone)
   end
   fun {NewFromInt I} {CounterPack {NewCell I}} end
   fun {New} {NewFromInt 1} end
end
