
/*+==========================================================================+*\
            ___  ____  ___  ____________   ______ ___________________ __
           / _ \/ __ \/ _ \/ __/ ___/ _ | / __/ //_/ __/_  __/ ___/ // /
          / // / /_/ / // / _// /__/ __ |_\ \/ ,< / _/  / / / /__/ _  / 
         /____/\____/____/___/\___/_/ |_/___/_/|_/___/ /_/  \___/_//_/  

\*+==========================================================================+*/

#include <map>
#include <list>
#include <ctime>
#include <cmath>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <functional>
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"
#include "GL/glu.h"
using namespace std;

#define rDouble ((double)std::rand()/RAND_MAX)

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

	unsigned int sw = 400;
	unsigned int sh = 400;

	auto screenshot = [&](char* filename){
		SDL_Surface* image = SDL_CreateRGBSurface(SDL_SWSURFACE,sw,sh,24,0x000000FF,0x0000FF00,0x00FF0000,0);
		glReadBuffer(GL_FRONT);
		glReadPixels(0,0,sw,sh,GL_RGB,GL_UNSIGNED_BYTE,image->pixels);
		SDL_SaveBMP(image,filename);
		SDL_FreeSurface(image);
	};

        // ------------------------------------ camera initialization //

	double cameraX =  15,cameraGoalX = 0,cameraGrabX = 0;
	double cameraY = -15,cameraGoalY = 0,cameraGrabY = 0;

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
		gluPerspective(45,(double)windowWidth/windowHeight,0.1,20);
		glMatrixMode(GL_MODELVIEW);
	};

	reshape(windowWidth,windowHeight);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        // --------------------------------------------- vector class //

	struct vec3{
		double x,y,z;
		vec3() = default;
		vec3(double x,double y,double z):x(x),y(y),z(z){}
		static vec3 rand(){
			double i = acos(2*rDouble-1),a = 2*M_PI*rDouble;
			return vec3(sin(i)*cos(a),sin(i)*sin(a),cos(i));
		}

		double sqMag(){return x*x+y*y+z*z;}
		double mag(){return sqrt(sqMag());}
		vec3 getNorm(){double i = 1.0/mag();return vec3(x*i,y*i,z*i);}
		vec3 normalize(){return *this = getNorm();}
		vec3 operator+(const vec3 &rhs){return vec3(x+rhs.x,y+rhs.y,z+rhs.z);}
		vec3 operator-(const vec3 &rhs){return vec3(x-rhs.x,y-rhs.y,z-rhs.z);}
		vec3 operator*(const double &rhs){return vec3(x*rhs,y*rhs,z*rhs);}
		void glVertex(){glVertex3f(x,y,z);}
		void glColor(){glColor3f(x,y,z);}

		void glHSV(){
			[&](double h,double s,double v){
				float c = v*s;
				h = fmod(h*6,6);
				float i = c*(1-abs(fmod(h,2)-1));
				     if (0 <= h && h < 1) vec3(c,i,0).glColor();
				else if (1 <= h && h < 2) vec3(i,c,0).glColor();
				else if (2 <= h && h < 3) vec3(0,c,i).glColor();
				else if (3 <= h && h < 4) vec3(0,i,c).glColor();
				else if (4 <= h && h < 5) vec3(i,0,c).glColor();
				else if (5 <= h && h < 6) vec3(c,0,i).glColor();
				else                      vec3(0,0,0).glColor();
			}((atan2(z,x)+2*M_PI)/(2*M_PI),1,acos(-y/sqMag())/M_PI);
		}
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
		{ 6, 1,10},{ 9, 0,11},{ 9,11, 2},{ 9, 2, 5},{ 7, 2,11},
	};

	function<void(vec3,vec3,vec3,int,bool)> icoRecurse = [&](vec3 v1,vec3 v2,vec3 v3,int depth,bool colorize){
		if(depth == 0){
			if(colorize)v3.glHSV();v3.glVertex();
			if(colorize)v2.glHSV();v2.glVertex();
			if(colorize)v1.glHSV();v1.glVertex();
			return;
		}vec3 v12 = (v1+v2).getNorm(),v23 = (v2+v3).getNorm(),v31 = (v3+v1).getNorm();
		icoRecurse(v1,v12,v31,depth-1,colorize);icoRecurse( v2,v23,v12,depth-1,colorize);
		icoRecurse(v3,v31,v23,depth-1,colorize);icoRecurse(v12,v23,v31,depth-1,colorize);
	};

	auto renderIco = [&](bool colorize){
		glBegin(GL_TRIANGLES);
			for(int i=0;i<20;++i)icoRecurse(icoInd[icoTri[i][0]].normalize(),
				icoInd[icoTri[i][1]].normalize(),icoInd[icoTri[i][2]].normalize(),4,colorize);
		glEnd();
	};

	int icoDList = glGenLists(1);
	glNewList(icoDList,GL_COMPILE);
		renderIco(false);
	glEndList();

	int colorDList = glGenLists(1);
	glNewList(colorDList,GL_COMPILE);
		renderIco(true);
	glEndList();

	int starDList = glGenLists(1);
	glNewList(starDList,GL_COMPILE);
	glBegin(GL_POINTS);
	for(int i=0;i<10000;++i){
		vec3 v = vec3::rand()*(2+rDouble*10);
		glColor4f(1,1,1,0.5-v.sqMag()*0.02);
		v.glVertex();
	}glEnd();
	glEndList();

        // ------------------------------------------ drawing related //

	bool mode = false; // false = drawing, true = color-picker
	vec3 currentColor = vec3(0.6,1,0).normalize();
	int currentLayer = 0;
	float renderedLayer = 0;
	list<vec3> currentStrokeLs;
	map<int,list<int>> layerLs;
	list<int> undoStack;
	vec3 pointerGoal,pointer;

	auto mouseToWorld = [&](int x,int y){
		int viewport[4];
		double modelview[16],projection[16];
		GLdouble posX,posY,posZ;
		glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
		glGetDoublev(GL_PROJECTION_MATRIX,projection);
		glGetIntegerv(GL_VIEWPORT,viewport);
		GLfloat windowX = x,windowY = viewport[3]-y,windowZ;
		glReadPixels(x,windowY,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&windowZ);
		gluUnProject(windowX,windowY,windowZ,modelview,projection,viewport,&posX,&posY,&posZ);
		return vec3(posX,posY,posZ);
	};

	auto renderTile = [&]{
		if(abs(pointer.mag()-1)<0.1){
			glColor4f(1,1,1,0.5);
			glBegin(GL_POINTS);
				pointer.glVertex();
			glEnd();
		}

		currentColor.glHSV();
		glBegin(GL_LINE_STRIP);
			for(auto i:currentStrokeLs)i.glVertex();
		glEnd();
		for(auto i:layerLs){
			glPushMatrix();
				double s = (i.first-renderedLayer)*0.01+1;
				glScalef(s,s,s);
				for(auto j:i.second)glCallList(j);
			glPopMatrix();
		}
	};

	auto renderFace = [&]{
		glColor3f(1,1,1);
		glBegin(GL_LINES);
			glVertex3f(0,1.02,0);
			glVertex3f(0,0.95,0);
		glEnd();

		glPushMatrix();
			for(int i=0;i<5;++i){
				renderTile();
				glRotatef(72,0,1,0);
			}
		glPopMatrix();
	};

	auto renderHemi = [&]{
		renderFace();
		glPushMatrix();
			for(int i=0;i<5;++i){
				glPushMatrix();
					glRotatef(63.43495,0,0,1);
					glRotatef(180,0,1,0);
					renderFace();
				glPopMatrix();
				glRotatef(72,0,1,0);
			}
		glPopMatrix();
	};

	auto renderDodec = [&]{
		glPushMatrix();
			renderHemi();
			glRotatef(180,0,0,1);
			renderHemi();
		glPopMatrix();
	};

	auto renderColorSelector = [&]{
		glBegin(GL_LINES);
			glColor3f(1,1,1);
			(currentColor*1.1).glVertex();
			(currentColor*1.3).glVertex();
		glEnd();
	};

        // ------------------------------------------------- controls //

	bool mouseBtn[3] = {false,false,false};

	auto controls = [&]{
		SDL_Event event;
		while(SDL_PollEvent(&event))switch(event.type){
			case SDL_QUIT:exit(0);break;
			case SDL_WINDOWEVENT:switch(event.window.event){
				case SDL_WINDOWEVENT_RESIZED:reshape(event.window.data1,event.window.data2);break;
			}break;
			case SDL_KEYUP:if(!event.key.repeat)switch(event.key.keysym.sym){
				case SDLK_ESCAPE:exit(0);break;
				case SDLK_F5:{
					reshape(sw,sh);
					system("mkdir tmp");
					for(int r=0;r<73;++r){ // TODO: fix first frame
						glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
						glLoadIdentity();
						glTranslatef(0,0,-4);
						glRotatef(-30,1,0,0);
						glRotatef(r,0,1,0);
						renderDodec();
						glPushMatrix();
							glScalef(0.99,0.99,0.99);
							glColor4f(0.02,0.02,0.02,0.9);
							glCallList(mode?colorDList:icoDList);
						glPopMatrix();
						SDL_GL_SwapWindow(window);
						char filename[16];
						sprintf(filename,"tmp/%02d.bmp",r);
						screenshot(filename);
					}stringstream ss;
					ss << "rm tmp/00.bmp; convert -delay 3 -loop 0 tmp/*.bmp " << time(0) << ".gif; rm tmp/*.bmp; rmdir tmp";
					system(ss.str().c_str());
				}break;
				case SDLK_z:if(event.key.keysym.mod && KMOD_CTRL){
					if(undoStack.size() <= 0)return;
					int layer = undoStack.back();
					undoStack.pop_back();
					layerLs[layer].pop_back();
				}break;
			}break;

			case SDL_MOUSEBUTTONDOWN:switch(event.button.button){
				case SDL_BUTTON_LEFT:
					mouseBtn[0] = true;
					break;
				case SDL_BUTTON_RIGHT:
					cameraGrabX = event.motion.x-cameraGoalX;
					cameraGrabY = event.motion.y-cameraGoalY;
					mouseBtn[2] = true;
					break;
			}break;

			case SDL_MOUSEMOTION:{
				int mouseX = event.motion.x,mouseY = event.motion.y;
				glClear(GL_DEPTH_BUFFER_BIT);
				glCallList(icoDList);
				pointerGoal = mouseToWorld(mouseX,mouseY);
				if(mouseBtn[0]){
					if(abs(pointer.mag()-1)<0.1){
						if(!mode)currentStrokeLs.push_back(pointer.getNorm());
						else currentColor = pointer.getNorm();
					}
				}if(mouseBtn[2]){
					cameraGoalX = mouseX-cameraGrabX;
					cameraGoalY = mouseY-cameraGrabY;
				}
			}break;

			case SDL_MOUSEWHEEL:
				if(mode)return;
				currentLayer -= event.wheel.y;
				break;

			case SDL_MOUSEBUTTONUP:switch(event.button.button){
				case SDL_BUTTON_LEFT:{
					mouseBtn[0] = false;
					if(mode)return;
					undoStack.push_back(currentLayer);
					int dList = glGenLists(1);
					glNewList(dList,GL_COMPILE);
						currentColor.glHSV();
						glBegin(GL_LINE_STRIP);
							for(auto i:currentStrokeLs)i.glVertex();
						glEnd();
					glEndList();
					currentStrokeLs.clear();
					layerLs[currentLayer].push_back(dList);
				}break;
				case SDL_BUTTON_MIDDLE:
					if(mouseBtn[0])return;
					mode = !mode;
					break;
				case SDL_BUTTON_RIGHT:mouseBtn[2] = false;break;
			}break;
		}
	};

        // ------------------------------------------------ main loop //

	while(true){
		SDL_Delay(10);
		unsigned long newTick = SDL_GetTicks();
		elapsed = newTick-tick;
		tick = newTick;

		glLoadIdentity();
		glTranslatef(0,0,-4);
		glRotatef(cameraY,1,0,0);
		glRotatef(cameraX,0,1,0);

		controls();
		updateCamera();

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glPushMatrix();
			glRotatef(tick*0.001,0,1,0);
			glCallList(starDList);
		glPopMatrix();

		if(abs(pointer.mag()-1)>0.1 && abs(pointerGoal.mag()-1)<0.1)pointer = pointerGoal;
		pointer = pointer+(pointerGoal-pointer)*elapsed*0.02;
		renderedLayer += (currentLayer-renderedLayer)*elapsed*0.01;
		if(!mode)renderDodec();
		else renderColorSelector();

		glPushMatrix();
			glScalef(0.99,0.99,0.99);
			glColor4f(0.02,0.02,0.02,0.9);
			glCallList(mode?colorDList:icoDList);
		glPopMatrix();

		SDL_GL_SwapWindow(window);
	}return 0;
}
