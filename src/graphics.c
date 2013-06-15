#include <assert.h>

#include <SDL.h>
#include <SDL_image.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include "graphics.h"

#define TILE_SECTOR_SIZE 16

struct graphics {
	int width;
	int height;
	map* map;
	GLuint terrain_texture;
	GLuint terrain_program;
	GLuint terrain_texture_uniform;
	GLuint terrain_camera_uniform;
	GLuint terrain_vbo[3];
	float cam_pos_x;
	float cam_pos_y;
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

static int load_texture(const SDL_Surface* surf)
{
	int hasAlpha = surf->format->BytesPerPixel == 4;
	GLuint texture;
	GLenum format;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if(hasAlpha) {
		if (surf->format->Rmask == 0x000000ff)
			format = GL_RGBA;
		else
			format = GL_BGRA;
	} else {
		if (surf->format->Rmask == 0x000000ff)
			format = GL_RGB;
		else
			format = GL_BGR;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, format,
			surf->w, surf->h, 0, format,
			GL_UNSIGNED_BYTE,
			(char*)surf->pixels);
	return texture;
}

static int load_textures(graphics* g)
{
	SDL_Surface* terrain_surface = IMG_Load("share/terrain.png");
	if(!terrain_surface)
		return 1;
	g->terrain_texture = load_texture(terrain_surface);
	SDL_FreeSurface(terrain_surface);
	g->terrain_texture_uniform = glGetUniformLocation(g->terrain_program, "sTexture");
	g->terrain_camera_uniform = glGetUniformLocation(g->terrain_program, "uCamera");
	return 0;
}

static void get_tile_texcoords(const graphics* g, int i, int j, GLfloat tile_texcoords[static 4])
{
	terrain_type tt = map_get_terrain_at(g->map, i, j);
	switch(tt) {
		case tt_forest:
			tile_texcoords[0] = 0.0f; tile_texcoords[1] = 0.0f;
			break;

		case tt_grass:
			tile_texcoords[0] = 0.5f; tile_texcoords[1] = 0.0f;
			break;

		case tt_hills:
			tile_texcoords[0] = 0.0f; tile_texcoords[1] = 0.5f;
			break;

		case tt_sea:
			tile_texcoords[0] = 0.5f; tile_texcoords[1] = 0.5f;
			break;
	}

	tile_texcoords[2] = tile_texcoords[0] + 0.5f;
	tile_texcoords[3] = tile_texcoords[1] + 0.5f;
}

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

static int load_vertices(graphics* g)
{
	GLfloat vertices[TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 12];
	GLfloat texcoord[TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 8];
	GLushort indices[TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 6];

	int i, j;
	const float zoom_factor = 0.3f;

	// for each tile
	for(j = 0; j < TILE_SECTOR_SIZE; j++) {
		for(i = 0; i < TILE_SECTOR_SIZE; i++) {
			int ind = j * TILE_SECTOR_SIZE + i;
			{
				int fi = ind * 6;
				int ii0 = 4 * (i * TILE_SECTOR_SIZE + j);
				int ii1 = ii0 + 2;
				int ii2 = ii0 + 1;
				int ii3 = ii0 + 3;
				indices[fi + 0] = ii0; indices[fi + 1] = ii2; indices[fi + 2] = ii1;
				indices[fi + 3] = ii1; indices[fi + 4] = ii2; indices[fi + 5] = ii3;
			}

			{
				int t = ind * 8;
				GLfloat tile_texcoords[4];
				get_tile_texcoords(g, i, j, tile_texcoords);
				texcoord[t + 0] = tile_texcoords[0]; texcoord[t + 1] = tile_texcoords[3];
				texcoord[t + 2] = tile_texcoords[2]; texcoord[t + 3] = tile_texcoords[3];
				texcoord[t + 4] = tile_texcoords[0]; texcoord[t + 5] = tile_texcoords[1];
				texcoord[t + 6] = tile_texcoords[2]; texcoord[t + 7] = tile_texcoords[1];
			}

			{
				int v = ind * 12;
				vertices[v + 0] = -1.0f + i       * zoom_factor;
				vertices[v + 1]  = -1.0f + j       * zoom_factor;
				vertices[v + 2] = 0;
				vertices[v + 3] = -1.0f + (i + 1) * zoom_factor;
				vertices[v + 4]  = -1.0f + j       * zoom_factor;
				vertices[v + 5] = 0;
				vertices[v + 6] = -1.0f + i       * zoom_factor;
				vertices[v + 7]  = -1.0f + (j + 1) * zoom_factor;
				vertices[v + 8] = 0;
				vertices[v + 9] = -1.0f + (i + 1) * zoom_factor;
				vertices[v + 10] = -1.0f + (j + 1) * zoom_factor;
				vertices[v + 11] = 0;
			}
		}
	}

	glGenBuffers(3, g->terrain_vbo);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, g->terrain_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE(vertices) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// texcoord
	glBindBuffer(GL_ARRAY_BUFFER, g->terrain_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE(texcoord) * sizeof(GLfloat), texcoord, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g->terrain_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ARRAY_SIZE(indices) * sizeof(GLushort), indices, GL_STATIC_DRAW);

	return 0;
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

	glBindAttribLocation(programobj, 0, "aPosition");
	glBindAttribLocation(programobj, 1, "aTexcoord");

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

static int draw_map(graphics* g)
{
	glViewport(0, 0, g->width, g->height);

	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(g->terrain_program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g->terrain_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i(g->terrain_texture_uniform, 0);
	glUniform2f(g->terrain_camera_uniform, g->cam_pos_x, g->cam_pos_y);

	glDrawElements(GL_TRIANGLES, TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 6, GL_UNSIGNED_SHORT, NULL);
	SDL_GL_SwapBuffers();
	return 0;
}

static void cleanup_sdl(void)
{
	SDL_Quit();
}

static int init_gl(graphics* g)
{
	g->terrain_program = init_program();
	if(!g->terrain_program)
		return 1;

	glEnable(GL_CULL_FACE);

	if(load_vertices(g))
		return 1;

	if(load_textures(g))
		return 1;

	return 0;
}

graphics* graphics_init(int width, int height, map* m)
{
	if(init_sdl(width, height)) {
		return NULL;
	}
	graphics* g = malloc(sizeof(graphics));
	assert(g);
	g->width = width;
	g->height = height;
	g->map = m;
	g->cam_pos_x = 0.0f;
	g->cam_pos_y = 0.0f;
	if(init_gl(g)) {
		cleanup_sdl();
		return NULL;
	}
	return g;
}

int graphics_draw(graphics* g)
{
	assert(g);
	return draw_map(g);
}

void graphics_move_camera(graphics* g, float x, float y)
{
	assert(g);
	g->cam_pos_x -= x;
	g->cam_pos_y += y;
}

void graphics_cleanup(graphics* g)
{
	cleanup_sdl();
	glDeleteTextures(1, &g->terrain_texture);
	free(g);
}


