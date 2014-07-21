#ifndef BODY_H_INCLUDED
#define BODY_H_INCLUDED


/**
 * By maintaining a unique ID for each body, we can avoid doing redundant
 * work in the O(n^2) advance() function.  Specifically, we needn't check for
 * collisions or re-compute gravitational force for the latter member of a pair
 * of bodies that we've already considered.
 */

class Body {
  friend class Universe;

public:

  // represents a force vector
  // TODO - make these consts
  typedef struct Force {
    double x_comp, y_comp, z_comp;
  } Force;

  Body(double x, double y, double z, double vx, double vy, double vz,
  	       double mass, double radius, unsigned long long id);
  void applyForce(const Force &f);
  void updatePosition();

private:
  double x, y, z;     // position
  double vx, vy, vz;  // velocity
  const double mass;
  const double radius;
  const unsigned long long id;
};

#endif