#include <assert.h>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include "graphics.h"

#define TILE_SECTOR_SIZE 24

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

#define ZOOM_FACTOR (60.0f)

// tile sector
typedef struct {
	GLuint vbos[3];
} tile_sector;

void cleanup_tile_sector(tile_sector* s)
{
	if(s->vbos[0])
		glDeleteBuffers(3, s->vbos);
}

static void get_tile_texcoords(const map* m, int i, int j, GLfloat tile_texcoords[static 4])
{
	terrain_type tt = map_get_terrain_at(m, i, j);
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

static int load_map_vertices(const map* m, GLuint vbos[static 3],
		int map_x, int map_y,
		float offset_x, float offset_y)
{
	GLfloat vertices[TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 12];
	GLfloat texcoord[TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 8];
	GLushort indices[TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 6];

	int i, j;

	// for each tile
	for(j = 0; j < TILE_SECTOR_SIZE; j++) {
		for(i = 0; i < TILE_SECTOR_SIZE; i++) {
			int ind = j * TILE_SECTOR_SIZE + i;
			{
				int fi = ind * 6;
				int ii0 = 4 * (i * TILE_SECTOR_SIZE + j);
				int ii1 = ii0 + 1;
				int ii2 = ii0 + 2;
				int ii3 = ii0 + 3;
				indices[fi + 0] = ii0; indices[fi + 1] = ii2; indices[fi + 2] = ii1;
				indices[fi + 3] = ii1; indices[fi + 4] = ii2; indices[fi + 5] = ii3;
			}

			{
				int t = ind * 8;
				GLfloat tile_texcoords[4];
				get_tile_texcoords(m, map_x + i, map_y + j, tile_texcoords);
				texcoord[t + 0] = tile_texcoords[0]; texcoord[t + 1] = tile_texcoords[3];
				texcoord[t + 2] = tile_texcoords[2]; texcoord[t + 3] = tile_texcoords[3];
				texcoord[t + 4] = tile_texcoords[0]; texcoord[t + 5] = tile_texcoords[1];
				texcoord[t + 6] = tile_texcoords[2]; texcoord[t + 7] = tile_texcoords[1];
			}

			{
				int v = ind * 12;
				vertices[v + 0] = i + offset_x;
				vertices[v + 1] = -j + offset_y;
				vertices[v + 2] = 0.5f;
				vertices[v + 3] = i + 1.0f + offset_x;
				vertices[v + 4] = -j + offset_y;
				vertices[v + 5] = 0.5f;
				vertices[v + 6] = i + offset_x;
				vertices[v + 7] = -(j + 1) + offset_y;
				vertices[v + 8] = 0.5f;
				vertices[v + 9] = i + 1.0f + offset_x;
				vertices[v + 10] = -(j + 1) + offset_y;
				vertices[v + 11] = 0.5f;
			}
		}
	}

	glGenBuffers(3, vbos);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE(vertices) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	// texcoord
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE(texcoord) * sizeof(GLfloat), texcoord, GL_STATIC_DRAW);

	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ARRAY_SIZE(indices) * sizeof(GLushort), indices, GL_STATIC_DRAW);

	return 0;
}

typedef struct {
	char string[256];
	GLuint texture;
	GLuint vbo[3];
} text_piece;

// graphics
struct graphics {
	int width;
	int height;
	map* map;
	GLuint terrain_texture;
	GLuint player_texture;
	GLuint terrain_program;
	GLuint texture_uniform;
	GLuint terrain_camera_uniform;
	GLuint right_uniform;
	GLuint top_uniform;
	GLuint zoom_uniform;
	tile_sector tile_sectors[9];
	GLuint player_vbo[3];
	float cam_pos_x;
	float cam_pos_y;
	int cam_offset_x;
	int cam_offset_y;
	TTF_Font* font;
	text_piece time_text;
	text_piece status_text;
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

	surface = SDL_SetVideoMode(width, height, info->vfmt->BitsPerPixel, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL | SDL_RESIZABLE);
	if(!surface) {
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_WM_SetCaption("Medium Aevum", NULL);

	if(TTF_Init() != 0) {
		fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
		return 1;
	}

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

static int load_texture_from_sdl_surface(SDL_Surface* surf)
{
	int hasAlpha = surf->format->BytesPerPixel == 4;
	GLuint texture;
	GLenum format;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

	glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha ? 4 : 3,
			surf->w, surf->h, 0, format,
			GL_UNSIGNED_BYTE,
			(char*)surf->pixels);
	return texture;
}

static int load_texture(const char* filename)
{
	SDL_Surface* surf = IMG_Load(filename);
	if(!surf) {
		fprintf(stderr, "Couldn't load image %s\n", filename);
		return 0;
	}
	int tex = load_texture_from_sdl_surface(surf);
	SDL_FreeSurface(surf);
	return tex;
}

static int load_textures(graphics* g)
{
	g->terrain_texture = load_texture("share/terrain.png");
	g->player_texture = load_texture("share/monk.png");
	g->texture_uniform = glGetUniformLocation(g->terrain_program, "sTexture");
	g->terrain_camera_uniform = glGetUniformLocation(g->terrain_program, "uCamera");
	g->right_uniform = glGetUniformLocation(g->terrain_program, "uRight");
	g->top_uniform = glGetUniformLocation(g->terrain_program, "uTop");
	g->zoom_uniform = glGetUniformLocation(g->terrain_program, "uZoom");
	return 0;
}

static int load_player_vertices(graphics* g)
{
	GLfloat vertices[] = {1.0f, 0.0f, 0.5f,
		1.0f, -1.0f, 0.5f,
		0.0f, 0.0f, 0.5f,
		0.0f, -1.0f, 0.5f};
	GLfloat texcoord[] = {1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		0.0f, 1.0f};
	GLushort indices[] = {0, 2, 1,
		1, 2, 3};

	glGenBuffers(3, g->player_vbo);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, g->player_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// texcoord
	glBindBuffer(GL_ARRAY_BUFFER, g->player_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord), texcoord, GL_STATIC_DRAW);

	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g->player_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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

static void draw_on_map(graphics* g)
{
	glUniform1f(g->zoom_uniform, 1.0f / ZOOM_FACTOR);
}

static void draw_gui(graphics* g)
{
	glUniform1f(g->zoom_uniform, 0.5f);
}

static int start_frame(graphics* g)
{
	glViewport(0, 0, g->width, g->height);
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform1f(g->right_uniform, g->width);
	glUniform1f(g->top_uniform, g->height);
	draw_on_map(g);
	return 0;
}

static int finish_frame(graphics* g)
{
	SDL_GL_SwapBuffers();
	return 0;
}

static int draw_player(graphics* g)
{
	int player_pos_x, player_pos_y;
	map_get_player_position(g->map, &player_pos_x, &player_pos_y);

	float cpx = -g->cam_pos_x + player_pos_x;
	float cpy = g->cam_pos_y - player_pos_y;

	glUniform1i(g->texture_uniform, 1);
	glUniform2f(g->terrain_camera_uniform, cpx, cpy);

	glBindBuffer(GL_ARRAY_BUFFER, g->player_vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, g->player_vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g->player_vbo[2]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	return 0;
}

static void draw_tile_sector(const tile_sector* s)
{
	glBindBuffer(GL_ARRAY_BUFFER, s->vbos[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, s->vbos[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->vbos[2]);
	glDrawElements(GL_TRIANGLES, TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 6, GL_UNSIGNED_SHORT, NULL);
}

static int draw_map(graphics* g)
{
	float cpx = g->cam_pos_x - g->cam_offset_x;
	float cpy = g->cam_pos_y - g->cam_offset_y;

	glUniform1i(g->texture_uniform, 0);
	glUniform2f(g->terrain_camera_uniform, -cpx, cpy);

	for(int i = 0; i < 9; i++) {
		draw_tile_sector(&g->tile_sectors[i]);
	}

	return 0;
}

static int draw_text(TTF_Font* font, const char text[static 256], text_piece* piece)
{
	if(strcmp(text, piece->string)) {
		static SDL_Color color = { 255, 255, 255 };
		SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, color);
		assert(surf);
		if(piece->texture) {
			glDeleteTextures(1, &piece->texture);
		} else {
			GLfloat texcoord[] = {1.0f, 0.0f,
				1.0f, 1.0f,
				0.0f, 0.0f,
				0.0f, 1.0f};
			GLushort indices[] = {0, 2, 1,
				1, 2, 3};

			glGenBuffers(3, piece->vbo);

			// texcoord
			glBindBuffer(GL_ARRAY_BUFFER, piece->vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord), texcoord, GL_STATIC_DRAW);

			// indices
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, piece->vbo[2]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		}
		glActiveTexture(GL_TEXTURE2);
		piece->texture = load_texture_from_sdl_surface(surf);

		strncpy(piece->string, text, 256);

		GLfloat vertices[] = {1.0f * surf->w, 0.0f, 0.5f,
			1.0f * surf->w, -1.0f * surf->h, 0.5f,
			0.0f, 0.0f, 0.5f,
			0.0f, -1.0f * surf->h, 0.5f};

		SDL_FreeSurface(surf);

		// vertices
		glBindBuffer(GL_ARRAY_BUFFER, piece->vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	}

	{
		glBindTexture(GL_TEXTURE_2D, piece->texture);
		glBindBuffer(GL_ARRAY_BUFFER, piece->vbo[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, piece->vbo[1]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, piece->vbo[2]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
	}

	return 0;
}

static int draw_time(graphics* g)
{
	char new_time_string[256];
	int hours, minutes;

	map_get_timeofday(g->map, &hours, &minutes);

	snprintf(new_time_string, 255, "%02d:%02d", hours, minutes);
	new_time_string[255] = 0;

	glUniform2f(g->terrain_camera_uniform, -g->width / 2 + 20.0f, g->height / 2 - 20.0f);
	return draw_text(g->font, new_time_string, &g->time_text);
}

static int draw_status(graphics* g)
{
	char new_string[256];

	snprintf(new_string, 255, "Hunger: %hhu", map_get_player_hunger(g->map));
	new_string[255] = 0;

	glUniform2f(g->terrain_camera_uniform, -g->width / 2 + 20.0f, g->height / 2 - 40.0f);
	return draw_text(g->font, new_string, &g->status_text);
}

static int draw_texts(graphics* g)
{
	glUniform1i(g->texture_uniform, 2);
	draw_time(g);
	draw_status(g);
	return 0;
}

static void cleanup_sdl(void)
{
	TTF_Quit();
	SDL_Quit();
}

static void reload_tile_sectors(graphics* g)
{
	g->cam_offset_x = (int)g->cam_pos_x - TILE_SECTOR_SIZE;
	g->cam_offset_y = (int)g->cam_pos_y - TILE_SECTOR_SIZE;

	for(int j = -1; j <= 1; j++) {
		for(int i = -1; i <= 1; i++) {
			tile_sector* s = &g->tile_sectors[(j + 1) * 3 + i + 1];
			cleanup_tile_sector(s);
			int off_x = TILE_SECTOR_SIZE * i + TILE_SECTOR_SIZE / 2;
			int off_y = TILE_SECTOR_SIZE * j - TILE_SECTOR_SIZE / 2;
			load_map_vertices(g->map, s->vbos,
					g->cam_offset_x + off_x,
					g->cam_offset_y - off_y,
					off_x,
					off_y);
		}
	}
}

static int init_gl(graphics* g)
{
	g->terrain_program = init_program();
	if(!g->terrain_program)
		return 1;

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	reload_tile_sectors(g);

	if(load_player_vertices(g))
		return 1;

	if(load_textures(g))
		return 1;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g->terrain_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g->player_texture);
	glUseProgram(g->terrain_program);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	return 0;
}

int init_font(graphics* g)
{
	g->font = TTF_OpenFont("share/DejaVuSans.ttf", 18);
	if(!g->font) {
		fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
		return 1;
	}

	return 0;
}

graphics* graphics_init(int width, int height, map* m)
{
	assert(width);
	assert(height);
	if(init_sdl(width, height)) {
		return NULL;
	}
	graphics* g = malloc(sizeof(graphics));
	assert(g);
	memset(g, 0x00, sizeof(*g));
	g->width = width;
	g->height = height;
	g->map = m;

	{
		int player_pos_x, player_pos_y;
		map_get_player_position(g->map, &player_pos_x, &player_pos_y);

		g->cam_pos_x = player_pos_x + 0.5f;
		g->cam_pos_y = player_pos_y + 0.5f;
	}

	if(init_gl(g)) {
		cleanup_sdl();
		free(g);
		return NULL;
	}

	if(init_font(g)) {
		cleanup_sdl();
		free(g);
		return NULL;
	}
	return g;
}

int graphics_draw(graphics* g)
{
	assert(g);
	if(start_frame(g))
		return 1;

	draw_on_map(g);
	if(draw_map(g))
		return 1;
	if(draw_player(g))
		return 1;

	draw_gui(g);
	if(draw_texts(g))
		return 1;

	if(finish_frame(g))
		return 1;

	{
		GLenum err;
		while((err = glGetError()) != GL_NO_ERROR) {
			fprintf(stderr, "GL error 0x%04x\n", err);
		}
	}

	return 0;
}

void graphics_move_camera(graphics* g, float x, float y)
{
	assert(g);
	graphics_set_camera_position(g, g->cam_pos_x + x, g->cam_pos_y + y);
}

int graphics_resized(graphics* g, int w, int h)
{
	assert(g);
	if(w && h) {
		const SDL_VideoInfo* info;
		SDL_Surface* surface;
		g->width = w;
		g->height = h;
		info = SDL_GetVideoInfo();
		if(!info) {
			fprintf(stderr, "SDL_GetVideoInfo: %s\n", SDL_GetError());
			return 1;
		}

		surface = SDL_SetVideoMode(w, h,
				info->vfmt->BitsPerPixel, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL | SDL_RESIZABLE);
		if(!surface) {
			fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
			return 1;
		}

	}
	return 0;
}

void graphics_cleanup(graphics* g)
{
	if(g->font) {
		TTF_CloseFont(g->font);
	}
	glDeleteTextures(1, &g->terrain_texture);
	glDeleteTextures(1, &g->player_texture);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(3, g->player_vbo);
	glDeleteBuffers(3, g->time_text.vbo);
	for(int i = 0; i < 4; i++) {
		cleanup_tile_sector(&g->tile_sectors[i]);
	}
	cleanup_sdl();
	free(g);
}

void graphics_get_camera_position(const graphics* g, int* x, int* y)
{
	*x = floor(g->cam_pos_x);
	*y = floor(g->cam_pos_y);
}

void graphics_set_camera_position(graphics* g, int x, int y)
{
	g->cam_pos_x = x + 0.5f;
	g->cam_pos_y = y + 0.5f;
	if(g->cam_pos_x - g->cam_offset_x < 0 ||
			g->cam_pos_x - g->cam_offset_x > 2 * TILE_SECTOR_SIZE ||
			g->cam_pos_y - g->cam_offset_y < 0 ||
			g->cam_pos_y - g->cam_offset_y > 2 * TILE_SECTOR_SIZE) {
		reload_tile_sectors(g);
	}
}


