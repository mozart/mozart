{ print $0; }

/^[ \t]*<!--#include "[^"\r\n]+"-->[ \t\r]*$/ {
  match($0,/".+"/);
  file = "../" substr($0, RSTART + 1, RLENGTH - 2);
  system("if [ -r " file " ]; then cat " file "; fi");
}
