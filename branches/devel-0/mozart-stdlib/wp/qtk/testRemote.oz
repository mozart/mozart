declare

[QTk]={Module.link ["QTkBare.ozf"]}

C

{Show 1}
Win={QTk.buildMigratable td(canvas(handle:C))}
{Show 2}
Tag1={C newTag($)}
Tag2={C newTag($)}
{C create(line 10 10 100 100 tags:q(Tag1 Tag2))}
{Show 3}

{Pickle.save {Connection.offerUnlimited {Win get(ref:$)}} "ticket"}
{Show 4}
{Tag2 move(10 10)}