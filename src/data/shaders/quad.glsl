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
uniform sampler2D u_blur;
uniform vec2 u_resolution;
uniform float u_time;

float rand(vec2 p)
{
  vec2 k1 = vec2(23.14069263277926, 2.665144142690225);
  return fract(cos(dot(p, k1)) * 12345.6789);
}

void main()
{
  vec2 curvature = vec2(2.25, 2.0);
  vec2 uvc = vec2(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0);
  vec2 offset = abs(uvc.yx) / vec2(curvature.x, curvature.y);
  uvc = uvc + uvc * offset * offset;
  uvc = uvc * 0.5 + 0.5;

  vec3 col = texture(u_texture, uvc).rgb;
  vec3 blur = texture(u_blur, uvc).rgb;

  vec3 lines = vec3(1.0);
  float intensity = sin(uvc.x * 1006.0 * 3.14 * 2.5);
  intensity = ((0.5 * intensity) + 0.5) * 0.9 + 0.1;
  lines *= vec3(pow(intensity, 0.05)); // .1
  intensity = sin(uvc.y * 650.0 * 3.14 * 2.5);
  intensity = ((0.5 * intensity) + 0.5) * 0.9 + 0.1;
  lines *= vec3(pow(intensity, 0.05)); // .1

  color = vec4((col + blur) * lines, 1.0);

  // film grain
  vec2 uvr = uv;
  uvr.y *= rand(vec2(uvr.y, u_time));
  color.rgb += color.rgb * (rand(uvr) * 0.5);

  // rolling scanlines
  float y = ((gl_FragCoord.y + u_time * 20.0) / 650.0);
  color.rgb *= 1.0 - 0.25 * (sin(y * 64.0) * 0.5 + 0.5);

  // TODO: work around this
  if (uvc.x < 0 || uvc.x > 1.0 || uvc.y < 0 || uvc.y > 1.0)
    color = vec4(0.0);
}
#END FS