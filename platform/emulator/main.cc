#include "am.hh"
#include "io.hh"
#include "thread.hh"

int main (int argc,char **argv)
{
  am.init(argc,argv);
  engine();
  IO::exitOz(0);
  return 0;  // to make CC quiet
}

