#include <list>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"
#include "GL/glu.h"
using namespace std;

#define rFloat ((float)std::rand()/RAND_MAX)

int main(int argc,char *argv[]){

        // ------------------------------------ window initialization //

	SDL_Window    *window;
	unsigned int   windowWidth  = 640;
	unsigned int   windowHeight = 480;
	unsigned long  elapsed,tick = 0;

	srand(time(0));
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

        // --------------------------------------------- vector class //

	struct vec3{
		float x,y,z;
		vec3() = default;
		vec3(float x,float y,float z):x(x),y(y),z(z){}

		static vec3 rand(){
			float i = acos(2*rFloat-1);
			float a = 2*M_PI*rFloat;
			return vec3(sin(i)*cos(a),
			            sin(i)*sin(a),
			            cos(i));
		}

		float sqMag()const{return x*x+y*y+z*z;}
		vec3 operator*(const float &rhs)const{return vec3(x*rhs,y*rhs,z*rhs);}

		void glVertex()const{glVertex3f(x,y,z);}
	};

        // ---------------------------------- geometry initialization //

	list<vec3> starLs;
	for(int i=0;i<10000;++i)starLs.push_back(vec3::rand()*(2+rFloat*10));

        // ------------------------------------------------ main loop //

	while(true){
		unsigned long newTick = SDL_GetTicks();
		elapsed = newTick-tick;
		tick = newTick;

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

		glPushMatrix();
			glRotatef(tick*0.001,0,1,0);
			glBegin(GL_POINTS);
			for(auto i:starLs){
				glColor4f(1,1,1,0.5-i.sqMag()*0.02);
				i.glVertex();
			}glEnd();
		glPopMatrix();

		SDL_GL_SwapWindow(window);
	}

	return 0;
}
