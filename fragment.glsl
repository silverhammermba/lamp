#version 130

in vec3 Color;
in vec2 TexCoord;

out vec4 outColor;

uniform sampler2D tex;

void main()
{
	outColor = texture(tex, TexCoord) * vec4(Color, 1.0);;
}
