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
	vec2 light = vec2(22.5 + sin(time / 2000.0) * 42.5, 28.0 + cos(time / 2000.0) * 48.0);

	vec2 light_dir = normalize(light - floor(WorldPos) + 0.5);

	// store light info in a vector
	vec4 c;
	c.x = texture(right, TexCoord).x;
	c.y = texture(top, TexCoord).x;
	c.z = texture(left, TexCoord).x;
	c.w = texture(bottom, TexCoord).x;
	// use c/2 to restrict to [0.0, 0.5] and get a plausible projected area
	vec4 a = (c / 2.0) / (c / -2.0 + 1.0);
	// assume normal is a function of projected area
	vec4 n = 1 / (a + 1.0);

	// weighted sum, times 2 to make up for previous down scaling
	float lightness = 2 * (
		a.x * clamp(light_dir.x * n.x, 0.0, 1.0) +
		a.y * clamp(light_dir.y * n.y, 0.0, 1.0) +
		a.z * clamp(light_dir.x * -n.z, 0.0, 1.0) +
		a.w * clamp(light_dir.y * -n.w, 0.0, 1.0)
	);

	outColor = texture(ambient, TexCoord).a * vec4(vec3(lightness), 1.0);
}
