#START VS
#version 330 core
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_uv;

out vec2 uv;

void main()
{
    gl_Position = vec4(in_position.x, in_position.y, 0.0, 1.0);
    uv = in_uv;
}
#END VS


#START FS
#version 330 core
out vec4 color;

in vec2 uv;

uniform sampler2D u_texture;
uniform bool u_hor;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
  vec2 tex_offset = 1.0 / textureSize(u_texture, 0);
  vec3 result = texture(u_texture, uv).rgb * weight[0];

  if (u_hor) {
    for (int i=1; i<5; i++) {
      result += texture(u_texture, uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
      result += texture(u_texture, uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    }
  } else {
    for (int i=1; i<5; i++) {
      result += texture(u_texture, uv + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
      result += texture(u_texture, uv - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    }
  }

  color = vec4(result, 1.0);
}
#END FS
