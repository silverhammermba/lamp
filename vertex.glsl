#version 150

in vec2 position;
in vec2 tex_coord;

out vec2 TexCoord;
out vec2 WorldPos;

uniform vec2 window;
uniform vec2 camera;

void main()
{
	mat4 view = mat4(1.0);
	view[0][0] = window.x;
	view[1][1] = window.y;
	view[3] = vec4(-camera * window, 0.0, 1.0);

	TexCoord = tex_coord;
	WorldPos = position; // for now

	gl_Position = view * vec4(position, 0.0, 1.0);
}
