#version 130

in vec2 position;
in vec3 color;
in vec2 tex_coord;

out vec3 Color;
out vec2 TexCoord;

void main()
{
	Color = color;
	TexCoord = tex_coord;

	gl_Position = vec4(position, 0.0, 1.0);
}
