#ifndef UNIVERSE_H_INCLUDED
#define UNIVERSE_H_INCLUDED

#include <vector>

#include "body.h"

#define G 6.67384e-11   // universal gravitational constant


class Universe {
  //friend class Body;

public:

  Universe(size_t num_bodies);
  void advance();
  void run();
  void runNTurns(size_t num_turns);

private:

  double distance(const Body &p1, const Body &p2);
  Body::Force force(const Body &p1, const Body &p2);
  void addForce(const Body::Force &to_add, Body::Force &cumulative);

  std::vector<Body> bodies;

};

#endif