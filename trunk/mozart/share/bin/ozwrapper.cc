/*
 * wrapper for syslets. Does something like
 *     "exec ozengine $0 $@"
 */

#include <windows.h>
#include <process.h>

void main(int argc, char **argv)
{
  char buffer[5000];
  GetModuleFileName(NULL, buffer, sizeof(buffer));

  char **newargs = new char*[argc+1];

  newargs[0] = "ozengine";
  newargs[1] = buffer;
  int i=2;
  for(; i<=argc; i++) {
    newargs[i] = argv[i-1];
  }
  newargs[i] = 0;

  return spawnvp(P_WAIT,newargs[0],newargs);
}
