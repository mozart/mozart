declare

[QTk]={Module.link ["QTkBare.ozf"]}

RW RC
RW={QTk.buildMigratable td(canvas(handle:RC))}

R
W={QTk.build td(receiver(handle:R) title:"1")}
{W show}
{R set({RW get(ref:$)})}

{RC create(line 10 10 100 100)}

Tag
{RC create(oval 10 10 100 100 handle:Tag)}
{Tag move(10 10)}
{Browse Tag}
R2
{{QTk.build td(receiver(handle:R2) title:"2")} show}

%Tag2={RC create(rect 10 10 100 100 handle:$)}

{R2 set({RW get(ref:$)})}

