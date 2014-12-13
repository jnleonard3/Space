#ifndef SPACE_H
#define SPACE_H

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

#endif