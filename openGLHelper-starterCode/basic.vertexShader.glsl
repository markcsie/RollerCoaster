#version 150

in vec3 position;
in vec2 vertexUV;
in vec4 color;

out vec4 col;
out vec2 UV;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
  col = color;
  UV = vertexUV;
}

