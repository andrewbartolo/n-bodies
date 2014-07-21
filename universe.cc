#include <iostream>
#include <math.h>
#include <time.h>
#include <stdlib.h>

// #include <GL/glew.h>
// #include <GLFW/glfw3.h>    // check out bash history on your desktop to get a list of dependencies...

#include "universe.h"
#include "pheap.h"

#define RAND_DOUBLE (((double)rand()) / (double)RAND_MAX)   // uniform? distribution btwn [0, 1)

#define G 6.67384e-11   // universal gravitational constant

using namespace std;

/**
 * TODO - comment the header file, not the .cc file...
 */


// can't set vector size at declaration unless the contained type has a
// default constructor.  Body does not, so we call reserve() in the
// constructor instead
Universe::Universe(size_t num_bodies) {//: bodies(num_bodies) {
  bodies.reserve(num_bodies);
  srand(time(NULL));      // TODO - possible issue here with static storage

  // fill the vector with randomly-created bodies
  // bodies are constructed in-place using emplace, w/o copy constructor
  for (size_t i = 0; i < num_bodies; ++i)
    bodies.emplace_back(RAND_DOUBLE, RAND_DOUBLE, RAND_DOUBLE, RAND_DOUBLE,
                           RAND_DOUBLE, RAND_DOUBLE, RAND_DOUBLE, RAND_DOUBLE,
                           i);

  // cout << "num_bodies was " << num_bodies << endl;
  // cout << "vector bodies is of size " << bodies.size() << endl;
  // cout << bodies[num_bodies / 2].getMass() << endl;
}

/**
 * Advances the simulation through one turn.
 * This function runs in O(n^2) time.  For each body in vector bodies, it
 * considers every other point, checking to see if a collision has occurred (via volume
 * calculated from radius), and computes gravitational force.
 */
void Universe::advance() {
  for (Body &outer : bodies) {
    // cumulative net force on the body considered in the outer for loop
    Body::Force net_force;
    net_force.x_comp = net_force.y_comp = net_force.z_comp = 0;

    for (Body &inner : bodies) {
      // bodies don't affect themselves
      if (outer.id != inner.id) {
        // TODO - check for collisions first
        // if a collision occurred, volume-merge the bodies

        addForce(force(outer, inner), net_force);
      }

    }

    outer.applyForce(net_force);    // changes bodies' velocities
  }

  for (Body &b : bodies) {          // updates bodies' positions based on their velocities
    b.updatePosition();
  }

}

/**
 * Runs the simulation indefinitely.
 */
void Universe::run() {
  while (true) {
    advance();
  }
}

/**
 * Runs the simulation for n turns.
 */
void Universe::runNTurns(size_t num_turns) {
  for (size_t i = 0; i < num_turns; ++i) {
    advance();
  }
}

inline double Universe::distance(const Body &p1, const Body &p2) {
  double x_diff = p1.x - p2.x;
  double y_diff = p1.y - p2.y;
  double z_diff = p1.z - p2.z;
  return sqrt((x_diff * x_diff) + (y_diff * y_diff) + (z_diff * z_diff));
}

/**
 * Computes the gravitational force vector on p1 as a result of p2.
 */
inline Body::Force Universe::force(const Body &p1, const Body &p2) {
  double dist = distance(p1, p2);
  double magn = (G * p1.mass * p2.mass) / (dist * dist);

  // TODO - need to check my trig here...
  // f.x_comp = magn * cos(theta_x), where theta_x = arccos(x_dist / dist)... ?
  // TODO - return a struct literal instead?
  Body::Force f;
  f.x_comp = magn * ((p2.x - p1.x) / dist);
  f.y_comp = magn * ((p2.y - p1.y) / dist);
  f.z_comp = magn * ((p2.z - p1.z) / dist);
  return f;
}

inline void Universe::addForce(const Body::Force &to_add, Body::Force &cumulative) {
  cumulative.x_comp += to_add.x_comp;
  cumulative.y_comp += to_add.y_comp;
  cumulative.z_comp += to_add.z_comp;
}