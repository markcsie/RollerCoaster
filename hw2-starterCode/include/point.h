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

inline Point crossProduct(Point u, Point v)
{
  Point uv;
  uv.x_ = u.y_ * v.z_ - u.z_*v.y_;
  uv.y_ = u.z_ * v.x_ - u.x_*v.z_;
  uv.z_ = u.x_ * v.y_ - u.y_*v.x_;
  return uv;
}

inline Point normalize(Point p)
{
  Point n;
  double length = std::sqrt(std::pow(p.x_, 2) + std::pow(p.y_, 2) + std::pow(p.z_, 2));
  n.x_ = p.x_ / length;
  n.y_ = p.y_ / length;
  n.z_ = p.z_ / length;
  return n;
}

inline Point operator-(const Point &p1, const Point &p2)
{
  Point p;
  p.x_ = p1.x_ - p2.x_;
  p.y_ = p1.y_ - p2.y_;
  p.z_ = p1.z_ - p2.z_;
  return p;
}

inline Point operator+(const Point &p1, const Point &p2)
{
  Point p;
  p.x_ = p1.x_ + p2.x_;
  p.y_ = p1.y_ + p2.y_;
  p.z_ = p1.z_ + p2.z_;
  return p;
}

#endif /* POINT_H */

