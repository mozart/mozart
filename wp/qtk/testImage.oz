declare

[QTk]={Module.link ["QTkBare.ozf"]}

Img={QTk.newImage bitmap(url:"mini-inc.xbm")}

%{{QTk.build td(label(image:Img))} show}

Remote={QTk.buildMigratable td(label(image:Img))}
Rec
{{QTk.build td(receiver(handle:Rec))} show}
{Rec set({Remote get(ref:$)})}
Rec2
{{QTk.build td(receiver(handle:Rec2))} show}
{Rec2 set({Remote get(ref:$)})}

Lib={QTk.newImageLibrary}
{Lib newBitmap(name:'mini-inc' url:"mini-inc.xbm")}
{QTk.saveImageLibrary Lib "testImageLib.ozf"}

[TestLibMod]={Module.link ["testImageLib.ozf"]}

TestLib={QTk.buildImageLibrary TestLibMod.buildLibrary}



Rem2={QTk.buildMigratable td(label(image:{TestLib get(name:'mini-inc' image:$)}))}
{Rec set({Rem2 get(ref:$)})}
