local


Name = {OS.tmpnam}
SourceName = !Name#".tex"
DVIName = !Name#".dvi"

XgrabFileName =  {OS.tmpnam}#".ps"

LaTeXSource=
'\\documentstyle[11pt,float,pstricks,pst-node,epsf,epic]{article}
\\begin{document}
\\section*{Vorlesungsplan der Katholischen Hochschule
           f\\"ur Soziale Arbeit, Saarbr\\"ucken}
\\begin{center}
\\parbox{14cm}{
\\epsfxsize=14cm
\\epsffile{
'
#
!XgrabFileName
#
'}}
\\end{center}
\\end{document}'

local
   OO = {New Open.file init(name:SourceName
                            flags:[read write 'create'])}
in
   {OO write(vs:LaTeXSource)}
end

in

   %% Write computes the first solution
   %% and writes three different listings to files

   proc {Xgrab}
      Flag={OS.system 'xgrabsc -ps -click -o '#XgrabFileName}
   in
      {Wait Flag}
      {OS.system 'cd /tmp; latex '#SourceName#'; dvips '#DVIName _}
   end
end
