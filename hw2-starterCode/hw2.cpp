/*
  CSCI 420 Computer Graphics, USC
  Assignment 2: Roller Coaster
  C++ starter code

  Student username: <kaichiem>
 */

#include <glm/ext.hpp>

#include <iostream>
#include <vector>

#include "openGLHeader.h"
#include "glutHeader.h"
#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"

#include "include/catmull_rom.h"
#include "include/spline.h"

std::vector<GLfloat> cube_map_vertices = {
  // Positions          
  -1.0f, 1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, 1.0f, -1.0f,
  -1.0f, 1.0f, -1.0f,

  -1.0f, -1.0f, 1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f, 1.0f, -1.0f,
  -1.0f, 1.0f, -1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f, -1.0f, 1.0f,

  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,

  -1.0f, -1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, -1.0f, 1.0f,
  -1.0f, -1.0f, 1.0f,

  -1.0f, 1.0f, -1.0f,
  1.0f, 1.0f, -1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, -1.0f,

  -1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, 1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, 1.0f,
  1.0f, -1.0f, 1.0f
};

// speed
double divide_threshold = 0.01;
clock_t last_time;
const float g = 9.8 * 1000;
const float max_z = 7.0;
size_t current_spline_point_index = 0;

// spline
Spline spline;
std::vector<GLfloat> spline_vertices;
std::vector<glm::vec3> spline_n;
std::vector<glm::vec3> spline_t;
std::vector<glm::vec3> spline_b;
const float alpha = 0.1;
const float beta = 0.4;
const float cross_section_width = 2.0;
size_t numSplines;
size_t num_spline_points;

const GLuint windowWidth = 1280;
const GLuint windowHeight = 720;
const char windowTitle[512] = "CSCI 420 homework II";
size_t screenShotCount = 0;

OpenGLMatrix openGLMatrix;

BasicPipelineProgram texture_pipeline;
GLuint metal_texture_id;
GLuint wood_texture_id;
GLuint left_cross_section_vao;
std::vector<GLfloat> left_cross_section_vertices;
GLuint right_cross_section_vao;
std::vector<GLfloat> right_cross_section_vertices;
GLuint cross_bars_vao;
std::vector<GLfloat> cross_bar_vertices;

BasicPipelineProgram cube_map_pipeline;
GLuint cube_map_vao;
GLuint cube_map_id;

int loadSplines(char * argv)
{
  char * cName = (char *) malloc(128 * sizeof (char));
  FILE * fileList;
  FILE * fileSpline;

  // load the track file 
  fileList = fopen(argv, "r");
  if (fileList == NULL)
  {
    printf("can't open file\n");
    exit(1);
  }

  // stores the number of splines in a global variable 
  if (fscanf(fileList, "%lu", &numSplines) <= 0)
  {
    std::cerr << "error" << std::endl;
  }

  glm::vec3 last_point(0, 0, 0);
  // reads through the spline files 
  for (size_t j = 0; j < numSplines; j++)
  {
    if (fscanf(fileList, "%s", cName) <= 0)
    {
      std::cerr << "error" << std::endl;
    }
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL)
    {
      printf("can't open file\n");
      exit(1);
    }

    // gets length for spline file
    int iType, iLength;
    if (fscanf(fileSpline, "%d %d", &iLength, &iType) <= 0)
    {
      std::cerr << "error" << std::endl;
    }

    size_t i = 0;
    // saves the data to the struct
    glm::vec3 point;
    glm::vec3 first_point;
    while (fscanf(fileSpline, "%f %f %f", &point.x, &point.y, &point.z) != EOF)
    {
      if (i == 0)
      {
        first_point = point;
      }
      point = point - first_point;
      point = point + last_point;
      if (i != 0 || j == 0)
      {
        spline.points.push_back(point);
      }
      i++;
    }

    last_point = spline.points.back();
  }

  free(cName);

  return 0;
}

int initTexture(const char * imageFilename, GLuint textureHandle)
{
  // read the texture image
  ImageIO img;
  ImageIO::fileFormatType imgFormat;
  ImageIO::errorType err = img.load(imageFilename, &imgFormat);

  if (err != ImageIO::OK)
  {
    printf("Loading texture from %s failed.\n", imageFilename);
    return -1;
  }

  // check that the number of bytes is a multiple of 4
  if (img.getWidth() * img.getBytesPerPixel() % 4)
  {
    printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
    return -1;
  }

  // allocate space for an array of pixels
  int width = img.getWidth();
  int height = img.getHeight();
  unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

  // fill the pixelsRGBA array with the image pixels
  memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
  for (int h = 0; h < height; h++)
    for (int w = 0; w < width; w++)
    {
      // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
      pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
      pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
      pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
      pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

      // set the RGBA channels, based on the loaded image
      int numChannels = img.getBytesPerPixel();
      for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
        pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
    }

  // bind the texture
  glBindTexture(GL_TEXTURE_2D, textureHandle);

  // initialize the texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

  // generate the mipmaps for this texture
  glGenerateMipmap(GL_TEXTURE_2D);

  // set the texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // query support for anisotropic texture filtering
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  printf("Max available anisotropic samples: %f\n", fLargest);
  // set anisotropic texture filtering
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

  //query for any errors
  //    GLenum errCode = glGetError();
  //    if (errCode != 0)
  //    {
  //      printf("Texture initialization error. Error code: %d.\n", errCode);
  //      return -1;
  //    }

  // de-allocate the pixel array -- it is no longer needed
  delete [] pixelsRGBA;

  return 0;
}

int initCubeMap(const std::vector<std::string> &file_names)
{
  glGenTextures(1, &cube_map_id);
  glActiveTexture(GL_TEXTURE0);

  // bind the texture
  glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);

  // read the texture image
  for (unsigned int i = 0; i < file_names.size(); i++)
  {
    ImageIO img;
    ImageIO::errorType err = img.loadJPEG(file_names[i].c_str());
    if (err != ImageIO::OK)
    {
      printf("Loading texture from %s failed.\n", file_names[i].c_str());
      return -1;
    }

    // check that the number of bytes is a multiple of 4
    if (img.getWidth() * img.getBytesPerPixel() % 4)
    {
      printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", file_names[i].c_str());
      return -1;
    }

    // allocate space for an array of pixels
    int width = img.getWidth();
    int height = img.getHeight();
    unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

    // fill the pixelsRGBA array with the image pixels
    memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
    for (int h = 0; h < height; h++)
    {
      for (int w = 0; w < width; w++)
      {
        // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
        pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
        pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
        pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
        pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

        // set the RGBA channels, based on the loaded image
        int numChannels = img.getBytesPerPixel();
        for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
          pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
      }
    }

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, img.getWidth(), img.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

    // de-allocate the pixel array -- it is no longer needed
    delete [] pixelsRGBA;
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  return 0;
}

void setTextureUnit(GLint unit)
{
  glActiveTexture(unit); // select the active texture unit
  GLint h_textureImage = glGetAttribLocation(texture_pipeline.GetProgramHandle(), "textureImage");
  // deem the shader variable "textureImage" to read from texture unit "unit"
  glUniform1i(h_textureImage, unit - GL_TEXTURE0);
}

// write a screenshot to the specified filename

void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);
  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
  {
    std::cout << "File " << filename << " saved successfully." << std::endl;
  }
  else
  {
    std::cout << "Failed to save file " << filename << '.' << std::endl;
  }

  delete [] screenshotData;
}

void drawCubeMap()
{
  glDepthMask(GL_FALSE);
  cube_map_pipeline.Bind();
  glBindVertexArray(cube_map_vao);

  // Set model view matrix for shaders
  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
  openGLMatrix.PushMatrix();

  openGLMatrix.Rotate(90, 1, 0, 0);
  openGLMatrix.Scale(1, -1, 1);
  GLfloat modelViewMatrix[16];
  openGLMatrix.GetMatrix(modelViewMatrix);
  cube_map_pipeline.SetModelViewMatrix(modelViewMatrix);

  glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glDepthMask(GL_TRUE);

  openGLMatrix.PopMatrix();
}

void drawCrossSection()
{
  texture_pipeline.Bind();

  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
  GLfloat modelViewMatrix[16];
  openGLMatrix.GetMatrix(modelViewMatrix);
  texture_pipeline.SetModelViewMatrix(modelViewMatrix);

  glBindTexture(GL_TEXTURE_2D, metal_texture_id);
  // left track
  glBindVertexArray(left_cross_section_vao);
  glDrawArrays(GL_TRIANGLES, 0, left_cross_section_vertices.size());

  // right track
  glBindVertexArray(right_cross_section_vao);
  glDrawArrays(GL_TRIANGLES, 0, right_cross_section_vertices.size());

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void drawCrossBars()
{
  texture_pipeline.Bind();

  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
  GLfloat modelViewMatrix[16];
  openGLMatrix.GetMatrix(modelViewMatrix);
  texture_pipeline.SetModelViewMatrix(modelViewMatrix);

  // bars
  glBindTexture(GL_TEXTURE_2D, wood_texture_id);
  glBindVertexArray(cross_bars_vao);
  glDrawArrays(GL_TRIANGLES, 0, cross_bar_vertices.size());

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void pushFaceVertices(std::vector<GLfloat> &vertices, const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3)
{
  vertices.push_back(v0.x);
  vertices.push_back(v0.y);
  vertices.push_back(v0.z);

  vertices.push_back(v1.x);
  vertices.push_back(v1.y);
  vertices.push_back(v1.z);

  vertices.push_back(v2.x);
  vertices.push_back(v2.y);
  vertices.push_back(v2.z);

  vertices.push_back(v2.x);
  vertices.push_back(v2.y);
  vertices.push_back(v2.z);

  vertices.push_back(v3.x);
  vertices.push_back(v3.y);
  vertices.push_back(v3.z);

  vertices.push_back(v0.x);
  vertices.push_back(v0.y);
  vertices.push_back(v0.z);
}

void pushFaceUV(std::vector<GLfloat> &uv)
{
  uv.push_back(0.0);
  uv.push_back(0.0);

  uv.push_back(1.0);
  uv.push_back(0.0);

  uv.push_back(1.0);
  uv.push_back(1.0);

  uv.push_back(1.0);
  uv.push_back(1.0);

  uv.push_back(0.0);
  uv.push_back(1.0);

  uv.push_back(0.0);
  uv.push_back(0.0);
}

void setCubeMap()
{
  // ================ cube map
  // Generate and bind Vertex Array Object
  glGenVertexArrays(1, &cube_map_vao);
  glBindVertexArray(cube_map_vao);

  for (auto &f : cube_map_vertices)
  {
    f *= 1000;
  }

  // Generate, bind and send vertex Vertex Buffer Object to shaders
  GLuint cube_map_vertexBufferName;
  glGenBuffers(1, &cube_map_vertexBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, cube_map_vertexBufferName);
  glBufferData(GL_ARRAY_BUFFER, cube_map_vertices.size() * sizeof (GLfloat), &cube_map_vertices[0], GL_STATIC_DRAW);
  GLuint cube_map_posLocation = glGetAttribLocation(cube_map_pipeline.GetProgramHandle(), "position");
  glVertexAttribPointer(cube_map_posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexArrayAttrib(cube_map_vao, cube_map_posLocation);

  std::vector<std::string> file_names = {"./arrakisday_rt.jpg", "./arrakisday_lf.jpg", "./arrakisday_dn.jpg", "./arrakisday_up.jpg", "./arrakisday_bk.jpg", "./arrakisday_ft.jpg"};

  if (initCubeMap(file_names) != 0)
  {
    std::cerr << "ggg " << std::endl;
  }
}

void setSplinePoints()
{
  // generate splines vertices
  std::cout << "spline.points.size() " << spline.points.size() << std::endl;
  glm::vec3 p0 = spline.points[0];
  glm::vec3 p_last = spline.points[spline.points.size() - 1];

  glm::vec3 p1, p2, p3, p4;
  for (size_t j = 0; j < spline.points.size() - 1; j++)
  {
    if (j == 0)
    {
      p1 = p0;
    }
    else
    {
      p1 = spline.points[j - 1];
    }
    p2 = spline.points[j];
    p3 = spline.points[j + 1];
    if (j == spline.points.size() - 2)
    {
      p4 = p_last;
    }
    else
    {
      p4 = spline.points[j + 2];
    }

    //            std::cout << "p1: " << p1.x << " " << p1.y << " " << p1.z << std::endl;
    //            std::cout << "p2: " << p2.x << " " << p2.y << " " << p2.z << std::endl;
    //            std::cout << "p3: " << p3.x << " " << p3.y << " " << p3.z << std::endl;
    //            std::cout << "p4: " << p4.x << " " << p4.y << " " << p4.z << std::endl;
    CatmullRom catmull_rom(p1, p2, p3, p4, 0.5);
    std::vector<glm::vec3> spline_points = catmull_rom.subDivide(0.0, 1.0, divide_threshold);

    // remove repeated points 
    if (j != 0)
    {
      spline_points.erase(spline_points.begin());
    }

    for (const glm::vec3 &p : spline_points)
    {
      spline_vertices.push_back(p.x);
      spline_vertices.push_back(p.y);
      spline_vertices.push_back(p.z);
      //        std::cout << p.x_ << " " << p.y_ << " " << p.z_ << std::endl;
    }
  }
  num_spline_points = spline_vertices.size() / 3;
  std::cout << "num_spline_points: " << num_spline_points << std::endl;
}

void setCrossSectionVertices()
{
  for (size_t i = 0; i < num_spline_points; i++)
  {
    glm::vec3 p(spline_vertices[i * 3], spline_vertices[i * 3 + 1], spline_vertices[i * 3 + 2]);

    glm::vec3 next_p;
    if (i == num_spline_points - 1)
    {
      next_p.x = spline_vertices[(i - 1) * 3];
      next_p.y = spline_vertices[(i - 1) * 3 + 1];
      next_p.z = spline_vertices[(i - 1) * 3 + 2];
    }
    else
    {
      next_p.x = spline_vertices[(i + 1) * 3];
      next_p.y = spline_vertices[(i + 1) * 3 + 1];
      next_p.z = spline_vertices[(i + 1) * 3 + 2];
    }

    glm::vec3 n;
    glm::vec3 t;
    if (i == num_spline_points - 1)
    {
      t = normalize(p - next_p);
    }
    else
    {
      t = normalize(next_p - p);
    }
    glm::vec3 b;
    if (i == 0)
    {
      n = glm::vec3(0, 0, 1);
    }
    else
    {
      n = glm::normalize(glm::cross(spline_b[i - 1], t));
    }
    b = glm::normalize(glm::cross(t, n));

    spline_n.push_back(n);
    spline_t.push_back(t);
    spline_b.push_back(b);
  }

  std::vector<GLfloat> uvData;
  std::vector<GLfloat> bar_uvData;
  const size_t cross_section_k = num_spline_points / 100;
  for (size_t i = 0; i < num_spline_points - 1; i++)
  {
    if (i % cross_section_k == 0)
    {
      glm::vec3 p(spline_vertices[i * 3], spline_vertices[i * 3 + 1], spline_vertices[i * 3 + 2]);
      glm::vec3 next_p(spline_vertices[(i + cross_section_k) * 3], spline_vertices[(i + cross_section_k) * 3 + 1], spline_vertices[(i + cross_section_k) * 3 + 2]);

      glm::vec3 left_v0 = p - alpha * spline_n[i] - beta * spline_b[i];
      glm::vec3 left_v1 = p + alpha * spline_n[i] - beta * spline_b[i];
      glm::vec3 left_v2 = p + alpha * spline_n[i] - cross_section_width * beta * spline_b[i];
      glm::vec3 left_v3 = p - alpha * spline_n[i] - cross_section_width * beta * spline_b[i];

      glm::vec3 left_v4 = next_p - alpha * spline_n[i + cross_section_k] - beta * spline_b[i + cross_section_k];
      glm::vec3 left_v5 = next_p + alpha * spline_n[i + cross_section_k] - beta * spline_b[i + cross_section_k];
      glm::vec3 left_v6 = next_p + alpha * spline_n[i + cross_section_k] - cross_section_width * beta * spline_b[i + cross_section_k];
      glm::vec3 left_v7 = next_p - alpha * spline_n[i + cross_section_k] - cross_section_width * beta * spline_b[i + cross_section_k];

      pushFaceVertices(left_cross_section_vertices, left_v0, left_v1, left_v2, left_v3);
      pushFaceVertices(left_cross_section_vertices, left_v0, left_v4, left_v5, left_v1);
      pushFaceVertices(left_cross_section_vertices, left_v1, left_v5, left_v6, left_v2);
      pushFaceVertices(left_cross_section_vertices, left_v2, left_v6, left_v7, left_v3);
      pushFaceVertices(left_cross_section_vertices, left_v7, left_v4, left_v0, left_v3);

      // right
      glm::vec3 right_v0 = p - alpha * spline_n[i] + cross_section_width * beta * spline_b[i];
      glm::vec3 right_v1 = p + alpha * spline_n[i] + cross_section_width * beta * spline_b[i];
      glm::vec3 right_v2 = p + alpha * spline_n[i] + beta * spline_b[i];
      glm::vec3 right_v3 = p - alpha * spline_n[i] + beta * spline_b[i];

      glm::vec3 right_v4 = next_p - alpha * spline_n[i + cross_section_k] + cross_section_width * beta * spline_b[i + cross_section_k];
      glm::vec3 right_v5 = next_p + alpha * spline_n[i + cross_section_k] + cross_section_width * beta * spline_b[i + cross_section_k];
      glm::vec3 right_v6 = next_p + alpha * spline_n[i + cross_section_k] + beta * spline_b[i + cross_section_k];
      glm::vec3 right_v7 = next_p - alpha * spline_n[i + cross_section_k] + beta * spline_b[i + cross_section_k];

      pushFaceVertices(right_cross_section_vertices, right_v0, right_v1, right_v2, right_v3);
      pushFaceVertices(right_cross_section_vertices, right_v0, right_v4, right_v5, right_v1);
      pushFaceVertices(right_cross_section_vertices, right_v1, right_v5, right_v6, right_v2);
      pushFaceVertices(right_cross_section_vertices, right_v2, right_v6, right_v7, right_v3);
      pushFaceVertices(right_cross_section_vertices, right_v7, right_v4, right_v0, right_v3);

      glm::vec3 bar_v1 = right_v0 + 0.4 * spline_b[i];
      glm::vec3 bar_v0 = bar_v1 - 0.1 * spline_n[i];
      glm::vec3 bar_v2 = left_v3 - 0.4 * spline_b[i];
      glm::vec3 bar_v3 = bar_v2 - 0.1 * spline_n[i];

      glm::vec3 bar_v5 = bar_v1 + 0.3 * (right_v4 - right_v0);
      glm::vec3 bar_v4 = bar_v5 - 0.1 * spline_n[i + 1];
      glm::vec3 bar_v6 = bar_v2 + 0.3 * (left_v7 - left_v3);
      glm::vec3 bar_v7 = bar_v6 - 0.1 * spline_n[i + 1];

      pushFaceVertices(cross_bar_vertices, bar_v0, bar_v1, bar_v2, bar_v3);
      pushFaceVertices(cross_bar_vertices, bar_v0, bar_v4, bar_v5, bar_v1);
      pushFaceVertices(cross_bar_vertices, bar_v1, bar_v5, bar_v6, bar_v2);
      pushFaceVertices(cross_bar_vertices, bar_v2, bar_v6, bar_v7, bar_v3);
      pushFaceVertices(cross_bar_vertices, bar_v7, bar_v4, bar_v0, bar_v3);
      pushFaceVertices(cross_bar_vertices, bar_v4, bar_v7, bar_v6, bar_v5);

      for (size_t i = 0; i < 6; i++)
      {
        pushFaceUV(bar_uvData);
      }
    }

    for (size_t i = 0; i < 5; i++)
    {
      pushFaceUV(uvData);
    }
  }

  glGenTextures(1, &metal_texture_id);
  if (initTexture("./MetalDsk.jpg", metal_texture_id) != 0)
  {
    std::cerr << "loading metal_texture_id error" << std::endl;
    exit(EXIT_FAILURE);
  }

  glGenTextures(1, &wood_texture_id);
  if (initTexture("./wood.jpg", wood_texture_id) != 0)
  {
    std::cerr << "loading wood_texture_id error" << std::endl;
    exit(EXIT_FAILURE);
  }

  setTextureUnit(GL_TEXTURE0);

  // left track
  // Generate and bind Vertex Array Object
  glGenVertexArrays(1, &left_cross_section_vao);
  glBindVertexArray(left_cross_section_vao);

  // Generate, bind and send vertex Vertex Buffer Object to shaders
  GLuint left_vertexBufferName;
  glGenBuffers(1, &left_vertexBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, left_vertexBufferName);
  glBufferData(GL_ARRAY_BUFFER, left_cross_section_vertices.size() * sizeof (GLfloat), &left_cross_section_vertices[0], GL_STATIC_DRAW);
  GLuint left_posLocation = glGetAttribLocation(texture_pipeline.GetProgramHandle(), "position");
  glVertexAttribPointer(left_posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(left_posLocation);

  // Generate, bind and send uv Vertex Buffer Object to shaders for texture mapping
  GLuint left_uvbuffer;
  glGenBuffers(1, &left_uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, left_uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, uvData.size() * sizeof (GLfloat), &uvData[0], GL_STATIC_DRAW);
  GLuint left_uvLocation = glGetAttribLocation(texture_pipeline.GetProgramHandle(), "texCoord");
  glVertexAttribPointer(left_uvLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(left_uvLocation);

  // right track
  // Generate and bind Vertex Array Object
  glGenVertexArrays(1, &right_cross_section_vao);
  glBindVertexArray(right_cross_section_vao);

  // Generate, bind and send vertex Vertex Buffer Object to shaders
  GLuint right_vertexBufferName;
  glGenBuffers(1, &right_vertexBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, right_vertexBufferName);
  glBufferData(GL_ARRAY_BUFFER, right_cross_section_vertices.size() * sizeof (GLfloat), &right_cross_section_vertices[0], GL_STATIC_DRAW);
  GLuint right_posLocation = glGetAttribLocation(texture_pipeline.GetProgramHandle(), "position");
  glVertexAttribPointer(right_posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(right_posLocation);

  // Generate, bind and send uv Vertex Buffer Object to shaders for texture mapping
  GLuint right_uvbuffer;
  glGenBuffers(1, &right_uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, right_uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, uvData.size() * sizeof (GLfloat), &uvData[0], GL_STATIC_DRAW);
  GLuint right_uvLocation = glGetAttribLocation(texture_pipeline.GetProgramHandle(), "texCoord");
  glVertexAttribPointer(right_uvLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(right_uvLocation);

  // cross bars
  // Generate and bind Vertex Array Object
  glGenVertexArrays(1, &cross_bars_vao);
  glBindVertexArray(cross_bars_vao);

  // Generate, bind and send vertex Vertex Buffer Object to shaders
  GLuint cross_bar_vertexBufferName;
  glGenBuffers(1, &cross_bar_vertexBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, cross_bar_vertexBufferName);
  glBufferData(GL_ARRAY_BUFFER, cross_bar_vertices.size() * sizeof (GLfloat), &cross_bar_vertices[0], GL_STATIC_DRAW);
  GLuint cross_bar_posLocation = glGetAttribLocation(texture_pipeline.GetProgramHandle(), "position");
  glVertexAttribPointer(cross_bar_posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(cross_bar_posLocation);

  // Generate, bind and send uv Vertex Buffer Object to shaders for texture mapping
  GLuint cross_bar_uvbuffer;
  glGenBuffers(1, &cross_bar_uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, cross_bar_uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, bar_uvData.size() * sizeof (GLfloat), &bar_uvData[0], GL_STATIC_DRAW);
  GLuint cross_bar_uvLocation = glGetAttribLocation(texture_pipeline.GetProgramHandle(), "texCoord");
  glVertexAttribPointer(cross_bar_uvLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(cross_bar_uvLocation);
}

void setCamera()
{
  glm::vec3 current_up = spline_n[current_spline_point_index];
  glm::vec3 current_eye;
  current_eye.x = spline_vertices[current_spline_point_index * 3];
  current_eye.y = spline_vertices[current_spline_point_index * 3 + 1];
  current_eye.z = spline_vertices[current_spline_point_index * 3 + 2];
  current_eye = current_eye + spline_n[current_spline_point_index];
  glm::vec3 current_focus = current_eye + spline_t[current_spline_point_index];
  openGLMatrix.LookAt(current_eye.x, current_eye.y, current_eye.z, current_focus.x, current_focus.y, current_focus.z, current_up.x, current_up.y, current_up.z);

  const clock_t current_time = clock();
  float delta_t = static_cast<float> (current_time - last_time) / CLOCKS_PER_SEC;
  float speed = delta_t * std::sqrt(2 * g * (max_z - current_eye.z));
  current_spline_point_index += std::ceil(speed);
  if (current_spline_point_index >= num_spline_points)
  {
    current_spline_point_index = 0;
  }

  last_time = current_time;
}

void displayFunc()
{
  // Clear the scene
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  GLfloat projectionMatrix[16];
  openGLMatrix.SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix.GetMatrix(projectionMatrix);
  texture_pipeline.Bind();
  texture_pipeline.SetProjectionMatrix(projectionMatrix);
  cube_map_pipeline.Bind();
  cube_map_pipeline.SetProjectionMatrix(projectionMatrix);

  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
  openGLMatrix.LoadIdentity();
  setCamera();

  // Draw cube map
  drawCubeMap();
  drawCrossSection();
  drawCrossBars();

  glutSwapBuffers(); // Swap buffers for double buffering
}

void idleFunc()
{
  // Uncomment the following to enable saving animation images, 
//  char screenName[25];
//  sprintf(screenName, "%04lu", screenShotCount);
//  saveScreenshot(("animation/" + std::string(screenName) + ".jpg").c_str());
//  screenShotCount++;

  // make the screen update 
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  // Setup perspective matrix
  openGLMatrix.SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix.LoadIdentity();
  openGLMatrix.Perspective(60.0, 1.0 * w / h, 0.01, 1800.0);
  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
      break;
    case ' ':
      // Reset
      current_spline_point_index = 0;
      std::cout << "You pressed the spacebar." << std::endl;
      break;

    case 'x':
      // Take a screenshot
      saveScreenshot("screenshot.jpg");
      break;
  }
}

void initScene()
{
  cube_map_pipeline.Init("../openGLHelper-starterCode", "cubeMap.vertexShader.glsl", "cubeMap.fragmentShader.glsl");
  texture_pipeline.Init("../openGLHelper-starterCode", "texture.vertexShader.glsl", "texture.fragmentShader.glsl");

  setCubeMap();
  setSplinePoints();
  setCrossSectionVertices();

  // Enable depth testing so that the hidden scene will not be drawn.
  glEnable(GL_DEPTH_TEST);

  // Background color
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

int main(int argc, char ** argv)
{
  if (argc < 2)
  {
    printf("usage: %s <trackfile>\n", argv[0]);
    exit(0);
  }

  // load the splines from the provided filename
  loadSplines(argv[1]);

  printf("Loaded %lu spline(s).\n", numSplines);

  // initialize openGL
  std::cout << "Initializing GLUT..." << std::endl;
  glutInit(&argc, argv);
  // Use core profile
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

  std::cout << "Initializing OpenGL..." << std::endl;

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);
  glutCreateWindow(windowTitle);

  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  // To ensure that all extensions with valid entry points will be exposed.
  glewExperimental = GL_TRUE;
  GLint result = glewInit();
  if (result != GLEW_OK)
  {
    std::cout << "error: " << glewGetErrorString(result) << std::endl;
    exit(EXIT_FAILURE);
  }

  GLenum errCode = glGetError();
  if (errCode != 0)
  {
    printf("ggg Error code: %d.\n", errCode); // TODO: why???
  }

  // do initialization
  initScene();

  last_time = clock();
  // sink forever into the glut loop
  glutMainLoop();

  return 0;
}