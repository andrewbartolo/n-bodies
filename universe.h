#ifndef UNIVERSE_H_INCLUDED
#define UNIVERSE_H_INCLUDED

#include <vector>

#include "body.h"

#include "pheap.h"

#define G 6.67384e-11   // universal gravitational constant

// using this larger value of G, body motion is made evident body.cc's position & force debugging printout
//#define G 6.67384e-6


class Universe {
  //friend class Body;

public:

  Universe(const size_t num_bodies);
  void advance(const size_t psync_period);
  void run(const size_t psync_period);
  void runNTurns(const size_t num_turns, const size_t psync_period);

private:

  double distance(const Body &p1, const Body &p2);
  Body::Force force(const Body &p1, const Body &p2);
  void addForce(const Body::Force &to_add, Body::Force &cumulative);

  std::vector<Body> bodies;
  size_t turns_completed;
};

#endif