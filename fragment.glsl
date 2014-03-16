#version 150

in vec2 TexCoord;
in vec2 WorldPos;

out vec4 outColor;

uniform uint time;
uniform sampler2D tex;

void main()
{
	vec2 light = vec2(10.0 + sin(time / 1000.0) * 20.0, 28.0 + cos(time / 1000.0) * 30.0);

	outColor = texture(tex, TexCoord) * vec4(vec3((50.0 - clamp(distance(light, WorldPos), 0.0, 50.0)) / 50.0), 1.0);
}
