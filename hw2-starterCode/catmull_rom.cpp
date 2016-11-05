#include "catmull_rom.h"

#include <iostream>
#include <cmath>
#include <vector>

CatmullRom::CatmullRom(const Point &p1, const Point &p2, const Point &p3, const Point &p4, const double &s) : p1_(p1), p2_(p2), p3_(p3), p4_(p4), s_(s)
{
}

CatmullRom::CatmullRom(const CatmullRom& other)
{
}

CatmullRom::~CatmullRom()
{
}

Point CatmullRom::splinePoint(const double &u)
{
  double u3 = std::pow(u, 3);
  double u2 = std::pow(u, 2);
  double k1 = -s_ * u3 + 2 * s_ * u2 - s_ * u;
  double k2 = (2 - s_) * u3 + (s_ - 3) * u2 + 1;
  double k3 = (s_ - 2) * u3 + (3 - 2 * s_) * u2 + s_ * u;
  double k4 = s_ * u3 - s_ * u2;
  
  Point p;
  p.x_ = k1 * p1_.x_ + k2 * p2_.x_ + k3 * p3_.x_ + k4 * p4_.x_;
  p.y_ = k1 * p1_.y_ + k2 * p2_.y_ + k3 * p3_.y_ + k4 * p4_.y_;
  p.z_ = k1 * p1_.z_ + k2 * p2_.z_ + k3 * p3_.z_ + k4 * p4_.z_;
  return p;
}

//Point catmullRom(double u, double s) {
//
//}
//
//void subDivide(double u_0, double u_1, double max_line_length) {
//  double u_mid = (u_0 + u_1) / 2;
//  Point x_0 = catmullRom(u_0)
//}

std::vector<Point> CatmullRom::subDivide(const double &u_0, const double &u_1, const double &max_line_length) {
  Point x_0 = splinePoint(u_0);
  Point x_1 = splinePoint(u_1);
  if (distance(x_0, x_1) > max_line_length)
  {
    double u_mid = (u_0 + u_1) / 2;
    std::vector<Point> left_points = subDivide(u_0, u_mid, max_line_length);
//    std::cout << "left_points " << std::endl;
//    for (const auto &p : left_points)
//    {
//      std::cout << p.x_ << " " << p.y_ << " " << p.z_ << std::endl;
//    }
    std::vector<Point> right_points = subDivide(u_mid, u_1, max_line_length);
//    std::cout << "right_points " << std::endl;
//    for (const auto &p : right_points)
//    {
//      std::cout << p.x_ << " " << p.y_ << " " << p.z_ << std::endl;
//    }
    left_points.insert(std::end(left_points), std::begin(right_points) + 1, std::end(right_points));
    return left_points;
  } else {
    std::vector<Point> points;
    points.push_back(x_0);
    points.push_back(x_1);
    return points;
  }
}
