#version 150

in vec4 col; // gradient
in vec2 UV; // texture

uniform vec4 fCol; // wireframe color or triangle color
uniform sampler2D textureSampler; // texture

void main()
{
  // compute the final pixel color
  gl_FragColor = vec4(texture(textureSampler, UV).rgb, 0.0f) * fCol * col;
}

