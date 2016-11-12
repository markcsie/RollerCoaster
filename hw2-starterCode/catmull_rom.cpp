#include "catmull_rom.h"

#include <iostream>
#include <cmath>
#include <vector>

CatmullRom::CatmullRom(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &p4, const float &s) : p1_(p1), p2_(p2), p3_(p3), p4_(p4), s_(s)
{
}

CatmullRom::CatmullRom(const CatmullRom& other)
{
}

CatmullRom::~CatmullRom()
{
}

glm::vec3 CatmullRom::splinePoint(const float &u)
{
  float u3 = std::pow(u, 3);
  float u2 = std::pow(u, 2);
  float k1 = -s_ * u3 + 2 * s_ * u2 - s_ * u;
  float k2 = (2 - s_) * u3 + (s_ - 3) * u2 + 1;
  float k3 = (s_ - 2) * u3 + (3 - 2 * s_) * u2 + s_ * u;
  float k4 = s_ * u3 - s_ * u2;
  
  return k1 * p1_ + k2 * p2_ + k3 * p3_ + k4 * p4_;
}

glm::vec3 CatmullRom::splineTangent(const float &u)
{
  float u2 = std::pow(u, 2);
  
  float k1 = -s_ * 3 * u2 + 2 * s_ * 2 * u - s_;
  float k2 = (2 - s_) * 3 * u2 + (s_ - 3) * 2 * u2;
  float k3 = (s_ - 2) * 3 * u2 + (3 - 2 * s_) * 2 * u2 + s_;
  float k4 = s_ * 3 * u2 - s_ * 2 * u2;
  
  return k1 * p1_ + k2 * p2_ + k3 * p3_ + k4 * p4_;
}

std::vector<glm::vec3> CatmullRom::subDivide(const float &u_0, const float &u_1, const float &max_line_length) {
  glm::vec3 x_0 = splinePoint(u_0);
  glm::vec3 x_1 = splinePoint(u_1);
  if (glm::distance(x_0, x_1) > max_line_length)
  {
    float u_mid = (u_0 + u_1) / 2.0;
    std::vector<glm::vec3> left_points = subDivide(u_0, u_mid, max_line_length);
//    std::cout << "left_points " << std::endl;
//    for (const auto &p : left_points)
//    {
//      std::cout << p.x_ << " " << p.y_ << " " << p.z_ << std::endl;
//    }
    std::vector<glm::vec3> right_points = subDivide(u_mid, u_1, max_line_length);
//    std::cout << "right_points " << std::endl;
//    for (const auto &p : right_points)
//    {
//      std::cout << p.x_ << " " << p.y_ << " " << p.z_ << std::endl;
//    }
    left_points.insert(std::end(left_points), std::begin(right_points) + 1, std::end(right_points));
    return left_points;
  } else {
    std::vector<glm::vec3> points;
    points.push_back(x_0);
    points.push_back(x_1);
    return points;
  }
}
