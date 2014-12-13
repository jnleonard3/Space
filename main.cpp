#include "SDL2\SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstdio>

#include "space.h"
#include "log.h"
#include "game.h"

static int gW = 8, gH = 15;

struct GlyphCoord {
	int x, y;
	SDL_Color fg, bg;
};

GlyphCoord MapGlyph(space::SpaceType type, SDL_Color fg, SDL_Color bg) {
	GlyphCoord coord = { 10, 2, fg, bg };
	switch(type) {
		case space::NONE:
			coord.x = 10;
			coord.y = 2;
			break;
		case space::WALL:
			coord.x = 23;
			coord.y = 1;
			break;
		case space::FLOOR:
			coord.x = 11;
			coord.y = 3;
			break;
		case space::ENGINE:
			coord.x = 4;
			coord.y = 3;
			break;
		case space::FIRE:
			coord.x = 24;
			coord.y = 1;
			break;
		case space::METEOR:
			coord.x = 13;
			coord.y = 3;
			break;
		case space::METEOR2:
			coord.x = 14;
			coord.y = 0;
			break;
	}
	return coord;
}

struct TextureCache {
	int hash;
	SDL_Texture* texture;
};

SDL_Texture* CreateColoredGlyph(SDL_Renderer* renderer, SDL_Surface* bwGlyph, SDL_Color fgC, SDL_Color bgC) {
	SDL_Surface* colorSurface = SDL_ConvertSurface(bwGlyph, bwGlyph->format, 0);

	colorSurface->format->palette->colors[0] = bgC;
	colorSurface->format->palette->colors[1] = fgC;

	SDL_Texture* coloredGlyph = SDL_CreateTextureFromSurface(renderer, colorSurface);
	SDL_FreeSurface(colorSurface);
	return coloredGlyph;
}

SDL_Texture* GetColoredGlyph(SDL_Renderer* renderer, SDL_Surface* bwGlyph, SDL_Color fgC, SDL_Color bgC, TextureCache* cache, int cacheWidth) {
	int hash = fgC.r * 3 + fgC.g * 5 + fgC.b * 7 + bgC.r * 11 + bgC.g * 13 + bgC.b * 17;
	TextureCache entry = cache[hash % cacheWidth];
	if(entry.hash == hash && entry.texture != 0) {
		return entry.texture;
	}
	SDL_DestroyTexture(entry.texture);
	SDL_Texture* texture = CreateColoredGlyph(renderer, bwGlyph, fgC, bgC);
	entry.hash = hash;
	entry.texture = texture;
	cache[hash % cacheWidth] = entry;
	return texture;
}

void renderGlyph(SDL_Renderer* renderer, SDL_Texture* glyphs, int x, int y, int gX, int gY) {
	SDL_Rect quad = { x, y, gW, gH };
	SDL_Rect clip = { gX*gW, gY*gH, gW, gH };
	SDL_RenderCopy(renderer, glyphs, &clip, &quad);
}

GlyphCoord MapChar(char ch, SDL_Color fg, SDL_Color bg) {
	GlyphCoord coord = { 10, 2, fg, bg };
	if(ch >= 'a' && ch <= 'z') {
		coord.x = ch - 'a';
		coord.y = 0;
	} else if (ch >= 'A' && ch <= 'Z') {
		coord.x = ch - 'A';
		coord.y = 1;
	} else if (ch >= '1' && ch <= '9') {
		coord.x = ch - '1';
		coord.y = 2;
	} else if (ch == '0') {
		coord.x = 9;
		coord.y = 2;
	}
	return coord;
}

int main(int argc, char* argv[]) {

	space::World* world = new space::World();
	space::Quad* rootQuad = world->rootQuad;

	CreateRoom(world, -5, -5, 5, 5);
	CreateRoom(world, -7, -2, 7, 2); 
	CreateRoom(world, -2, -4, 2, -9); 
	CreateBlock(world, space::ENGINE, -3, 6, 3, 6, true);
	CreateBlock(world, space::FIRE, -3, 7, 3, 7, true);

	srand (time(NULL));

    	SDL_Window *window;
    	SDL_Init(SDL_INIT_VIDEO);

	static int WIN_WIDTH = rootQuad->length;
	static int WIN_HEIGHT = rootQuad->length + 1;

    	window = SDL_CreateWindow(
		"Spaaaaaaace", 
		SDL_WINDOWPOS_UNDEFINED,
        	SDL_WINDOWPOS_UNDEFINED,
        	WIN_WIDTH * gW,
        	WIN_HEIGHT * gH,
        	SDL_WINDOW_OPENGL
    	);

    	if (window == NULL) {
        	printf("Could not create window: %s\n", SDL_GetError());
       		return 1;
    	}

	SDL_Renderer* Main_Renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Surface* Loading_Surf = SDL_LoadBMP("glyphs_black.bmp");

	static int TEXTURE_CACHE_SIZE = 128;

	TextureCache* textureCache = new TextureCache[TEXTURE_CACHE_SIZE];
	for(int i = 0; i < TEXTURE_CACHE_SIZE; i += 1) {
		textureCache[i].hash;
		textureCache[i].texture = 0;
	}

	SDL_Color black = { 0, 0, 0 }, white = { 255, 255, 255 };
	
	int** hashes = new int*[WIN_HEIGHT];
	for(int i = 0; i < WIN_HEIGHT; i += 1) {
		hashes[i] = new int[WIN_WIDTH];
		for(int j = 0; j < WIN_WIDTH; j += 1) {
			hashes[i][j] = -1;
		}
	}

	static int NUM_METEORS = 20;
	Meteor* meteors = new Meteor[NUM_METEORS];
	for(int i = 0; i < NUM_METEORS; i += 1) {
		InitalizeMeteor(world, &meteors[i], true);
	}

	Log::Get()->Write("Herp Derp 1230", 14);

 	for(SDL_Event e; e.type != SDL_QUIT; SDL_PollEvent(&e)) {

		int redraws = 0;
		
		ManageScrollingMeteors(world, NUM_METEORS, meteors);

		char* logMsg = Log::Get()->buffer;

		for(int i = 0; i < WIN_WIDTH; i += 1) {
			for(int j = 0; j < WIN_HEIGHT; j += 1) {

				GlyphCoord coord = { 10, 2, black, white };
				if(i < rootQuad->length && j < rootQuad->length) {
					space::Space* space = rootQuad->GetSpace(i - (rootQuad->length/2), j - (rootQuad->length/2));
					space::SpaceType type = space->overlay;
					if(space->type != space::NONE) {
						type = space->type;
					}
					coord = MapGlyph(type, white, black);
				} else {
					char ch = logMsg[i];
					coord = MapChar(ch, black, white);
				}

				int hash = coord.x * 3 + coord.y * 7 + coord.fg.r * 11 + coord.bg.r * 13;
				if(hashes[i][j] == hash) {
					continue;
				} else {
					hashes[i][j] = hash;
				}
				SDL_Texture* glyphs = GetColoredGlyph(Main_Renderer, Loading_Surf, coord.fg, coord.bg, textureCache, TEXTURE_CACHE_SIZE);
				renderGlyph(Main_Renderer, glyphs, i*gW, j*gH, coord.x, coord.y);
				redraws += 1;
			}
		}
	
		char* redrawMsg = new char[Log::BUFFER_SIZE];	
		snprintf(redrawMsg, Log::BUFFER_SIZE, "Redraws per frame: %d", redraws);
		Log::Get()->Write(redrawMsg, Log::BUFFER_SIZE);


		SDL_RenderPresent(Main_Renderer);
   		SDL_Delay(25); 
  	}

	SDL_FreeSurface(Loading_Surf);
	for(int i = 0; i < TEXTURE_CACHE_SIZE; i += 1) {
		TextureCache cache = textureCache[i];
		if(cache.texture != 0) {
			SDL_DestroyTexture(cache.texture);
		}
	}
	SDL_DestroyRenderer(Main_Renderer);
    	SDL_DestroyWindow(window);

    	SDL_Quit();
    	return 0;
}