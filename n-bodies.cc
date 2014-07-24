#include <iostream>
#include <stdlib.h>

// #include <GL/glew.h>
// #include <GLFW/glfw3.h>    // check out bash history on your desktop to get a list of dependencies...


// TODO:
// determine sane body values
// id-based memoization optimization (store force vectors between pairs, and calculate the 
// value for one member.  Then, easily get the value for the second by negating x, y, z_comps).
// bounds/overflow checks

#include "universe.h"

#include "pheap.h"

using namespace std;

// replaces operators 'new' and 'delete' in the global namespace
// these definitions could go in their own separate translation unit,
// or could be defined only within the scope of the class

void *operator new(size_t n) {
  cout << "operator 'new' called" << endl;
  return pmalloc(n);
  // it seems that placement new is automatically called for the class being instantiated...
}

void operator delete(void *p) throw() {
  cout << "operator 'delete' called" << endl;
  pfree(p);
}


void die() {
  //fputs("usage: ./n-bodies <# bodies>\n", stderr);
  cerr << "usage: ./n-bodies <# bodies> <# turns> <psync period>\n"
  << "(# turns = 0 runs indefinitely; psync period = 0 disables psync)" << endl;
  exit(1);
}

/**
 * Runs a series of n-body simulations.
 */

int main(int argc, char *argv[]) {
  if (!(argv[1] && argv[2] && argv[3])) die();
  size_t num_bodies = (size_t)strtol(argv[1], NULL, 10);
  if (0 == num_bodies) die();

  size_t num_turns = (size_t)strtol(argv[2], NULL, 10);
  size_t psync_period = (size_t)strtol(argv[3], NULL, 10);

  Universe *u = new Universe(num_bodies);

  cout << "simulating " << num_bodies << " bodies" << endl;
  if (0 != num_turns) {
    cout << "running for " << num_turns << " turns" << endl;
    u->runNTurns(num_turns, psync_period);    // TODO - coupling too tight
  }
  else {
    cout << "running forever" << endl;
    u->run(psync_period);
  }

  delete u;

  return 0;
}