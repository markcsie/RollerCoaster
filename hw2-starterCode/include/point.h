#ifndef POINT_H
#define POINT_H

#include <cmath>

// represents one control point along the spline 

struct Point
{
  double x_;
  double y_;
  double z_;
};

inline double distance(Point p1, Point p2)
{
  return std::sqrt(std::pow(p1.x_ - p2.x_, 2) + std::pow(p1.y_ - p2.y_, 2) + std::pow(p1.z_ - p2.z_, 2));
}

#endif /* POINT_H */

