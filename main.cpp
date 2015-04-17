#include <ctime>
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"
#include "GL/glu.h"

int main(int argc,char *argv[]){

        // ------------------------------------ window initialization //

	SDL_Window   *window;
	unsigned int  windowWidth  = 640;
	unsigned int  windowHeight = 480;

	srand(std::time(0));
	SDL_Init(SDL_INIT_EVERYTHING);
	atexit(SDL_Quit);
	SDL_GL_CreateContext(window = SDL_CreateWindow("Dodecasketch",
	                     SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
	                     windowWidth,windowHeight,
	                     SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE));

        // ------------------------------------ camera initialization //

	auto reshape = [&](unsigned int width,unsigned int height){
		SDL_SetWindowSize(window,windowWidth=width,windowHeight=height);
		glViewport(0,0,windowWidth,windowHeight);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45,(float)windowWidth/windowHeight,0.1,20);
		glMatrixMode(GL_MODELVIEW);
	};

	reshape(windowWidth,windowHeight);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glPointSize(2);

        // ------------------------------------------------ main loop //

	while(true){
		SDL_Event event;
		while(SDL_PollEvent(&event))switch(event.type){
			case SDL_QUIT:return 0;
			case SDL_KEYUP:if(!event.key.repeat)switch(event.key.keysym.sym){
				case SDLK_ESCAPE:return 0;
			}break;
		}

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glTranslatef(0,0,-4);
		glRotatef(SDL_GetTicks()*0.1,0,1,0);

		glBegin(GL_TRIANGLES);
			glVertex3f( 1, 0, 0);glVertex3f( 0, 1, 0);glVertex3f( 0, 0, 1);
			glVertex3f( 0, 0,-1);glVertex3f( 0,-1, 0);glVertex3f(-1, 0, 0);
		glEnd();

		SDL_GL_SwapWindow(window);
	}

	return 0;
}
