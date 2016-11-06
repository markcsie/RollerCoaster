#version 330 core

layout (location = 0) in vec3 position;
out vec3 TexCoords;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
  // TexCoords = position;
  TexCoords = vec3(-position.x, position.yz);
}  