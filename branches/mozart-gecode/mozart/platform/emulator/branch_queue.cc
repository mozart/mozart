#include "branch_queue.hh"

void BranchQueue::print(void) {
  for(node *tmp=first; tmp!= NULL; tmp=tmp->next) {
    printf("BQ: %s\n",OZ_toC(tmp->branch,100,100)); fflush(stdout);
  }
  printf("BQ: FIN\n"); fflush(stdout);
}

