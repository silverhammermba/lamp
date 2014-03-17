#version 150

in vec2 TexCoord;
in vec2 WorldPos;

out vec4 outColor;

uniform uint time;
uniform sampler2D ambient;
uniform sampler2D right;
uniform sampler2D top;
uniform sampler2D left;
uniform sampler2D bottom;

void main()
{
	vec2 light = vec2(22.5 + sin(time / 2000.0) * 60.0, 28.0 + cos(time / 2000.0) * 60.0);

	vec2 light_dir = normalize(light - WorldPos);
	vec2 normal = normalize(vec2(-2.0 * texture(left, TexCoord).x + 1.0, 2.0 * texture(top, TexCoord).x - 1.0));
	float lightness = clamp(dot(light_dir, normal), 0.0, 1.0);

	outColor = texture(ambient, TexCoord) * vec4(vec3(lightness), 1.0);
}
