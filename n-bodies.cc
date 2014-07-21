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

using namespace std;


void die() {
  //fputs("usage: ./n-bodies <# bodies>\n", stderr);
  cerr << "usage: ./n-bodies <# bodies> [# turns]\n" << endl;
  exit(1);
}

/**
 * Runs a series of n-body simulations.
 */
int main(int argc, char *argv[]) {
  if (!argv[1]) die();
  size_t num_bodies = (size_t)strtol(argv[1], NULL, 10);
  if (0 == num_bodies) die();

  ssize_t num_turns = (argv[2]) ? (size_t)strtol(argv[2], NULL, 10) : -1;

  Universe *u = new Universe(num_bodies);

  cout << "simulating " << num_bodies << " bodies" << endl;
  if (-1 != num_turns) {
    cout << "running for " << num_turns << " turns" << endl;
    u->runNTurns(num_turns);
  }
  else {
    cout << "running forever" << endl;
    u->run();
  }

  return 0;
}