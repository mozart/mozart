functor

export

   Editor
   
   Monday
   Tuesday
   Wednesday
   Thursday
   Friday
   
   QuartersPerDay
   QuartersPerHour

   DemoMode

require

   ErrorRegistry
   
prepare

   {ErrorRegistry.put college
    fun {$ E}
       T = 'error in college timetabling system'
    in
       case E
       of college(A Xs S) then
	  %% expected: A: application Xs:list, S:virtualString
	  error(kind: T
		items: ([hint(l: 'In statement' m: apply(A Xs))
			 hint(l: 'Reason      ' m: S)]))
	  
       else
	  error(kind: T
		items: [line(oz(E))])
       end
    end}

define
   %% my favorite editor
   Editor          = "emacs"
   
   %% the week days
   Monday          = 1#36
   Tuesday         = 37#72
   Wednesday       = 73#108
   Thursday        = 109#144
   Friday          = 145#180
   
   QuartersPerDay  = 36
   QuartersPerHour = 4

   DemoMode = on

end
