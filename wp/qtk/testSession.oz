declare

[Session]={Module.link ["Session.ozf"]}

SA={Session.new}

{Browse {Session.getStream SA}}
{Browse {Session.getStateStream SA}}
SB={Session.new}

{Browse {Session.getStream SB}}
{Browse {Session.getStateStream SB}}

{Session.bind SB connect proc{$} {Session.aSend SB 'helloB'} end}
{Show 1}
{Session.connect SA SB}
{Show 2}
{Session.aSend SA 'helloA'}
{Show 3}
{Delay 2000}
{Session.disconnect SA}

{Browse {Session.getSideStream SA}}
{Session.sideSend SC SA side}
SC={Session.new}
{Session.connect SC SA}
{Show {Session.isConnected SC}}
{Session.aSend SC hello}