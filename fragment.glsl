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
	vec2 light = vec2(22.5 + sin(time / 500.0) * 22.5, 28.0 + cos(time / 500.0) * 28.0);

	vec2 light_dir = light - floor(WorldPos) + 0.5;
	light_dir = light_dir / (abs(light_dir.x) + abs(light_dir.y)); // make components sum to 1
	float lightness =
		clamp(light_dir.x, 0.0, 1.0) * texture(right, TexCoord).x +
		clamp(light_dir.y, 0.0, 1.0) * texture(top, TexCoord).x +
		clamp(-light_dir.x, 0.0, 1.0) * texture(left, TexCoord).x +
		clamp(-light_dir.y, 0.0, 1.0) * texture(bottom, TexCoord).x;

	outColor = texture(ambient, TexCoord) * vec4(vec3(lightness), 1.0);
}
