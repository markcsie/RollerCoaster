/*
  CSCI 420 Computer Graphics, USC
  Assignment 2: Roller Coaster
  C++ starter code

  Student username: <kaichiem>
 */

#include <iostream>
#include <vector>
#include <array>
#include <cstring>

#include "openGLHeader.h"
#include "glutHeader.h"
#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"

#include "include/catmull_rom.h"

// spline struct 
// contains how many control points the spline has, and an array of control points 

std::vector<GLfloat> cube_map_vertices = {
    // Positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

//std::vector<GLfloat> cube_map_vertices = {// the idea is that all vectors have a length of 1 so that position can also be used as cubemap texture coords
//  -1, -1, -1,
//  +1, -1, -1,
//  -1, +1, -1,
//  +1, +1, -1,
//  -1, -1, +1,
//  +1, -1, +1,
//  -1, +1, +1,
//  +1, -1, +1
//};

//std::vector<GLint> cube_map_indices = {
//  7, 5, 1, // positive x
//  1, 3, 7,
//  6, 4, 0, // negative x
//  0, 2, 6,
//  6, 7, 3, // positive y
//  3, 2, 6,
//  4, 5, 1, // negative y
//  1, 7, 4,
//  4, 5, 7, // positive z
//  7, 6, 4,
//  3, 1, 0, // negative z
//  0, 2, 3,
//};

struct Spline
{
  int numControlPoints;
  std::vector<Point> points;
};

// the spline array 
std::vector<Spline> splines;
// total number of splines 
GLsizei numSplines;
std::vector<GLsizei> g_num_spline_vertices;
GLsizei numVertices;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum
{
  ROTATE, TRANSLATE, SCALE
} CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
std::array<GLfloat, 3> landRotate = {0.0f, 0.0f, 0.0f};
std::array<GLfloat, 3> landTranslate = {0.0f, 0.0f, 0.0f};
std::array<GLfloat, 3> landScale = {1.0f, 1.0f, 1.0f};

const GLuint windowWidth = 1280;
const GLuint windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";

OpenGLMatrix openGLMatrix;

BasicPipelineProgram basic_pipeline;
GLuint basic_vao;

BasicPipelineProgram texture_pipeline;
GLuint texture_vao;

BasicPipelineProgram cube_map_pipeline;
GLuint cube_map_vao;
GLuint cube_map_id;


GLuint numSplineVertices = 0;

int loadSplines(char * argv)
{
  char * cName = (char *) malloc(128 * sizeof (char));
  FILE * fileList;
  FILE * fileSpline;
  int iType, i = 0, j, iLength;

  // load the track file 
  fileList = fopen(argv, "r");
  if (fileList == NULL)
  {
    printf("can't open file\n");
    exit(1);
  }

  // stores the number of splines in a global variable 
  fscanf(fileList, "%d", &numSplines);
  splines.clear();
  splines.resize(numSplines);

  // reads through the spline files 
  for (j = 0; j < numSplines; j++)
  {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL)
    {
      printf("can't open file\n");
      exit(1);
    }

    // gets length for spline file
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    // allocate memory for all the points
    splines[j].points.clear();
    splines[j].points.resize(iLength);
    splines[j].numControlPoints = iLength;

    // saves the data to the struct
    while (fscanf(fileSpline, "%lf %lf %lf",
            &splines[j].points[i].x_,
            &splines[j].points[i].y_,
            &splines[j].points[i].z_) != EOF)
    {
      i++;
    }
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

  // query for any errors
  //  GLenum errCode = glGetError();
  //  if (errCode != 0)
  //  {
  //    printf("Texture initialization error. Error code: %d.\n", errCode);
  //    return -1;
  //  }

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
  // ... Set view and projection matrix
  glBindVertexArray(cube_map_vao);

  GLfloat projectionMatrix[16];
  openGLMatrix.SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix.GetMatrix(projectionMatrix);
  cube_map_pipeline.SetProjectionMatrix(projectionMatrix);

  // Set model view matrix for shaders
  GLfloat modelViewMatrix[16];
  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
  openGLMatrix.LoadIdentity();
  openGLMatrix.LookAt(0, 0, 50, 0, 0, 0, 0, 1, 0);

  // T R S
  openGLMatrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  openGLMatrix.Rotate(landRotate[0], 1, 0, 0);
  openGLMatrix.Rotate(landRotate[1], 0, 1, 0);
  openGLMatrix.Rotate(landRotate[2], 0, 0, 1);
  openGLMatrix.Scale(landScale[0], -landScale[1], landScale[2]);
  openGLMatrix.GetMatrix(modelViewMatrix);
  cube_map_pipeline.SetModelViewMatrix(modelViewMatrix);

  glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
//  glDrawElements(GL_TRIANGLES, cube_map_indices.size(), GL_UNSIGNED_INT, (const GLvoid *) 0);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glDepthMask(GL_TRUE);
}

void displayFunc()
{
  // Clear the scene
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw cube map
  drawCubeMap();

  basic_pipeline.Bind();
  glBindVertexArray(basic_vao);
  // Set projection matrix for shaders
  GLfloat projectionMatrix[16];
  openGLMatrix.SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix.GetMatrix(projectionMatrix);
  basic_pipeline.SetProjectionMatrix(projectionMatrix);
  //
  // Set model view matrix for shaders
  GLfloat modelViewMatrix[16];
  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
  openGLMatrix.LoadIdentity();
  openGLMatrix.LookAt(0, 0, 50, 0, 0, 0, 0, 1, 0);

  // T R S
  openGLMatrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  openGLMatrix.Rotate(landRotate[0], 1, 0, 0);
  openGLMatrix.Rotate(landRotate[1], 0, 1, 0);
  openGLMatrix.Rotate(landRotate[2], 0, 0, 1);
  openGLMatrix.Scale(landScale[0], landScale[1], landScale[2]);
  openGLMatrix.GetMatrix(modelViewMatrix);
  basic_pipeline.SetModelViewMatrix(modelViewMatrix);

  GLint index = 0;
  for (size_t i = 0; i < g_num_spline_vertices.size(); i++)
  {
    //    std::cout << "g_num_spline_vertices[i] " << g_num_spline_vertices[i] << std::endl;
    glDrawArrays(GL_LINE_STRIP, index, g_num_spline_vertices[i]);
    index += g_num_spline_vertices[i];

    openGLMatrix.Translate(5.0, 0.0, 0.0);
    openGLMatrix.GetMatrix(modelViewMatrix);
    basic_pipeline.SetModelViewMatrix(modelViewMatrix);
  }

  //  texture_pipeline.Bind();
  //  glBindVertexArray(texture_vao);
  //  texture_pipeline.SetProjectionMatrix(projectionMatrix);
  //  texture_pipeline.SetModelViewMatrix(modelViewMatrix);
  //  glDrawElements(GL_TRIANGLE_STRIP, 2 * 3, GL_UNSIGNED_INT, (const GLvoid *) 0);

  glutSwapBuffers(); // Swap buffers for double buffering
}

void idleFunc()
{
  // Uncomment the following to enable saving animation images, 
  //  char screenName[25];
  //  sprintf(screenName, "%03d", screenShotCount);
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
  openGLMatrix.Perspective(90.0, 1.0 * w / h, 0.01, 1000.0);
  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = {x - mousePos[0], y - mousePos[1]};

  switch (controlState)
  {
      // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.003f;
        landTranslate[1] -= mousePosDelta[1] * 0.003f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.03f;
      }
      break;

      // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1] * 0.1f;
        landRotate[1] += mousePosDelta[0] * 0.1f;
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1] * 0.1f;
      }
      break;

      // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // Store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // Set flags when a mouse button has been pressed or depressed

  // Keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
      break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
      break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
      break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
      break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
      break;

      // If CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
      break;
  }

  // Store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
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
      std::cout << "You pressed the spacebar." << std::endl;
      std::fill(landRotate.begin(), landRotate.end(), 0.0f);
      std::fill(landTranslate.begin(), landTranslate.end(), 0.0f);
      std::fill(landScale.begin(), landScale.end(), 1.0f);
      break;

    case 'x':
      // Take a screenshot
      saveScreenshot("screenshot.jpg");
      break;
  }
}

void initScene()
{
  // ================ cube map
  cube_map_pipeline.Init("../openGLHelper-starterCode", "cubeMap.vertexShader.glsl", "cubeMap.fragmentShader.glsl");
  // Generate and bind Vertex Array Object
  glGenVertexArrays(1, &cube_map_vao);
  glBindVertexArray(cube_map_vao);

  for (auto &f : cube_map_vertices)
  {
    f *= 100;
  }

  // Generate, bind and send vertex Vertex Buffer Object to shaders
  GLuint cube_map_vertexBufferName;
  glGenBuffers(1, &cube_map_vertexBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, cube_map_vertexBufferName);
  glBufferData(GL_ARRAY_BUFFER, cube_map_vertices.size() * sizeof (GLfloat), &cube_map_vertices[0], GL_STATIC_DRAW);
  GLuint cube_map_posLocation = glGetAttribLocation(cube_map_pipeline.GetProgramHandle(), "position");
  glVertexAttribPointer(cube_map_posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexArrayAttrib(cube_map_vao, cube_map_posLocation);
  
//  GLuint cube_map_elementbuffer;
//  glGenBuffers(1, &cube_map_elementbuffer);
//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_map_elementbuffer);
//  glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_map_indices.size() * sizeof (unsigned int), &cube_map_indices[0], GL_STATIC_DRAW);

  std::vector<std::string> file_names = {"./arrakisday_rt.jpg", "./arrakisday_lf.jpg", "./arrakisday_dn.jpg", "./arrakisday_up.jpg", "./arrakisday_bk.jpg", "./arrakisday_ft.jpg"};
//  std::vector<std::string> file_names = {"./right.jpg", "./left.jpg", "./bottom.jpg", "./top.jpg", "./back.jpg", "./front.jpg"};

  if (initCubeMap(file_names) != 0)
  {
    std::cerr << "ggg " << std::endl;
  }
  //

  std::vector<GLfloat> vertices;
  for (size_t i = 0; i < splines.size(); i++)
  {
    Point p0 = splines[i].points[0];
    Point p_last = splines[i].points[splines[i].numControlPoints - 1];

    Point p1, p2, p3, p4;
    g_num_spline_vertices.push_back(0);
    for (size_t j = 0; j < splines[i].numControlPoints - 1; j++)
    {
      if (j == 0)
      {
        p1 = p0;
      }
      else
      {
        p1 = splines[i].points[j - 1];
      }
      p2 = splines[i].points[j];
      p3 = splines[i].points[j + 1];
      if (j == splines[i].numControlPoints - 2)
      {
        p4 = p_last;
      }
      else
      {
        p4 = splines[i].points[j + 2];
      }

      //      std::cout << "p1: " << p1.x_ << " " << p1.y_ << " " << p1.z_ << std::endl;
      //      std::cout << "p2: " << p2.x_ << " " << p2.y_ << " " << p2.z_ << std::endl;
      //      std::cout << "p3: " << p3.x_ << " " << p3.y_ << " " << p3.z_ << std::endl;
      //      std::cout << "p4: " << p4.x_ << " " << p4.y_ << " " << p4.z_ << std::endl;
      CatmullRom catmull_rom(p1, p2, p3, p4, 0.5);
      std::vector<Point> spline_points = catmull_rom.subDivide(0.0, 1.0, 0.5);

      // remove repeated points 
      if (j != 0)
      {
        spline_points.erase(spline_points.begin());
      }

      for (const auto &p : spline_points)
      {
        vertices.push_back(p.x_);
        vertices.push_back(p.y_);
        vertices.push_back(p.z_);
        //        std::cout << p.x_ << " " << p.y_ << " " << p.z_ << std::endl;
        g_num_spline_vertices.back()++;
      }
    }
  }

  numVertices = vertices.size() / 3;
  std::cout << "numVertices: " << numVertices << std::endl;
  std::vector<GLfloat> colors;
  for (size_t i = 0; i < numVertices; i++)
  {
    colors.push_back(1.0);
    colors.push_back(1.0);
    colors.push_back(1.0);
    colors.push_back(0.0);
  }

  // Create shaders
  basic_pipeline.Init("../openGLHelper-starterCode", "basic.vertexShader.glsl", "basic.fragmentShader.glsl");

  // Generate and bind Vertex Array Object
  glGenVertexArrays(1, &basic_vao);
  glBindVertexArray(basic_vao);

  // Generate, bind and send vertex Vertex Buffer Object to shaders
  GLuint vertexBufferName;
  glGenBuffers(1, &vertexBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferName);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof (GLfloat), &vertices[0], GL_STATIC_DRAW);
  GLuint posLocation = glGetAttribLocation(basic_pipeline.GetProgramHandle(), "position");
  glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexArrayAttrib(basic_vao, posLocation);

  // Generate, bind and send color Vertex Buffer Object to shaders
  GLuint colorBufferName;
  glGenBuffers(1, &colorBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, colorBufferName);
  glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof (GLfloat), &colors[0], GL_STATIC_DRAW);
  GLuint colLocation = glGetAttribLocation(basic_pipeline.GetProgramHandle(), "color");
  glVertexAttribPointer(colLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexArrayAttrib(basic_vao, colLocation);

  // ====================objects with textures=====================
  texture_pipeline.Init("../openGLHelper-starterCode", "texture.vertexShader.glsl", "texture.fragmentShader.glsl");
  GLuint ground_texture_id;
  glGenTextures(1, &ground_texture_id);
  if (initTexture("./grass.jpg", ground_texture_id) != 0)
  {
    std::cerr << "loading ground_texture error" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Generate and bind Vertex Array Object
  glGenVertexArrays(1, &texture_vao);
  glBindVertexArray(texture_vao);

  setTextureUnit(GL_TEXTURE0);

  std::vector<GLfloat> texture_vertices;
  texture_vertices.push_back(-100.0);
  texture_vertices.push_back(100.0);
  texture_vertices.push_back(-10.0);

  texture_vertices.push_back(-100.0);
  texture_vertices.push_back(-100.0);
  texture_vertices.push_back(-10.0);

  texture_vertices.push_back(100.0);
  texture_vertices.push_back(-100.0);
  texture_vertices.push_back(-10.0);

  texture_vertices.push_back(100.0);
  texture_vertices.push_back(100.0);
  texture_vertices.push_back(-10.0);

  GLuint texture_vertexBufferName;
  glGenBuffers(1, &texture_vertexBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, texture_vertexBufferName);
  glBufferData(GL_ARRAY_BUFFER, texture_vertices.size() * sizeof (GLfloat), &texture_vertices[0], GL_STATIC_DRAW);
  GLuint texture_posLocation = glGetAttribLocation(texture_pipeline.GetProgramHandle(), "position");
  glVertexAttribPointer(texture_posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexArrayAttrib(texture_vao, texture_posLocation);

  std::vector<GLfloat> uvData;
  uvData.push_back(0.0);
  uvData.push_back(1.0);

  uvData.push_back(0.0);
  uvData.push_back(0.0);

  uvData.push_back(1.0);
  uvData.push_back(0.0);

  uvData.push_back(1.0);
  uvData.push_back(1.0);
  // Generate, bind and send uv Vertex Buffer Object to shaders for texture mapping
  GLuint uvbuffer;
  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, uvData.size() * sizeof (GLfloat), &uvData[0], GL_STATIC_DRAW);
  GLuint uvLocation = glGetAttribLocation(texture_pipeline.GetProgramHandle(), "texCoord");
  glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexArrayAttrib(texture_vao, uvLocation);

  // Generate and bind elements Vertex Buffer Object
  std::vector<GLuint> texture_triangleIndices;
  texture_triangleIndices.push_back(0);
  texture_triangleIndices.push_back(1);
  texture_triangleIndices.push_back(2);

  texture_triangleIndices.push_back(2);
  texture_triangleIndices.push_back(3);
  texture_triangleIndices.push_back(0);

  GLuint elementbuffer;
  glGenBuffers(1, &elementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, texture_triangleIndices.size() * sizeof (unsigned int), &texture_triangleIndices[0], GL_STATIC_DRAW);


  // Enable depth testing so that the hidden scene will not be drawn.
  glEnable(GL_DEPTH_TEST);

  // Background color
  //  glClearColor(131.0 / 255.0, 175 / 255.0, 155.0 / 255.0, 0.0);
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

  printf("Loaded %d spline(s).\n", numSplines);
  for (int i = 0; i < numSplines; i++)
  {
    printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);
  }

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
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
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

  // sink forever into the glut loop
  glutMainLoop();

  return 0;
}

