#include <iostream>
#include "openGLHeader.h"
#include "basicPipelineProgram.h"

int BasicPipelineProgram::Init(const char * shaderBasePath, const std::string &vertexShaderFileName, const std::string &fragmentShaderFileName) {
  if (BuildShadersFromFiles(shaderBasePath, vertexShaderFileName.c_str(), fragmentShaderFileName.c_str()) != 0) {
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

int BasicPipelineProgram::SetShaderVariableHandles() {
  // set h_modelViewMatrix and h_projectionMatrix
  SET_SHADER_VARIABLE_HANDLE(modelViewMatrix);
  SET_SHADER_VARIABLE_HANDLE(projectionMatrix);
  return 0;
}

