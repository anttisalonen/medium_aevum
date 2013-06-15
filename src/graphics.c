#include <unistd.h>
#include <assert.h>

#include <SDL.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include "graphics.h"

struct graphics {
	int width;
	int height;
};

static int init_sdl(int width, int height)
{
	int err;
	const SDL_VideoInfo* info;
	SDL_Surface* surface;

	err = SDL_Init(SDL_INIT_EVERYTHING);
	if(err) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return 1;
	}

	info = SDL_GetVideoInfo();
	if(!info) {
		fprintf(stderr, "SDL_GetVideoInfo: %s\n", SDL_GetError());
		return 1;
	}

	surface = SDL_SetVideoMode(width, height, info->vfmt->BitsPerPixel, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL);
	if(!surface) {
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_WM_SetCaption("Medium Aevum", NULL);

	return 0;
}

static int load_shader(const char* src, GLenum type)
{
	GLuint shader;
	GLint compiled;

	shader = glCreateShader(type);

	if(shader == 0)
		return 0;

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if(!compiled) {
		GLint infolen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolen);
		if(infolen > 1) {
			char* infolog = malloc(infolen);
			glGetShaderInfoLog(shader, infolen, NULL, infolog);
			fprintf(stderr, "Error compiling %s shader: %s\n",
					type == GL_VERTEX_SHADER ?
					"vertex" : "fragment",
					infolog);
			free(infolog);
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static char* load_text_file(const char* filename)
{
	FILE* f = fopen(filename, "r");
	long length;
	char* buffer = NULL;
	if(!f)
		return NULL;

	fseek(f, 0, SEEK_END);
	length = ftell(f);
	fseek(f, 0, SEEK_SET);
	if(length > 0) {
		buffer = malloc(length + 1);
		if(buffer) {
			if(fread(buffer, 1, length, f) != length) {
				free(buffer);
				return NULL;
			}
		}
		buffer[length] = 0;
	}
	fclose(f);
	return buffer;
}

static int init_program(void)
{
	GLuint vshader;
	GLuint fshader;
	GLuint programobj;
	GLint linked;
	char* vshader_src;
	char* fshader_src;
	GLenum glewerr;

	glewerr = glewInit();
	if (glewerr != GLEW_OK) {
		fprintf(stderr, "Unable to initialise GLEW.\n");
		return 0;
	}
	if (!GLEW_VERSION_2_1) {
		fprintf(stderr, "OpenGL 2.1 not supported.\n");
		return 0;
	}

	vshader_src = load_text_file("share/shader.vert");
	if(!vshader_src)
		return 0;

	fshader_src = load_text_file("share/shader.frag");
	if(!fshader_src) {
		free(vshader_src);
		return 0;
	}

	vshader = load_shader(vshader_src, GL_VERTEX_SHADER);
	fshader = load_shader(fshader_src, GL_FRAGMENT_SHADER);

	free(vshader_src);
	free(fshader_src);

	if(!vshader || !fshader)
		return 0;

	programobj = glCreateProgram();
	if(!programobj)
		return 0;

	glAttachShader(programobj, vshader);
	glAttachShader(programobj, fshader);

	glBindAttribLocation(programobj, 0, "vPosition");

	glLinkProgram(programobj);

	glGetProgramiv(programobj, GL_LINK_STATUS, &linked);
	if(!linked) {
		GLint infolen = 0;
		glGetProgramiv(programobj, GL_INFO_LOG_LENGTH, &infolen);
		if(infolen > 1) {
			char* infolog = malloc(infolen);
			glGetProgramInfoLog(programobj, infolen, NULL, infolog);
			fprintf(stderr, "Error linking shader: %s\n", infolog);
			free(infolog);
		}
		glDeleteProgram(programobj);
		return 0;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	return programobj;
}

static int draw_triangle(int width, int height)
{
	int program;

	program = init_program();
	if(!program)
		return 1;

	GLfloat vertices[] = {0.0f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f};

	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	SDL_GL_SwapBuffers();
	sleep(2);
	return 0;
}

static void cleanup_sdl(void)
{
	SDL_Quit();
}


graphics* graphics_init(int width, int height)
{
	if(init_sdl(width, height)) {
		return NULL;
	}
	graphics* g = malloc(sizeof(graphics));
	assert(g);
	g->width = width;
	g->height = height;
	return g;
}

int graphics_draw(graphics* g)
{
	assert(g);
	return draw_triangle(g->width, g->height);
}

void graphics_cleanup(graphics* g)
{
	cleanup_sdl();
	free(g);
}


