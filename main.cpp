//---------------------------------------------------------------------------
//
// This software is provided 'as-is' for assignment of COMP308
// in ECS, Victoria University of Wellington,
// without any express or implied warranty.
// In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
// Copyright (c) 2012 by Taehyun Rhee
//
// Edited by Roma Klapaukh, Daniel Atkins, and Taehyun Rhee
//
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "define.h"
#include "G308_Skeleton.h"
#include <iostream>
#include <vector>

GLuint g_mainWnd, g_frameWnd;
GLuint g_nWinWidth = G308_WIN_WIDTH;
GLuint g_nWinHeight = G308_WIN_HEIGHT;

void G308_keyboardListener(unsigned char, int, int);
void G308_Frame();
void G308_mouseListener(int, int, int, int);
void G308_Reshape(int w, int h);
void G308_display();
void G308_init();
void G308_motionListener(int, int);
void G308_SetCamera();
void G308_SetCamera2();
void G308_MainFrame();
void G308_SetLight();
void drawPickingMode();
int processPick(int, int);
void G308_animate();
void G308_frameMouseListener(int, int, int, int);

Skeleton* skeleton;
vector<G308_Point> splinePoints(0);
vector<G308_Point> interpolate(0);
int lastX = 0, lastY = 0, curX = 0, curY = 0, maxY = 0;
int arcball = false;
int addPoint = 0;
int currFrame = 0;
int animate = 0;


int main(int argc, char** argv) {

	if (argc < 2 || argc > 3) {
		//Usage instructions for core and challenge
		printf("Usage\n");
		printf("./Ass2 priman.asf [priman.amc]\n");
		exit(EXIT_FAILURE);
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(g_nWinWidth, g_nWinHeight);
	g_mainWnd = glutCreateWindow("COMP308 Assignment 3");

	glutKeyboardFunc(G308_keyboardListener);
	glutMotionFunc(G308_motionListener);
	glutMouseFunc(G308_mouseListener);
	glutDisplayFunc(G308_display);
	glutReshapeFunc(G308_Reshape);
	glutIdleFunc(G308_animate);

	G308_init();

	glutInitWindowSize(200, 200);

	g_frameWnd = glutCreateWindow("KeyFraming");
    glutMouseFunc(G308_frameMouseListener);
	glutPositionWindow(540,40);
	glutReshapeFunc(G308_Reshape);
    glutDisplayFunc(G308_Frame);

    glutKeyboardFunc(G308_keyboardListener);
    G308_SetCamera2();
    G308_SetLight();

	// [Assignment2] : Read ASF file
	skeleton = new Skeleton(argv[1]);

	glutMainLoop();

	return EXIT_SUCCESS;
}

// Init Light and Camera
void G308_init() {

	G308_SetLight();
	G308_SetCamera();
}

//Keyframe Window Display Function
void G308_Frame(){
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR){
        printf("%s\n", gluErrorString(err));
    }

    float dx, dy, xcr, ycr;  

    glBegin(GL_POINTS);
	for(std::vector<G308_Point>::iterator i = splinePoints.begin(); i != splinePoints.end(); i++)
		   glVertex2f(i->x, i->y);
    glEnd();

    glBegin(GL_LINE_STRIP);
	    for(std::vector<G308_Point>::iterator i = splinePoints.begin(); i != splinePoints.end() && (i+1) != splinePoints.end(); i++){
	        for(int k = 0;  k < 50; k++){
			   float t = k*0.02;  
               //--Eq. (7.34)--
               xcr = i->x + 0.5*t*(-(i-1)->x+(i+1)->x) 
                   + t*t*((i-1)->x - 2.5*i->x + 2*(i+1)->x - 0.5*(i+2)->x)
                   + t*t*t*(-0.5*(i-1)->x + 1.5*i->x - 1.5*(i+1)->x + 0.5*(i+2)->x);
               ycr = i->y + 0.5*t*(-(i-1)->y+(i+1)->y) 
                   + t*t*((i-1)->y - 2.5*i->y + 2*(i+1)->y - 0.5*(i+2)->y)
                   + t*t*t*(-0.5*(i-1)->y + 1.5*i->y - 1.5*(i+1)->y + 0.5*(i+2)->y);
 	           glVertex2f(xcr, ycr);
		   }
		}
		glEnd();
    // for (std::vector<G308_Point>::iterator i = splinePoints.begin(); i != splinePoints.end() && (i+1) != splinePoints.end(); i++){
    //     glVertex3f(i->x, i->y, i->z);
    //     glVertex3f((i+1)->x, (i+1)->y, (i+1)->z);
    // }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    glutSwapBuffers();
    //G308_display();
}

void G308_animate(){
	if (interpolate.size() != 0 && animate == 1){
		int startX = interpolate.at(0).x, startY = interpolate.at(0).y, startZ = interpolate.at(0).z;
		cout << "Interpolate Size = " << interpolate.size() << " && current Frame = " << currFrame << endl;
		skeleton->animate(interpolate.at(currFrame).x/100, (interpolate.at(currFrame).y)/100, interpolate.at(currFrame)./100, currFrame);
		G308_MainFrame();
		currFrame = (currFrame + 1)%interpolate.size();
	}
}

void G308_MainFrame(){

    glBegin(GL_LINES);
    float startx = 0, starty = 0, startz = 0, xcr, ycr;
    for (std::vector<G308_Point>::iterator i = splinePoints.begin(); i != splinePoints.end(); i++){
    	if (i == splinePoints.begin()){
    		startx = i->x;
    		starty = i->y;
    		startz = i->z;
    	}
        glPushMatrix();
        glTranslatef(((i->x)-startx)/100, ((i->z)-startz)/100, ((i->y)-starty)/100);
        glColor3f(1.0f, 1.0f, 1.0f);
        glutSolidSphere(0.05, 100, 100);
        glTranslatef(-((i->x)-startx)/100, -((i->z)-startz)/100, -((i->y)-starty)/100);
        glPopMatrix();
    }

    if (addPoint == 1){
    	interpolate.clear();
	    for(std::vector<G308_Point>::iterator i = splinePoints.begin(); i != splinePoints.end() && (i+1) != splinePoints.end(); i++){
	        for(int k = 0;  k < 50; k++){
			   float t = k*0.02;  
	           //--Eq. (7.34)--
	           xcr = i->x + 0.5*t*(-(i-1)->x+(i+1)->x) 
	               + t*t*((i-1)->x - 2.5*i->x + 2*(i+1)->x - 0.5*(i+2)->x)
	               + t*t*t*(-0.5*(i-1)->x + 1.5*i->x - 1.5*(i+1)->x + 0.5*(i+2)->x);
	           ycr = i->y + 0.5*t*(-(i-1)->y+(i+1)->y) 
	               + t*t*((i-1)->y - 2.5*i->y + 2*(i+1)->y - 0.5*(i+2)->y)
	               + t*t*t*(-0.5*(i-1)->y + 1.5*i->y - 1.5*(i+1)->y + 0.5*(i+2)->y);
	           glVertex2f(xcr, ycr);
	           G308_Point temp;
	           temp.x = xcr;
	           temp.y = ycr;
	           temp.z = i->z;

	           interpolate.push_back(temp);
		    }
		}
		addPoint = 0;
	}

	for (std::vector<G308_Point>::iterator i = interpolate.begin(); i != interpolate.end(); i++){
		if (i == interpolate.begin()){
			startx = i->x;
			starty = i->y;
			startz = i->z;
		}
		glPushMatrix();
		glTranslatef(((i->x)-startx)/100, ((i->z)-startz)/100, ((i->y)-starty)/100);
		glColor3f(1.0f, 1.0f, 1.0f);
		glutSolidSphere(0.01, 100, 100);
		glTranslatef(-((i->x)-startx)/100, -((i->z)-startz)/100, -((i->y)-starty)/100);
		glPopMatrix();
	}

	// gluDeleteQuadric(q);

    if (skeleton != NULL) {
		skeleton->display();
	}

}

// Display call back
void G308_display() {

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glShadeModel(GL_SMOOTH);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("%s\n", gluErrorString(err));
	}

    G308_MainFrame();

	// [Assignmet2] : render skeleton



	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);

	glutSwapBuffers();
}

void G308_keyboardListener(unsigned char key, int x, int y) {
	if (key == 'x'){
		if (animate == 1) animate = 0;
		else{animate = 1;}
	}
}

void G308_mouseListener(int button, int state, int x, int y){
	//0 = Left GLUT_LEFT_BUTTON
	//1 = Middle GLUT_MIDDLE_BUTTON
	//2 = Right GLUT_RIGHT_BUTTON
	//3 = MouseUp GLUT_SOMETHING
	//4 = MouseDown GLUT_SOMETHINGELSE
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		// drawPickingMode();
	// int check = processPick(x, y);
		cout << "Mouse Click" << endl;
		arcball = true;
		lastX = curX = x;
		lastY = curY = y;
	} else {
		arcball = false;
	}
}

void G308_motionListener(int x, int y){
	if (arcball){
		curX = x;
		curY = y;
		glRotatef((curX-lastX), 0, 1, 0);
		if (maxY > -180 && maxY < 180){
			glRotatef((curY-lastY), 1, 0, 0);
		}


		if ((maxY + (curY-lastY)) < 200 && (maxY + (curY-lastY)) > (-200)){
			maxY += (curY-lastY);
		}
		lastY = curY;
		lastX = curX;
		glutPostRedisplay();
	}
}

void G308_frameMouseListener(int button, int state, int x, int y){
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
    	cout << "Meow" << endl;
        GLint viewport[4]; 
        GLdouble modelview[16]; 
        GLdouble projection[16]; 
        GLfloat windowX, windowY, windowZ;
        GLdouble worldX, worldY, worldZ;

        glGetDoublev( GL_MODELVIEW_MATRIX, modelview ); //get the modelview info
        glGetDoublev( GL_PROJECTION_MATRIX, projection ); //get the projection matrix info
        glGetIntegerv( GL_VIEWPORT, viewport ); //get the viewport info

        windowX = (float)x;
        windowY = (float)viewport[3] - (float)y;
        windowZ = 0;

        //get the world coordinates from the screen coordinates
        gluUnProject( windowX, windowY, windowZ, modelview, projection, viewport, &worldX, &worldY, &worldZ);

        struct G308_Point temp;
        temp.x = worldX;
        temp.y = worldY;
        temp.z = worldZ;
        splinePoints.push_back(temp);
        cout << x << " x | y " << y << endl;
        cout << splinePoints.size() << endl;
        addPoint = 1;
        glutPostRedisplay();
    }
}

int processPick (int x, int y){
	GLint viewport[4];
	GLubyte pixel[3];

	glGetIntegerv(GL_VIEWPORT,viewport);

	glReadPixels(x,viewport[3]-y,1,1,
		GL_RGB,GL_UNSIGNED_BYTE,(void *)pixel);

	printf("%d %d %d\n",pixel[0],pixel[1],pixel[2]);
	if (pixel[0] == 255) {
		printf ("You picked the 1st snowman on the 1st row");
		return 1;
	}
	else if (pixel[1] == 255){
		printf ("You picked the 1st snowman on the 2nd row");
		return 1;
	}
	else if (pixel[2] == 255){
		printf ("You picked the 2nd snowman on the 1st row");
		return 1;
	}
	else if (pixel[0] == 250){
		printf ("You picked the 2nd snowman on the 2nd row");
		return 1;
	}
	else{
		printf("You didn't click a snowman!");
		return 0;
	}
	printf ("\n");

}

void drawPickingMode() {

// Draw 4 SnowMen


	glDisable(GL_DITHER);
	for(int i = 0; i < 2; i++)
		for(int j = 0; j < 2; j++) {
			glPushMatrix();
			glScalef(0.05, 0.05, 0.05);

// A different color for each snowman

			switch (i*2+j) {
				case 0: glColor3ub(255,0,0);break;
				case 1: glColor3ub(0,255,0);break;
				case 2: glColor3ub(0,0,255);break;
				case 3: glColor3ub(250,0,250);break;
			}

			glTranslatef(i*3.0,0,-j * 3.0);
			glutSolidSphere(1, 100, 100);
			glPopMatrix();
		}
		glEnable(GL_DITHER);
	}

// Reshape function
	void G308_Reshape(int w, int h) {
		if (h == 0)
			h = 1;

		g_nWinWidth = w;
		g_nWinHeight = h;

		glViewport(0, 0, g_nWinWidth, g_nWinHeight);
	}

// Set Camera Position
	void G308_SetCamera() {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(G308_FOVY, (double) g_nWinWidth / (double) g_nWinHeight, G308_ZNEAR_3D, G308_ZFAR_3D);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		gluLookAt(0.0, 0.0, 7.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	}

    void G308_SetCamera2(){
        glViewport( 0,0, 500, 500 );
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( 0.0, 200.0, 0.0, 200.0, 1.0, -1.0 );

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

// Set View Position
	void G308_SetLight() {
		float direction[] = { 1.0f, 1.0f, 1.0f, 0.0f };
		float diffintensity[] = { 0.5f, 0.5f, 0.5f, 1.0f };
		float ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

		glLightfv(GL_LIGHT0, GL_POSITION, direction);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffintensity);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

		glEnable(GL_LIGHT0);
	}

