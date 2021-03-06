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
	GLuint overlay_vbos[3];
	int overlay_indices;
} tile_sector;

void cleanup_tile_sector(tile_sector* s)
{
	if(s->vbos[0])
		glDeleteBuffers(3, s->vbos);
	if(s->overlay_vbos[0])
		glDeleteBuffers(3, s->overlay_vbos);
	s->overlay_indices = 0;
}

static void get_detailed_tile_texcoords(const detmap* m, int i, int j, GLfloat tile_texcoords[static 4])
{
	detterrain_type tt = detmap_get_terrain_at(m, i, j);
	switch(tt) {
		case dett_tree:
			tile_texcoords[0] = 0.0f; tile_texcoords[1] = 0.0f;
			break;

		case dett_grass:
			tile_texcoords[0] = 0.5f; tile_texcoords[1] = 0.0f;
			break;

		case dett_rock:
		case dett_wall:
			tile_texcoords[0] = 0.0f; tile_texcoords[1] = 0.5f;
			break;

		case dett_water:
			tile_texcoords[0] = 0.5f; tile_texcoords[1] = 0.5f;
			break;
	}

	tile_texcoords[2] = tile_texcoords[0] + 0.5f;
	tile_texcoords[3] = tile_texcoords[1] + 0.5f;
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

static void fill_index_buffer(int ind, int i, int j, GLushort* indices)
{
	int fi = ind * 6;
	int ii0 = ind * 4;
	int ii1 = ii0 + 1;
	int ii2 = ii0 + 2;
	int ii3 = ii0 + 3;
	indices[fi + 0] = ii0; indices[fi + 1] = ii2; indices[fi + 2] = ii1;
	indices[fi + 3] = ii1; indices[fi + 4] = ii2; indices[fi + 5] = ii3;
}

static void fill_vertex_buffer(int ind, int i, int j, float offset_x, float offset_y, GLfloat* vertices)
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

static void fill_vbos(GLuint vbos[static 3], int ind, GLfloat* vertices, GLfloat* texcoord, GLushort* indices)
{
	glGenBuffers(3, vbos);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, ind * 12 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	// texcoord
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, ind * 8 * sizeof(GLfloat), texcoord, GL_STATIC_DRAW);

	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind * 6 * sizeof(GLushort), indices, GL_STATIC_DRAW);
}

static int load_overlay_vertices(const map* m, const detmap* detm, tile_sector* sector,
		int map_x, int map_y, float offset_x, float offset_y)
{
	GLfloat vertices[TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 12];
	GLfloat texcoord[TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 8];
	GLushort indices[TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 6];

	int i, j;

	int num_overlays = 0;

	// for each tile
	for(j = 0; j < TILE_SECTOR_SIZE; j++) {
		for(i = 0; i < TILE_SECTOR_SIZE; i++) {
			if(!detm) {
				const town* t = map_get_town_at(m, map_x + i, map_y + j);
				if(!t)
					continue;
			} else {
				detmap_overlay overlay = detmap_get_overlay_at(detm, map_x + i, map_y + j);
				if(overlay == detmap_overlay_none)
					continue;
			}

			fill_index_buffer(num_overlays, i, j, indices);

			{
				int t = num_overlays * 8;
				GLfloat tile_texcoords[4];
				tile_texcoords[0] = 0.0f; tile_texcoords[1] = 0.0f;
				tile_texcoords[2] = 1.0f; tile_texcoords[3] = 1.0f;
				texcoord[t + 0] = tile_texcoords[0]; texcoord[t + 1] = tile_texcoords[3];
				texcoord[t + 2] = tile_texcoords[2]; texcoord[t + 3] = tile_texcoords[3];
				texcoord[t + 4] = tile_texcoords[0]; texcoord[t + 5] = tile_texcoords[1];
				texcoord[t + 6] = tile_texcoords[2]; texcoord[t + 7] = tile_texcoords[1];
			}

			fill_vertex_buffer(num_overlays, i, j, offset_x, offset_y, vertices);

			num_overlays++;
		}
	}

	sector->overlay_indices = num_overlays * 6;

	if(num_overlays)
		fill_vbos(sector->overlay_vbos, num_overlays, vertices, texcoord, indices);

	return 0;
}

static int load_map_vertices(const map* m, const detmap* detm, tile_sector* sector,
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

			fill_index_buffer(ind, i, j, indices);
			{
				int t = ind * 8;
				GLfloat tile_texcoords[4];
				memset(tile_texcoords, 0x00, sizeof(tile_texcoords));
				if(detm)
					get_detailed_tile_texcoords(detm, map_x + i, map_y + j, tile_texcoords);
				else
					get_tile_texcoords(m, map_x + i, map_y + j, tile_texcoords);
				texcoord[t + 0] = tile_texcoords[0]; texcoord[t + 1] = tile_texcoords[3];
				texcoord[t + 2] = tile_texcoords[2]; texcoord[t + 3] = tile_texcoords[3];
				texcoord[t + 4] = tile_texcoords[0]; texcoord[t + 5] = tile_texcoords[1];
				texcoord[t + 6] = tile_texcoords[2]; texcoord[t + 7] = tile_texcoords[1];
			}
			fill_vertex_buffer(ind, i, j, offset_x, offset_y, vertices);
		}
	}

	fill_vbos(sector->vbos, TILE_SECTOR_SIZE * TILE_SECTOR_SIZE, vertices, texcoord, indices);

	return load_overlay_vertices(m, detm, sector, map_x, map_y, offset_x, offset_y);
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
	player* player;
	person_directory* pd;
	worldtime* time;
	GLuint terrain_texture;
	GLuint overlay_texture;
	GLuint player_texture;
	GLuint person_texture;
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
	text_piece discussion_text;
	text_piece answer_text[8];
	text_piece message_text;

	int detailed;
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
	g->person_texture = load_texture("share/hero.png");
	g->overlay_texture = load_texture("share/town.png");
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
	glUniform1i(g->texture_uniform, 0);
	draw_on_map(g);
	return 0;
}

static int finish_frame(graphics* g)
{
	SDL_GL_SwapBuffers();
	return 0;
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
			load_map_vertices(g->map, player_get_detmap(g->player), s,
					g->cam_offset_x + off_x,
					g->cam_offset_y - off_y,
					off_x,
					off_y);
		}
	}
}

static int draw_person(graphics* g, int pos_x, int pos_y, GLuint texture)
{
	float cpx = -g->cam_pos_x + pos_x;
	float cpy = g->cam_pos_y - pos_y;

	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform2f(g->terrain_camera_uniform, cpx, cpy);

	glBindBuffer(GL_ARRAY_BUFFER, g->player_vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, g->player_vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g->player_vbo[2]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	return 0;
}

static int draw_people(graphics* g)
{
	const detmap* detm = player_get_detmap(g->player);
	if(detm) {
		const person* people[24];
		int mx, my;
		player_get_position(g->player, &mx, &my);
		int ret = person_directory_get_people(g->pd, mx, my, people, 24);
		if(ret == 24) {
			fprintf(stderr, "More people in the detmap than assumed!\n");
		}
		for(int i = 0; i < ret; i++) {
			int pos_x, pos_y;
			person_directory_get_person_position(g->pd, people[i], &pos_x, &pos_y);
			if(draw_person(g, pos_x, pos_y, g->person_texture))
				return 1;
		}
	}

	return 0;
}

static int draw_player(graphics* g)
{
	int player_pos_x, player_pos_y;
	player_get_current_position(g->player, &player_pos_x, &player_pos_y);

	return draw_person(g, player_pos_x, player_pos_y, g->player_texture);
}

static void draw_tile_sector(graphics* g, const tile_sector* s)
{
	// terrain
	glBindTexture(GL_TEXTURE_2D, g->terrain_texture);
	glBindBuffer(GL_ARRAY_BUFFER, s->vbos[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, s->vbos[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->vbos[2]);
	glDrawElements(GL_TRIANGLES, TILE_SECTOR_SIZE * TILE_SECTOR_SIZE * 6, GL_UNSIGNED_SHORT, NULL);

	// overlay
	if(s->overlay_indices) {
		glBindTexture(GL_TEXTURE_2D, g->overlay_texture);

		glBindBuffer(GL_ARRAY_BUFFER, s->overlay_vbos[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, s->overlay_vbos[1]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->overlay_vbos[2]);
		glDrawElements(GL_TRIANGLES, s->overlay_indices, GL_UNSIGNED_SHORT, NULL);
	}
}

static void center_camera_position(graphics* g)
{
	int player_pos_x, player_pos_y;
	player_get_current_position(g->player, &player_pos_x, &player_pos_y);

	g->cam_pos_x = player_pos_x + 0.5f;
	g->cam_pos_y = player_pos_y + 0.5f;
}

static int draw_map(graphics* g)
{
	if(player_get_detmap(g->player)) {
		if(!g->detailed) {
			g->detailed = 1;
			center_camera_position(g);
			reload_tile_sectors(g);
		}
	} else {
		if(g->detailed) {
			g->detailed = 0;
			center_camera_position(g);
			reload_tile_sectors(g);
		}
	}

	float cpx = g->cam_pos_x - g->cam_offset_x;
	float cpy = g->cam_pos_y - g->cam_offset_y;

	glUniform2f(g->terrain_camera_uniform, -cpx, cpy);

	for(int i = 0; i < 9; i++) {
		draw_tile_sector(g, &g->tile_sectors[i]);
	}

	return 0;
}

/* 0, 0 is top left */
static void set_text_pos_pixels(graphics* g, int x, int y)
{
	int dx, dy;
	if(x >= 0)
		dx = -g->width / 2 + x;
	else
		dx = g->width / 2 + x;
	if(y >= 0)
		dy = g->height / 2 - y;
	else
		dy = -g->height / 2 - y;
	glUniform2f(g->terrain_camera_uniform, dx, dy);
}

void free_text_piece(text_piece* piece)
{
	/* texture and vbo will be freed when reusing the text piece */
	piece->string[0] = 0;
}

static int draw_text(graphics* g, const char text[static 256], text_piece* piece, int x, int y)
{
	TTF_Font* font = g->font;
	set_text_pos_pixels(g, x, y);
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

	worldtime_get_timeofday(g->time, &hours, &minutes);

	snprintf(new_time_string, 255, "%02d:%02d", hours, minutes);
	new_time_string[255] = 0;

	return draw_text(g, new_time_string, &g->time_text, 20, 20);
}

static int draw_status(graphics* g)
{
	char new_string[256];

	snprintf(new_string, 255, "Hunger: %hhu%s", player_get_hunger(g->player),
			player_sleeping(g->player) ? " - sleeping" : "");
	new_string[255] = 0;

	if(draw_text(g, new_string, &g->status_text, 20, 40))
		return 1;

	if(g->message_text.string[0])
		return draw_text(g, g->message_text.string, &g->message_text, 20, 80);

	return 0;
}

static int draw_discussion(graphics* g)
{
	discussion* d = player_get_discussion(g->player);
	if(d) {
		char new_string[256];

		snprintf(new_string, 255, "%s", discussion_get_line(d));
		new_string[255] = 0;

		if(draw_text(g, new_string, &g->discussion_text, 20, 60))
			return 1;

		char** answers;
		int replies = discussion_get_answers(d, &answers);
		if(replies) {
			assert(replies - 1 < ARRAY_SIZE(g->answer_text));
			for(int i = replies - 1; i >= 0; i--) {
				char answer_string[256];
				snprintf(answer_string, 255, "%d: %s", i + 1, answers[i]);
				answer_string[255] = 0;
				if(draw_text(g, answer_string, &g->answer_text[i], 20, -20 * (replies - i)))
					return 1;
			}
		}
	}
	return 0;
}

static int draw_texts(graphics* g)
{
	draw_time(g);
	draw_status(g);
	draw_discussion(g);
	return 0;
}

static void cleanup_sdl(void)
{
	TTF_Quit();
	SDL_Quit();
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

graphics* graphics_create(int width, int height, player* p, worldtime* w, person_directory* pd)
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
	g->map = player_get_map(p);
	g->player = p;
	g->time = w;
	g->pd = pd;

	center_camera_position(g);

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

	if(draw_people(g))
		return 1;

	draw_gui(g);
	if(draw_texts(g))
		return 1;

	free_text_piece(&g->message_text);

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
	glDeleteTextures(1, &g->overlay_texture);
	glDeleteTextures(1, &g->terrain_texture);
	glDeleteTextures(1, &g->player_texture);
	glDeleteTextures(1, &g->person_texture);
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
	if(g->cam_pos_x - g->cam_offset_x - 1 < 0 ||
			g->cam_pos_x - g->cam_offset_x + 1 > 2 * TILE_SECTOR_SIZE ||
			g->cam_pos_y - g->cam_offset_y - 1 < 0 ||
			g->cam_pos_y - g->cam_offset_y + 1 > 2 * TILE_SECTOR_SIZE) {
		reload_tile_sectors(g);
	}
}

void graphics_add_message(graphics* g, const char* msg)
{
	assert(strlen(msg) < 256);
	draw_text(g, msg, &g->message_text, 20, 80);
}


