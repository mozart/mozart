#! /bin/csh -f

# this file is maintained by CVS in module 'Oz/bin'

# drucke oz dateien aus
# prinzip: oz2tex --> latex --> dvips
# Argumente: oz-files

if ($#argv < 1) then
  echo "usage: $0 <oz-files>"
  exit 1
endif



set tmpfile	= `mktemp oz2lpr-tex-`
set ozfile	= `mktemp oz2lpr-oz-`

foreach i ($*)

if ("$i" == "-") then    # input from stdin?
   set file = "stdin"
   cat >! /tmp/$ozfile.oz
else
   if ( -f $i ) then
     set file	= $i
   else
     set file	= $i.oz
   endif

   if (! -f $file ) then
     echo "file not found: $file"
     continue
  endif

  cp $file /tmp/$ozfile.oz
endif

oz2tex /tmp/$ozfile.oz >& /dev/null

if ($status) then
  echo "oz2tex failed: $file"
  continue
endif

echo "\documentstyle[11pt,dina4]{article}\
\include{oz2tex_preamble}\
\pagestyle{empty}\
\begin{document}\
\input{/tmp/$ozfile.tex}\
\end{document}\
" >> /tmp/$tmpfile.tex

(cd /tmp; latex $tmpfile > /dev/null < /dev/null)

if ($status) then
  echo "latex failed: $file"
  egrep '^\!' /tmp/$tmpfile.log
  goto next
endif

dvips /tmp/$tmpfile >& /dev/null

next:
rm /tmp/$tmpfile* /tmp/$ozfile* /tmp/oz2tex_preamble*

end # foreach

