#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

int main(int argc,char *argv[]){

	SDL_Window *window;

	SDL_Init(SDL_INIT_EVERYTHING);
	atexit(SDL_Quit);
	SDL_GL_CreateContext(window = SDL_CreateWindow("Dodecasketch",
		SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
		640,480,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE));

	while(true){
		SDL_Event event;
		while(SDL_PollEvent(&event))switch(event.type){
			case SDL_QUIT:return 0;
			case SDL_KEYUP:if(!event.key.repeat)switch(event.key.keysym.sym){
				case SDLK_ESCAPE:return 0;
			}break;
		}

		SDL_GL_SwapWindow(window);
	}

	return 0;
}
