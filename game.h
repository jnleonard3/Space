#include <algorithm>

#include "space.h"

#ifndef GAME_H
#define GAME_H

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

#endif