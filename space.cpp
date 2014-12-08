#include "SDL2\SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

namespace space {

	enum SpaceType {
		NONE = 0,
		WALL,
		FLOOR,
		METEOR,
		METEOR2
	};

	class Space {
	public:
		Space();
		SpaceType type;
		SpaceType overlay;
		int hash();
	};

	Space::Space() {
	}

	int Space::hash() {
		return (3 * type) + (7 * overlay);
	};

	class Quad {
	public:
		Quad();
		Quad(int length);
		int id;
		int length;
		Quad** children;
		Space** field;
		Space* GetSpace(int x, int y);
	};

	Quad::Quad() {
	};

	Quad::Quad(int length) {
		this->length = length;
		this->field = new Space*[length];
		for(int i = 0; i < length; i += 1) {
			this->field[i] = new Space[length];
		}
	};

	Space* Quad::GetSpace(int x, int y) {
		if (children != 0) {
			Quad* child = 0;
			int relX = x, relY = y;
			if(x < 0) {
				if(y < 0) {
					child = children[0];
					relX = child->length + x;
					relY = child->length + y;
				} else {
					child = children[1];
					relX = child->length + x;
				}
			} else {
				if(y < 0) {
					child = children[2];
					relY = child->length + y;
				} else {
					child = children[3];
				}
			}
			int rel2X = relX - child->length;
			int rel2Y = relY + (child->length / 2);

			return child->GetSpace(relX, relY);
		} else {
			return &field[x][y];
		}
	}

	class World {
	public:
		World();
		Quad* rootQuad;
	};

	World::World() {
		rootQuad = new Quad();
		rootQuad->children = new Quad*[4];
		rootQuad->length = 50;
		for(int i = 0; i < 4; i += 1) {
			rootQuad->children[i] = new Quad(rootQuad->length/2);
			rootQuad->children[i]->id = i + 1;
		}
	};
};


// Space Nonsense

struct Meteor {
	space::SpaceType type;
	float xD, yD;
	float xA, yA;
};

void MoveMeteor(space::World* world, Meteor* meteor) {
	space::Quad* rootQuad = world->rootQuad;
	int bound = rootQuad->length / 2;
	int negBound = -bound;
	if(meteor->xA >= negBound && meteor->xA < bound) {
		if(meteor->yA >= negBound && meteor->yA < bound) {
			space::Space* space = rootQuad->GetSpace(meteor->xA, meteor->yA);
			space->overlay = space::NONE;
		}
	}
	meteor->xA += meteor->xD;
	meteor->yA += meteor->yD;

	if(meteor->xA >= negBound && meteor->xA < bound) {
		if(meteor->yA >= negBound && meteor->yA < bound) {
			int x = meteor->xA > 0 ? ceil(meteor->xA) : floor(meteor->xA); 
			int y = meteor->yA > 0 ? ceil(meteor->yA) : floor(meteor->yA);
			space::Space* space = rootQuad->GetSpace(meteor->xA, meteor->yA);
			space->overlay = meteor->type;
		}
	}
}

void InitalizeMeteor(space::World* world, Meteor* meteor, bool randomY) {
	space::Quad* rootQuad = world->rootQuad;
	int bound = rootQuad->length / 2;
	int negBound = -bound;
	int xF = (rand() % rootQuad->length) - bound;
	int yF = randomY ? (rand() % rootQuad->length) - bound : negBound - (rand() % 5);
	int xT = ((rand() % 10) - 5) + xF;
	int yT = (rand() % 5) + bound;
	int binary = (rand() % 2);
	meteor->xA = xF;
	meteor->yA = yF;
	meteor->xD = xT - xF;
	meteor->yD = yT - yF;
	float length = sqrt(pow(meteor->xD, 2) + pow(meteor->yD, 2));
	meteor->xD /= length;
	meteor->yD /= length;
	if(binary == 0) {
		meteor->type = space::METEOR;
	} else {
		meteor->type = space::METEOR2;
	}

}

void ManageScrollingMeteors(space::World* world, int length, Meteor* meteors) {
	space::Quad* rootQuad = world->rootQuad;
	int bound = rootQuad->length / 2;
	int negBound = -bound;
	for(int i = 0; i < length; i += 1) {
		Meteor* meteor = &meteors[i];
		if(meteor->xD == 0 || meteor-> yA > bound) {
			InitalizeMeteor(world, meteor, false);
		} else {
			MoveMeteor(world, meteor);
		}
	}
}

void CreateBlock(space::World* world, space::SpaceType type, int xF, int yF, int xT, int yT, bool overwrite) {
	space::Quad* rootQuad = world->rootQuad;
	for(int i = std::min(xF, xT); i <= std::max(xF, xT); i += 1) {
		for(int j = std::min(yF, yT); j <= std::max(yF, yT); j += 1) {
			space::Space* space = rootQuad->GetSpace(i, j);
			if(space-> type == space::NONE || overwrite) {
				space->type = type;
			}
		}
	}
}

void CreateRoom(space::World* world, int xF, int yF, int xT, int yT) {
	space::Quad* rootQuad = world->rootQuad;
	CreateBlock(world, space::WALL, xF, yF, xF, yT, false);
	CreateBlock(world, space::WALL, xF, yF, xT, yF, false);
	CreateBlock(world, space::WALL, xT, yF, xT, yT, false);
	CreateBlock(world, space::WALL, xF, yT, xT, yT, false);
	CreateBlock(world, space::FLOOR, xF + 1, yF + 1, xT - 1, yT - 1, true);
}


// Graphics Nonsense

static int gW = 8, gH = 15;

struct GlyphCoord {
	int x, y;
};

GlyphCoord MapGlyph(space::SpaceType type) {
	GlyphCoord coord = { 10, 2 };
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

void renderGlyph(SDL_Renderer* renderer, SDL_Texture* glyphs, int x, int y, int gX, int gY) {
	SDL_Rect quad = { x, y, gW, gH };
	SDL_Rect clip = { gX*gW, gY*gH, gW, gH };
	SDL_RenderCopy(renderer, glyphs, &clip, &quad);
}

int main(int argc, char* argv[]) {

	space::World* world = new space::World();
	space::Quad* rootQuad = world->rootQuad;

	CreateRoom(world, -5, -5, 5, 5);
	CreateRoom(world, -7, -2, 7, 2); 
	CreateRoom(world, -2, -5, 2, -10); 

	srand (time(NULL));

    	SDL_Window *window;
    	SDL_Init(SDL_INIT_VIDEO);

    	window = SDL_CreateWindow(
			"Spaaaaaaace", 
			SDL_WINDOWPOS_UNDEFINED,
        	SDL_WINDOWPOS_UNDEFINED,
        	rootQuad->length*gW,
        	rootQuad->length*gH,
        	SDL_WINDOW_OPENGL
    	);

    	if (window == NULL) {
        	printf("Could not create window: %s\n", SDL_GetError());
       		return 1;
    	}

	SDL_Renderer* Main_Renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Surface* Loading_Surf = SDL_LoadBMP("glyphs_black.bmp");
	SDL_Texture* glyphs = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
  	SDL_FreeSurface(Loading_Surf);

	int** hashes = new int*[rootQuad->length];
	for(int i = 0; i < rootQuad->length; i += 1) {
		hashes[i] = new int[rootQuad->length];
	}

	static int NUM_METEORS = 30;
	Meteor* meteors = new Meteor[NUM_METEORS];
	for(int i = 0; i < NUM_METEORS; i += 1) {
		InitalizeMeteor(world, &meteors[i], true);
	}

 	for(SDL_Event e; e.type != SDL_QUIT; SDL_PollEvent(&e)) {
		
		ManageScrollingMeteors(world, NUM_METEORS, meteors);

		int k = 0;
		for(int i = 0; i < rootQuad->length; i += 1) {
			for(int j = 0; j < rootQuad->length; j += 1) {
				space::Space* space = rootQuad->GetSpace(i - (rootQuad->length/2), j - (rootQuad->length/2));
				int hash = space->hash();
				if(hashes[i][j] == hash) {
					continue;
				} else {
					hashes[i][j] = hash;
				}
				
				space::SpaceType type = space->overlay;
				if(space->type != space::NONE) {
					type = space->type;
				}
				GlyphCoord coord = MapGlyph(type);
				renderGlyph(Main_Renderer, glyphs, i*gW, j*gH, coord.x, coord.y);
				k += 1;	
			}
		}
		SDL_RenderPresent(Main_Renderer);
   		SDL_Delay(25); 
  	}

	SDL_DestroyTexture(glyphs);
	SDL_DestroyRenderer(Main_Renderer);
    	SDL_DestroyWindow(window);

    	SDL_Quit();
    	return 0;
}
