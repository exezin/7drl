#START VS
#version 330 core

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

out vec2 uv;
out vec4 col;

uniform mat4 u_projection;
uniform mat4 u_model;
uniform vec2 u_uv;

void main()
{
  gl_Position = u_projection * u_model * vec4(in_position, 0.0, 1.0);
  uv          = in_uv + u_uv;
  col         = in_color;
}
#END VS


#START FS
#version 330 core

out vec4 color;

in vec2 uv;
in vec4 col;

uniform sampler2D u_texture;
uniform vec4 u_color;

void main()
{
  vec4 diffuse = texture(u_texture, uv).rgba;
  color = vec4(diffuse * col) * u_color;

  if (color.a < 0.01)
    discard;
}
#END FS