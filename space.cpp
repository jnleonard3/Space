#include <windows.h>
#include <GL/glut.h>
#include <iostream>

namespace space {

	class Space {
	public:
		Space();
		int value;
	};

	Space::Space() {
	}

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
			//std::cout << "Quad: " << id << x << ", " << y << std::endl;
			//std::cout << "Quad: " << id << " Rel: " << relX << ", " << relY << std::endl;
			//std::cout << "Quad: " << id << " Rel: 2 " << rel2X << ", " << rel2Y << std::endl;

			return child->GetSpace(relX, relY);
		} else {
			//std::cout << "Quad: " << id << " Field: " << x << ", " << y << std::endl;
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
}

void display() {
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
   glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer
 
   // Draw a Red 1x1 Square centered at origin
   glBegin(GL_QUADS);              // Each set of 4 vertices form a quad
      glColor3f(1.0f, 0.0f, 0.0f); // Red
      glVertex2f(-0.5f, -0.5f);    // x, y
      glVertex2f( 0.5f, -0.5f);
      glVertex2f( 0.5f,  0.5f);
      glVertex2f(-0.5f,  0.5f);
   glEnd();
 
   glFlush();  // Render now
}

int main(int argc, char** argv) {
   glutInit(&argc, argv);                 // Initialize GLUT
   glutCreateWindow("OpenGL Setup Test"); // Create a window with the given title
   glutInitWindowSize(320, 320);   // Set the window's initial width & height
   glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
   glutDisplayFunc(display); // Register display callback handler for window re-paint
   glutMainLoop();           // Enter the infinitely event-processing loop
   return 0;
}

void printTheWorld(space::World *world) {
	space::Quad* rootQuad = world->rootQuad;
	for(int i = -(rootQuad->length / 2); i < (rootQuad->length / 2); i += 1) {
		for(int j = -(rootQuad->length / 2); j < (rootQuad->length / 2); j += 1) {
			std::cout << rootQuad->GetSpace(i, j)->value << " ";
		}
		std::cout << std::endl;
	}
}

int main2(int argc, char** argv) {
	space::World* world = new space::World();
	space::Quad* rootQuad = world->rootQuad;
	int val = 0;
	for(int i = -(rootQuad->length / 2); i < (rootQuad->length / 2); i += 1) {
		for(int j = -(rootQuad->length / 2); j < (rootQuad->length / 2); j += 1) {
			rootQuad->GetSpace(i, j)->value = 0;
			val += 1;
			if(i == j) {
				rootQuad->GetSpace(i, j)->value = 1;
			}
		}
	}

	printTheWorld(world);
}
