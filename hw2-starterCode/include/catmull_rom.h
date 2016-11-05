#ifndef CATMULL_ROM_H
#define CATMULL_ROM_H

#include <vector>

#include "point.h"

class CatmullRom
{
public:
  CatmullRom(const Point &p1, const Point &p2, const Point &p3, const Point &p4, const double &s);
  CatmullRom(const CatmullRom& orig);
  virtual ~CatmullRom();
  
  Point splinePoint(const double &u);
  std::vector<Point> subDivide(const double &u_0, const double &u_1, const double &max_line_length);
private:
  Point p1_, p2_, p3_, p4_;
  double s_;
};

#endif /* CATMULL_ROM_H */

