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

SDL_Surface* load_surface(const std::string& filename)
{
	SDL_Surface* surface = IMG_Load(filename.c_str());
	if (surface == nullptr)
		throw std::runtime_error("IMG_Load failed: " + std::string(IMG_GetError()));
	return surface;
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

	// variable positions
	GLint position_a = glGetAttribLocation(program, "position");
	GLint tex_coord_a = glGetAttribLocation(program, "tex_coord");
	GLint window_u = glGetUniformLocation(program, "window");
	GLint camera_u = glGetUniformLocation(program, "camera");
	GLint time_u = glGetUniformLocation(program, "time");

	GLint ambient_u = glGetUniformLocation(program, "ambient");
	GLint right_u = glGetUniformLocation(program, "right");
	GLint top_u = glGetUniformLocation(program, "top");
	GLint left_u = glGetUniformLocation(program, "left");
	GLint bottom_u = glGetUniformLocation(program, "bottom");

	// create vertex array and set active
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// create vertex buffer and set active
	GLuint vbo;
	glGenBuffers(1, &vbo);

	SDL_Surface* ambient = load_surface("storm_eagle_ambient.png");
	SDL_Surface* right = load_surface("storm_eagle_right.png");
	SDL_Surface* top = load_surface("storm_eagle_top.png");
	SDL_Surface* left = load_surface("storm_eagle_left.png");
	SDL_Surface* bottom = load_surface("storm_eagle_bottom.png");

	GLfloat w = ambient->w;
	GLfloat h = ambient->h;

	GLfloat vertices[] =
	{
		0.f, 0.f, 0.f, 1.f,
		  w, 0.f, 1.f, 1.f,
		  w,   h, 1.f, 0.f,
		0.f,   h, 0.f, 0.f,
	};

	// load data
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// describe position attributes
	glEnableVertexAttribArray(position_a);
	glVertexAttribPointer(position_a, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	// describe texture coordinate attributes
	glVertexAttribPointer(tex_coord_a, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(tex_coord_a);

	GLuint textures[5];
	glGenTextures(5, textures);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ambient->w, ambient->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ambient->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, right->w, right->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, right->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, top->w, top->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, top->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, left->w, left->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, left->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bottom->w, bottom->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bottom->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glUniform1i(ambient_u, 0);
	glUniform1i(right_u, 1);
	glUniform1i(top_u, 2);
	glUniform1i(left_u, 3);
	glUniform1i(bottom_u, 4);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	unsigned int last_time = SDL_GetTicks();
	unsigned int now;
	unsigned int frame_time;

	// precompute scale to get pixel-perfect display
	float scale = 4.f;
	GLfloat window_scale[2] = {(2.f * scale) / width, (2.f * scale) / height};
	glUniform2fv(window_u, 1, window_scale);

	// camera position in pixels
	GLfloat camera[2] = {22.f, 28.f};
	glUniform2fv(camera_u, 1, camera);

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

	glDeleteTextures(5, textures);

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
