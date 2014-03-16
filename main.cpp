#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::cerr;
using std::endl;

// read entire contents of file into string
std::string read_file(const std::string& filename)
{
	std::string content;

	std::ifstream in(filename);
	if (!in)
		throw std::runtime_error("Failed to read file: " + filename);

	in.seekg(0, std::ios::end);
	content.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&content[0], content.size());
	in.close();

	return content;
}

// load file contents, compile shader, report errors
void load_shader(GLuint shader, const std::string& filename)
{
	std::string source = read_file(filename);
	const char* ptr = source.c_str();
	glShaderSource(shader, 1, (const GLchar**)&ptr, nullptr);
	glCompileShader(shader);
	// error check
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	GLint log_length;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0)
	{
		char buffer[log_length];
		glGetShaderInfoLog(shader, log_length, nullptr, buffer);
		cerr << buffer;
	}
	if (status == GL_FALSE)
		throw std::runtime_error("Failed to compile shader: " + filename);
}

int main(int argc, char** argv)
{
	// INIT SYSTEMS

	// start SDL
	SDL_Init(SDL_INIT_VIDEO);

	// start SDL image
	int img_flags = IMG_INIT_PNG;
	if (IMG_Init(img_flags) & img_flags != img_flags)
	{
		cerr << "IMG_Init failed: " << IMG_GetError() << endl;
		return 1;
	}

	// specify non-deprecated OpenGL context
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// window dimensions
	unsigned int width = 640;
	unsigned int height = 480;

	// create window
	SDL_Window* window = SDL_CreateWindow(
		"Sprite Scene",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		SDL_WINDOW_OPENGL
	);
	if (window == nullptr)
	{
		cerr << "SDL_CreateWindow failed: " << SDL_GetError() << endl;
		return 1;
	}

	// create context
	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (context == nullptr)
	{
		cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << endl;
		return 1;
	}

	// start GLEW
	glewExperimental = GL_TRUE;
	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		cerr << "glewInit failed: " << glewGetErrorString(error) << endl;
		return 1;
	}

	// CREATE SHADER PROGRAM

	// compile shaders
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	load_shader(vertex_shader, "vertex.glsl");

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	load_shader(fragment_shader, "fragment.glsl");

	// create program
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	GLint log_length;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0)
	{
		char buffer[log_length];
		glGetProgramInfoLog(program, log_length, nullptr, buffer);
		cerr << buffer;
	}
	if (status == GL_FALSE)
	{
		cerr << "glLinkProgram failed\n";
		return 1;
	}

	glUseProgram(program);

	// describe position attributes
	GLint pos_attrib = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(pos_attrib);
	glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	// describe texture coordinate attributes
	GLint tex_attrib = glGetAttribLocation(program, "tex_coord");
	glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(tex_attrib);

	// PREPARE RENDERING DATA
    GLuint tex;
    glGenTextures(1, &tex);

	SDL_Surface* surf = IMG_Load("cat.jpg");
	if (surf == nullptr)
	{
		cerr << "IMG_Load failed: " << IMG_GetError() << endl;
		return 1;
	}
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surf->w, surf->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surf->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// create vertex array and set active
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// create vertex buffer and set active
	GLuint vbo;
	glGenBuffers(1, &vbo);

	GLfloat w = 0.5f;
	GLfloat h = 0.5f;

	GLfloat vertices[] =
	{
		0.f, 0.f, 0.f, 0.f,
		  w, 0.f, 1.f, 0.f,
		  w,   h, 1.f, 1.f,
		0.f,   h, 0.f, 1.f,
	};

	// load data
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// uniforms
	GLint time_u = glGetUniformLocation(program, "time");

	unsigned int last_time = SDL_GetTicks();
	unsigned int now;
	unsigned int frame_time;

	// main loop
	SDL_Event event;
	bool running = true;
	while (running)
	{
		// update timers
		now = SDL_GetTicks();
		frame_time = now - last_time;
		last_time = now;

		glUniform1ui(time_u, now);

		// event handling
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				running = false;
		}

		// draw
		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		SDL_GL_SwapWindow(window);
	}

    glDeleteTextures(1, &tex);

    glDeleteProgram(program);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);

    glDeleteBuffers(1, &vbo);

    glDeleteVertexArrays(1, &vao);

	// clean up
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);

	// stop SDL image
	IMG_Quit();

	// stop SDL
	SDL_Quit();

	return 0;
}

