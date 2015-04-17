#include <list>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <functional>
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

	float cameraX = 0,cameraGoalX = 0,cameraGrabX = 0;
	float cameraY = 0,cameraGoalY = 0,cameraGrabY = 0;

	auto updateCamera = [&]{
		if(cameraGoalY < -90)cameraGoalY = -90;
		if(cameraGoalY >  90)cameraGoalY =  90;
		cameraX += (cameraGoalX-cameraX)*elapsed*0.01;
		cameraY += (cameraGoalY-cameraY)*elapsed*0.01;
	};

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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        // --------------------------------------------- vector class //

	struct vec3{
		float x,y,z;
		vec3() = default;
		vec3(float x,float y,float z):x(x),y(y),z(z){}
		static vec3 rand(){
			float i = acos(2*rFloat-1),a = 2*M_PI*rFloat;
			return vec3(sin(i)*cos(a),sin(i)*sin(a),cos(i));
		}

		float sqMag(){return x*x+y*y+z*z;}
		float mag(){return sqrt(sqMag());}
		vec3 getNorm(){float i = 1.0/mag();return vec3(x*i,y*i,z*i);}
		vec3 normalize(){return *this = getNorm();}
		vec3 operator+(const vec3 &rhs){return vec3(x+rhs.x,y+rhs.y,z+rhs.z);}
		vec3 operator*(const float &rhs){return vec3(x*rhs,y*rhs,z*rhs);}
		void glVertex(){glVertex3f(x,y,z);}
	};

        // ---------------------------------- geometry initialization //

	#define icoX .525731112119133606
	#define icoZ .850650808352039932

	vec3 icoInd[12] = {
		vec3(-icoX,  0.0, icoZ),vec3(icoX,  0.0, icoZ),vec3(-icoX,  0.0,-icoZ),
		vec3( icoX,  0.0,-icoZ),vec3( 0.0, icoZ, icoX),vec3(  0.0, icoZ,-icoX),
		vec3(  0.0,-icoZ, icoX),vec3( 0.0,-icoZ,-icoX),vec3( icoZ, icoX,  0.0),
		vec3(-icoZ, icoX,  0.0),vec3(icoZ,-icoX,  0.0),vec3(-icoZ,-icoX,  0.0),
	};

	const int icoTri[20][3]={ 
		{ 0, 4, 1},{ 0, 9, 4},{ 9, 5, 4},{ 4, 5, 8},{ 4, 8, 1},
		{ 8,10, 1},{ 8, 3,10},{ 5, 3, 8},{ 5, 2, 3},{ 2, 7, 3},
		{ 7,10, 3},{ 7, 6,10},{ 7,11, 6},{11, 0, 6},{ 0, 1, 6},
		{ 6, 1,10},{ 9, 0,11},{ 9,11, 2},{ 9, 2, 5},{ 7, 2,11}
	};

	function<void(vec3,vec3,vec3,int)> icoRecurse = [&](vec3 v1,vec3 v2,vec3 v3,int depth){
		if(depth == 0){v3.glVertex();v2.glVertex();v1.glVertex();return;}
		vec3 v12 = (v1+v2).getNorm(),v23 = (v2+v3).getNorm(),v31 = (v3+v1).getNorm();
		icoRecurse(v1,v12,v31,depth-1);icoRecurse( v2,v23,v12,depth-1);
		icoRecurse(v3,v31,v23,depth-1);icoRecurse(v12,v23,v31,depth-1);
	};

	int icoDList = glGenLists(1);
	glNewList(icoDList,GL_COMPILE);
	glBegin(GL_TRIANGLES);
		for(int i=0;i<20;++i)icoRecurse(icoInd[icoTri[i][0]].normalize(),
			icoInd[icoTri[i][1]].normalize(),icoInd[icoTri[i][2]].normalize(),4);
	glEnd();
	glEndList();

	int starDList = glGenLists(1);
	glNewList(starDList,GL_COMPILE);
	glBegin(GL_POINTS);
	for(int i=0;i<10000;++i){
		vec3 v = vec3::rand()*(2+rFloat*10);
		glColor4f(1,1,1,0.5-v.sqMag()*0.02);
		v.glVertex();
	}glEnd();
	glEndList();

        // ------------------------------------------------- controls //

	bool mouseBtn[3] = {false,false,false};

	auto controls = [&]{
		SDL_Event event;
		while(SDL_PollEvent(&event))switch(event.type){
			case SDL_QUIT:exit(0);
			case SDL_KEYUP:if(!event.key.repeat)switch(event.key.keysym.sym){
				case SDLK_ESCAPE:exit(0);
			}break;

			case SDL_MOUSEBUTTONDOWN:switch(event.button.button){
				case SDL_BUTTON_RIGHT:
					cameraGrabX = event.motion.x-cameraGoalX;
					cameraGrabY = event.motion.y-cameraGoalY;
					mouseBtn[2] = true;
					break;
			}break;

			case SDL_MOUSEMOTION:
				if(mouseBtn[2]){
					cameraGoalX = event.motion.x-cameraGrabX;
					cameraGoalY = event.motion.y-cameraGrabY;
				}
			break;

			case SDL_MOUSEBUTTONUP:switch(event.button.button){
				case SDL_BUTTON_RIGHT:mouseBtn[2] = false;break;
			}break;
		}
	};

        // ------------------------------------------------ main loop //

	while(true){
		unsigned long newTick = SDL_GetTicks();
		elapsed = newTick-tick;
		tick = newTick;

		controls();
		updateCamera();

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glTranslatef(0,0,-4);
		glRotatef(cameraY,1,0,0);
		glRotatef(cameraX,0,1,0);

		glPushMatrix();
			glRotatef(tick*0.001,0,1,0);
			glCallList(starDList);
		glPopMatrix();

		glColor4f(1,0.6,0,0.7);
		glCallList(icoDList);

		SDL_GL_SwapWindow(window);
	}

	return 0;
}
