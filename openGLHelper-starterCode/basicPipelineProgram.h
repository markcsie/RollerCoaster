#include "pipelineProgram.h"

class BasicPipelineProgram : public PipelineProgram {
public:
  int Init(const char * shaderBasePath); // init the program, "shaderBasePath" is the path to the folder containing the shaders
  void SetModelViewMatrix(const float * m); // m is column-major
  void SetProjectionMatrix(const float * m); // m is column-major
  void SetFColor(const float * color);

protected:
  virtual int SetShaderVariableHandles();

  GLint h_projectionMatrix; // handle to the projectionMatrix variable in the shader
  GLint h_modelViewMatrix; // handle to the modelViewMatrix variable in the shader
  GLint h_fCol; // handle to the fCol variable in the shader
  GLint h_textureSampler; // handle to the textureSampler variable in the shader

  // note: we can use the general syntax: h_name is a handle to the shader variable "name"
};

