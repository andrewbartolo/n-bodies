#include <cstdlib>
#include <ctime>
#include <iostream>

#include "body.h"

using namespace std;


Body::Body(double x, double y, double z, double vx, double vy, double vz,
	                 double mass, double radius, unsigned long long id) :
                   x(x), y(y), z(z), vx(x), vy(y), vz(z), mass(mass), radius(radius), id(id) {}

/**
 * Uses the supplied force to compute acceleration
 * on the body, then changes the body's
 * velocity fields accordingly.  Note that the body's
 * position is not updated until updatePosition() is called.
 */
void Body::applyForce(const Force &f) {
  vx += (f.x_comp / mass);
  vy += (f.y_comp / mass);
  vz += (f.z_comp / mass);
  // cout << "force x-component: " << f.x_comp << endl;
  // cout << "force y-component: " << f.y_comp << endl;
  // cout << "force z-component: " << f.z_comp << endl;

}

/**
 * Updates the body's position based on its velocity.
 * (Velocity is given as change in position per turn.)
 */
void Body::updatePosition() {
  x += vx;
  y += vy;
  z += vz;
  // cout << "new x-position: " << x << endl;
  // cout << "new y-position: " << y << endl;
  // cout << "new z-position: " << z << endl;
}