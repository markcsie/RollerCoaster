#include <iostream>
#include "openGLHeader.h"
#include "basicPipelineProgram.h"

int BasicPipelineProgram::Init(const char * shaderBasePath) {
  if (BuildShadersFromFiles(shaderBasePath, "basic.vertexShader.glsl", "basic.fragmentShader.glsl") != 0) {
    std::cout << "Failed to build the pipeline program." << std::endl;
    return 1;
  }
  std::cout << "Successfully built the pipeline program." << std::endl;
  return 0;
}

void BasicPipelineProgram::SetModelViewMatrix(const GLfloat * m) {
  // Pass "m" to the pipeline program, as the modelview matrix
  glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);
}

void BasicPipelineProgram::SetProjectionMatrix(const GLfloat * m) {
  // Pass "m" to the pipeline program, as the projection matrix
  glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, m);
}

void BasicPipelineProgram::SetFColor(const GLfloat * color) {
  // Pass "color" to the pipeline program
  glUniform4fv(h_fCol, 1, color);
}

int BasicPipelineProgram::SetShaderVariableHandles() {
  // set h_modelViewMatrix and h_projectionMatrix
  SET_SHADER_VARIABLE_HANDLE(modelViewMatrix);
  SET_SHADER_VARIABLE_HANDLE(projectionMatrix);

  SET_SHADER_VARIABLE_HANDLE(fCol);
  return 0;
}

