#ifndef CATMULL_ROM_H
#define CATMULL_ROM_H

#include <vector>
#include <glm/glm.hpp>

class CatmullRom
{
public:
  CatmullRom(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &p4, const float &s);
  CatmullRom(const CatmullRom& orig);
  virtual ~CatmullRom();
  
  glm::vec3 splinePoint(const float &u);
  glm::vec3 splineTangent(const float &u);
  std::vector<glm::vec3> subDivide(const float &u_0, const float &u_1, const float &max_line_length);
private:
  glm::vec3 p1_, p2_, p3_, p4_;
  float s_;
};

#endif /* CATMULL_ROM_H */

