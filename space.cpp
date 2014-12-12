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
		ENGINE,
		METEOR,
		METEOR2,
		FIRE
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
		rootQuad->length = 60;
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
	int xMin = std::min(xF, xT), xMax = std::max(xF, xT);
	int yMin = std::min(yF, yT), yMax = std::max(yF, yT);
	CreateBlock(world, space::WALL, xMin, yMin, xMin, yMax, false);
	CreateBlock(world, space::WALL, xMin, yMin, xMax, yMin, false);
	CreateBlock(world, space::WALL, xMax, yMin, xMax, yMax, false);
	CreateBlock(world, space::WALL, xMin, yMax, xMax, yMax, false);
	CreateBlock(world, space::FLOOR, xMin + 1, yMin + 1, xMax - 1, yMax - 1, true);
}

class Log {
	public:
		static int BUFFER_SIZE;
		static Log* Get();

		void Write(const char* msg, int length);
		char* buffer;
	private:
		Log();
		static Log* Instance;
};

int Log::BUFFER_SIZE = 256;

Log* Log::Instance = 0;

Log::Log() {
	buffer = new char[Log::BUFFER_SIZE];
	for(int i = 0; i < BUFFER_SIZE; i += 1) {
		buffer[i] = 0;
	}
}

void Log::Write(const char* msg, int length) {
	length = std::min(length, BUFFER_SIZE);
	for(int i = 0; i < length; i += 1) {
		buffer[i] = msg[i];
	}
}

Log* Log::Get() {
	if(Instance == 0) {
		Instance = new Log();
	}
	return Instance;
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

void renderGlyph(SDL_Renderer* renderer, SDL_Texture* glyphs, int x, int y, int gX, int gY) {
	SDL_Rect quad = { x, y, gW, gH };
	SDL_Rect clip = { gX*gW, gY*gH, gW, gH };
	SDL_RenderCopy(renderer, glyphs, &clip, &quad);
}

GlyphCoord MapChar(char ch) {
	GlyphCoord coord = { 2, 10 };
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
	SDL_Texture* glyphs = SDL_CreateTextureFromSurface(Main_Renderer, Loading_Surf);
  	SDL_FreeSurface(Loading_Surf);

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
		
		ManageScrollingMeteors(world, NUM_METEORS, meteors);

		char* logMsg = Log::Get()->buffer;

		for(int i = 0; i < WIN_WIDTH; i += 1) {
			for(int j = 0; j < WIN_HEIGHT; j += 1) {

				GlyphCoord coord = { 0, 3 };
				if(i < rootQuad->length && j < rootQuad->length) {
					space::Space* space = rootQuad->GetSpace(i - (rootQuad->length/2), j - (rootQuad->length/2));
					space::SpaceType type = space->overlay;
					if(space->type != space::NONE) {
						type = space->type;
					}
					coord = MapGlyph(type);
				} else {
					char ch = logMsg[i];
					coord = MapChar(ch);
				}

				int hash = coord.x * 3 + coord.y * 7;
				if(hashes[i][j] == hash) {
					continue;
				} else {
					hashes[i][j] = hash;
				}
				renderGlyph(Main_Renderer, glyphs, i*gW, j*gH, coord.x, coord.y);
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
