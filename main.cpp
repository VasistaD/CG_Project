
#include <time.h>
#include"dhead1.h"
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <list>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

using namespace std;

static int window, returnmenu, returnsubmenu, returnsubmenucolor1, returnsubmenucolor2,value = 0;
void *font = GLUT_BITMAP_TIMES_ROMAN_24;
int window_ID;
float red = 0.0, green = 0.0, blue = 0.0;
int tmpx, tmpy; // store the first point when shape is line, rectangle or circle
int brushSize = 4;
int eraserSize = 1;
bool isSecond = false;
bool isRandom = false;
bool isEraser = false;
bool isRadial = false;
float window_w = 1500;
float window_h = 900;

int shape = 1; // 1:point, 2:line, 3:rectangle, 4:circle, 5:brush

std::vector<dh1::Dot> dots;		// store all the points until clear
std::list<int> undoHistory; // record for undo, maximum 20 shapes in history
std::list<int> redoHistory; // record for redo, maximum 20 shapes in history
std::vector<dh1::Dot> redoDots;  // store the dots after undo temporaly

void display(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(2);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i < dots.size(); i++)
	{
		glColor3f(dots[i].getR(), dots[i].getG(), dots[i].getB());
		glVertex2i(dots[i].getX(), dots[i].getY());
	}
	glEnd();
	glutSwapBuffers();
}

void clear()
{
	dots.clear();
	undoHistory.clear();
	redoDots.clear();
	redoHistory.clear();
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	std::cout << asctime(timeinfo)
			  << "[Info] The window and the history are cleared successfully.\n";
}

void quit()
{
	std::cout << "Thank you for using this Paint tool! Goodbye!" << std::endl;
	exit(0);
}
void undo()
{
	if (undoHistory.size() > 0)
	{
		if (undoHistory.back() != dots.size() && redoHistory.back() != dots.size())
		{
			redoHistory.push_back(dots.size());
		}
		int numRemove = dots.size() - undoHistory.back();
		for (int i = 0; i < numRemove; i++)
		{
			redoDots.push_back(dots.back());
			dots.pop_back();
		}
		redoHistory.push_back(undoHistory.back());
		undoHistory.pop_back();
	}
	else
	{
		time_t rawtime;
		struct tm *timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		std::cout << asctime(timeinfo)
				  << "[Warning] Cannot undo. This is the first record in the history.\n";
	}
}

void redo()
{
	if (redoHistory.size() > 1)
	{
		undoHistory.push_back(redoHistory.back());
		redoHistory.pop_back();
		int numRemove = redoHistory.back() - dots.size();
		for (int i = 0; i < numRemove; i++)
		{
			dots.push_back(redoDots.back());
			redoDots.pop_back();
		}
	}
	else
	{
		time_t rawtime;
		struct tm *timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		std::cout << asctime(timeinfo)
				  << "[Warning] Cannot redo. This is the last record in the history.\n";
	}
}

void drawDot(int mousex, int mousey)
{
	dh1::Dot newDot(mousex, window_h - mousey, isEraser ? 1.0 : red, isEraser ? 1.0 : green, isEraser ? 1.0 : blue);
	dots.push_back(newDot);
}

void drawBrush(int x, int y)
{
	for (int i = 0; i < brushSize; i++)
	{
		int randX = rand() % (brushSize + 1) - brushSize / 2 + x;
		int randY = rand() % (brushSize + 1) - brushSize / 2 + y;
		drawDot(randX, randY);
	}
}

void drawLine(int x1, int y1, int x2, int y2)
{
	bool changed = false;
	// Bresenham's line algorithm
	if (abs(x2 - x1) < abs(y2 - y1))
	{
		int tmp1 = x1;
		x1 = y1;
		y1 = tmp1;
		int tmp2 = x2;
		x2 = y2;
		y2 = tmp2;
		changed = true;
	}
	int dx = x2 - x1;
	int dy = y2 - y1;
	int yi = 1;
	int xi = 1;
	if (dy < 0)
	{
		yi = -1;
		dy = -dy;
	}
	if (dx < 0)
	{
		xi = -1;
		dx = -dx;
	}
	int d = 2 * dy - dx;
	int incrE = dy * 2;
	int incrNE = 2 * dy - 2 * dx;

	int x = x1, y = y1;
	if (changed)
		drawDot(y, x);
	else
		drawDot(x, y);
	while (x != x2)
	{
		if (d <= 0)
			d += incrE;
		else
		{
			d += incrNE;
			y += yi;
		}
		x += xi;
		if (changed)
			drawDot(y, x);
		else
			drawDot(x, y);
	}
}


void drawRectangle(int x1, int y1, int x2, int y2)
{
    drawLine(x1, y1, x2, y1);
    drawLine(x2, y1, x2, y2);
    drawLine(x2, y2, x1, y2);
    drawLine(x1, y2, x1, y1);
}

/**
 * Midpoint circle algorithm
 */
void drawCircle(int x1, int y1, int x2, int y2)
{
	int r = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
	double d;
	int x, y;

	x = 0;
	y = r;
	d = 1.25 - r;

	while (x <= y)
	{
		drawDot(x1 + x, y1 + y);
		drawDot(x1 - x, y1 + y);
		drawDot(x1 + x, y1 - y);
		drawDot(x1 - x, y1 - y);
		drawDot(x1 + y, y1 + x);
		drawDot(x1 - y, y1 + x);
		drawDot(x1 + y, y1 - x);
		drawDot(x1 - y, y1 - x);
		x++;
		if (d < 0)
		{
			d += 2 * x + 3;
		}
		else
		{
			y--;
			d += 2 * (x - y) + 5;
		}
	}
}

void drawRadialBrush(int x, int y)
{
	int xc = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int yc = glutGet(GLUT_WINDOW_HEIGHT) / 2;
	int dx, dy;

	dx = xc - x;
	dy = yc - y;

	drawDot(xc + dx, yc + dy);
	drawDot(xc - dx, yc + dy);
	drawDot(xc + dx, yc - dy);
	drawDot(xc - dx, yc - dy);
	drawDot(xc + dy, yc + dx);
	drawDot(xc - dy, yc + dx);
	drawDot(xc + dy, yc - dx);
	drawDot(xc - dy, yc - dx);
}

void erase(int x, int y)
{
	for (int i = -eraserSize; i <= eraserSize; i++)
	{
		for (int j = -eraserSize; j <= eraserSize; j++)
		{
			drawDot(x + i, y + j);
		}
	}
}

void keyboard(unsigned char key, int xIn, int yIn)
{
	isSecond = false;
	switch (key)
	{
	case 'q':
	case 27: // 27 is the esc key
		quit();
		break;
	case 'c':
		clear();
		break;
	case '+':
		if (shape == 5 && !isEraser)
		{
			if (brushSize < 16)
				brushSize += 4;
			else
			{
				time_t rawtime;
				struct tm *timeinfo;
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				std::cout << asctime(timeinfo)
						  << "[Warning] Airbrush's size cannot be larger. It is already the largest.\n";
			}
		}
		else if (isEraser)
		{
			if (eraserSize < 10)
				eraserSize += 4;
			else
			{
				time_t rawtime;
				struct tm *timeinfo;
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				std::cout << asctime(timeinfo)
						  << "[Warning] Eraser's size cannot be larger. It is already the largest.\n";
			}
		}
		break;
	case '-':
		if (shape == 5 && !isEraser)
		{
			if (brushSize > 4)
				brushSize -= 4;
			else
			{
				time_t rawtime;
				struct tm *timeinfo;
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				std::cout << asctime(timeinfo)
						  << "[Warning] Airbrush's size cannot be smaller. It is already the smallest.\n";
			}
		}
		else if (isEraser)
		{
			if (eraserSize > 2)
				eraserSize -= 4;
			else
			{
				time_t rawtime;
				struct tm *timeinfo;
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				std::cout << asctime(timeinfo)
						  << "[Warning] Eraser's size cannot be smaller. It is already the smallest.\n";
			}
		}
		break;
	case 'u':
		undo();
		break;
	case 'r':
		redo();
		break;
	}
}

void mouse(int bin, int state, int x, int y)
{
	if (bin == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (isRandom)
		{
			srand(time(NULL));
			red = float(rand()) / float(RAND_MAX);
			green = float(rand()) / float(RAND_MAX);
			blue = float(rand()) / float(RAND_MAX);
		}
		if (isEraser)
		{
			undoHistory.push_back(dots.size());
			erase(x, y);
		}
		else
		{
			if (shape == 1)
			{
				undoHistory.push_back(dots.size());
				if (isRadial)
					drawRadialBrush(x, y);
				else
					drawDot(x, y);
			}
			else if (shape == 5)
			{
				undoHistory.push_back(dots.size());
				drawBrush(x, y);
			}
			else
			{
				if (!isSecond)
				{
					tmpx = x;
					tmpy = y;
					isSecond = true;
				}
				else
				{
					if (undoHistory.back() != dots.size())
						undoHistory.push_back(dots.size());
					if (shape == 2)
						drawLine(tmpx, tmpy, x, y);
					else if (shape == 3)
						drawRectangle(tmpx, tmpy, x, y);
					else if (shape == 4)
						drawCircle(tmpx, tmpy, x, y);
					isSecond = false;
				}
			}
		}
		if (undoHistory.size() > 20)
		{
			undoHistory.pop_front();
		}
	}
}

void motion(int x, int y)
{
	if (isEraser)
		erase(x, y);
	else
	{
		if (shape == 1)
		{
			if (isRadial)
				drawRadialBrush(x, y);
			else
				drawDot(x, y);
		}
		if (shape == 5)
			drawBrush(x, y);
	}
}

void reshape(int w, int h)
{
	window_w = w;
	window_h = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}

void processMainMenu(int value)
{
	switch (value)
	{
	case 0:
		quit();
		break;
	case 1:
		clear();
		break;
	case 2:
		undo();
		break;
	case 3:
		redo();
		break;
	}
}

void processBrushSizeMenu(int value)
{
	shape = 5;
	isEraser = false;
	brushSize = value;
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
}

void processColourMenu(int value)
{
	isSecond = false;
	isEraser = false;
	isRandom = false;

	switch (value)
	{
	case 0: // black
		red = 0.0;
		green = 0.0;
		blue = 0.0;
		break;
	case 1: // red
		red = 1.0;
		green = 0.0;
		blue = 0.0;
		break;
	case 2: // green
		red = 0.0;
		green = 1.0;
		blue = 0.0;
		break;
	case 3: // blue
		red = 0.0;
		green = 0.0;
		blue = 1.0;
		break;
	case 4: // purple
		red = 0.5;
		green = 0.0;
		blue = 0.5;
		break;
	case 5: // yellow
		red = 1.0;
		green = 1.0;
		blue = 0.0;
		break;
    case 6: // pink
		red = 1.0;
		green = 0.75;
		blue = 0.8;
		break;
    case 7: // grey
		red = 0.5;
		green = 0.5;
		blue = 0.5;
		break;
	case 8: // random
		isRandom = true;
		break;

	}
}

void processShapeMenu(int value)
{
	shape = value;
	isEraser = false;
	isSecond = false;
	isRadial = false;

	switch (shape)
	{
	case 1:
		glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
		break;
	case 2:
	case 3:
	case 4:
		glutSetCursor(GLUT_CURSOR_CROSSHAIR);
		break;
	}
}

void processEraserSizeMenu(int value)
{
	glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
	eraserSize = value;
	isEraser = true;
}

void processRadicalBrushMenu(int value)
{
	isRadial = value == 1 ? true : false;
}

void createOurMenu()
{
	int colourMenu = glutCreateMenu(processColourMenu);
	glutAddMenuEntry("Black",0);
	glutAddMenuEntry("Red", 1);
	glutAddMenuEntry("Green", 2);
	glutAddMenuEntry("Blue", 3);
	glutAddMenuEntry("Purple", 4);
	glutAddMenuEntry("Yellow", 5);
	glutAddMenuEntry("Pink", 6);
	glutAddMenuEntry("Grey", 7);
	glutAddMenuEntry("Random", 8);

	int sizeMenu = glutCreateMenu(processBrushSizeMenu);
	glutAddMenuEntry("4px", 4);
	glutAddMenuEntry("8px", 8);
	glutAddMenuEntry("12px", 12);
	glutAddMenuEntry("16px", 16);

	int shapeMenu = glutCreateMenu(processShapeMenu);
	glutAddMenuEntry("Point", 1);
	glutAddMenuEntry("Line", 2);
	glutAddMenuEntry("Rectangle", 3);
	glutAddMenuEntry("Circle", 4);
	glutAddSubMenu("Airbrush", sizeMenu);

	int eraserSizeMenu = glutCreateMenu(processEraserSizeMenu);
	glutAddMenuEntry("Small", 2);
	glutAddMenuEntry("Medium", 6);
	glutAddMenuEntry("Large", 10);

	int radicalBrushMenu = glutCreateMenu(processRadicalBrushMenu);
	glutAddMenuEntry("True", 1);
	glutAddMenuEntry("False", 2);

	int main_id = glutCreateMenu(processMainMenu);
	glutAddSubMenu("Colour", colourMenu);
	glutAddSubMenu("Shapes", shapeMenu);
	//glutAddSubMenu("Radical Paint Brush", radicalBrushMenu);
	glutAddSubMenu("Eraser", eraserSizeMenu);
	glutAddMenuEntry("Undo", 2);
	glutAddMenuEntry("Redo", 3);
	glutAddMenuEntry("Clear", 1);
	glutAddMenuEntry("Quit", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void init(void)
{
	/* background color */
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glColor3f(red, green, blue);

	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, window_w, 0.0, window_h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void FPS(int val)
{
	glutPostRedisplay();
	glutTimerFunc(0, FPS, 0);
}

void callbackInit()
{
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(17, FPS, 0);
}

void printGuide()
{
	std::cout << "#########################################################################\n"
			  << "#                    Welcome to use this Paint tool!                    #\n"
			  << "#########################################################################\n"
			  << "A list of commands:\n"
			  << "Right click\t"
			  << "-> show menu\n"
			  << "Left click\t"
			  << "-> choose option\n"
			  << "Menu \"Colour\"\t"
			  << "-> You can choose Red, Green, Blue, Yellow or Random, the default color is Red.\n"
			  << "Menu \"Shapes\"\t"
			  << "-> The default shape is Point.\n"
			  << "\tPoint\t\t"
			  << "-> draw a dot at the point clicked with the mouse. Clicking and dragging will draw points constantly like free-form drawing.\n"
			  << "\tLine\t\t"
			  << "-> draw a linebetween two subseauently clicked points. (Bresenham's algorithm is used here.)\n"
			  << "\tRectangle\t"
			  << "-> draw a rectangle with top-left corner specified by the first click and the bottom-right corner specified by a second click. If the second click is bottom-left, top-right or top-left comparing to the first click, a warning will show in the console.\n"
			  << "\tCircle\t\t"
			  << "-> draw a circle centered at the position of the first click, with its radius set by a second click.\n"
			  << "\tAirbrush\t"
			  << "-> draw multiple points as brush around the clicked point. There are four options of size.\n"
			  << "Menu \"Eraser\"\t"
			  << "-> erase the points by clicking and dragging.\n"
			  << "Menu \"Undo\"\t"
			  << "-> undo, the history can keep maximum 20 records.\n"
			  << "Menu \"Redo\"\t"
			  << "->  redo, the history can keep maximum 20 records.\n"
			  << "Menu \"Clear\"\t"
			  << "-> clear all the points and clear the history.\n"
			  << "Menu \"Quit\"\t"
			  << "-> close the window.\n"
			  << "Keyboard 'q'\t"
			  << "-> close the window.\n"
			  << "Keyboard 'esc'\t"
			  << "-> close the window.\n"
			  << "Keyboard 'c'\t"
			  << "-> clear all the points and clear the history.\n"
			  << "Keyboard '+'\t"
			  << "-> larger size of eraser or brush.\n"
			  << "Keyboard '-'\t"
			  << "-> smaller size of eraser or brush.\n"
			  << "Keyboard 'u'\t"
			  << "-> undo, the history can keep maximum 20 records.\n"
			  << "Keyboard 'r'\t"
			  << "-> redo, the history can keep maximum 20 records.\n"
			  << "################################# Paint #################################" << std::endl;
}



void *fonts[] =
{
  GLUT_BITMAP_9_BY_15,
  GLUT_BITMAP_TIMES_ROMAN_10,
  GLUT_BITMAP_TIMES_ROMAN_24
};
void menu(int n){
if (n == 0)
{
	glutDestroyWindow(window);
	exit(0);
}

else
{
	value = n;
}
glutPostRedisplay();
}
void createMenu(void)
{
    returnsubmenucolor1 = glutCreateMenu(menu);
    glutAddMenuEntry("Black", 12);
    glutAddMenuEntry("Red", 11);
    glutAddMenuEntry("Blue", 13);
    glutAddMenuEntry("Green", 14);


    returnsubmenucolor2 = glutCreateMenu(menu);
    glutAddMenuEntry("Black", 22);
    glutAddMenuEntry("Red", 21);
    glutAddMenuEntry("Blue", 23);
    glutAddMenuEntry("Green", 24);

    returnmenu = glutCreateMenu(menu); //function to call menu function and return value
    glutAddSubMenu("Start Page", returnsubmenucolor1);
    glutAddSubMenu("Instructions",returnsubmenucolor2);
    glutAddMenuEntry("Start the Drawing Tool",30);
    glutAddMenuEntry("Quit", 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void output(int x, int y, char *string)
{
  int len, i;

  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
}


void displayf(void){

    glClear(GL_COLOR_BUFFER_BIT);
    output(500, 675, " COMPUTER GRAPHICS PROJECT ");
    output(600, 625, "DRAWING TOOL");
    output(565, 400, "RIGHT Click to Navigate");
    output(850, 150, "Submitted by: Vasista D  (1PE17CS168)");
    output(900, 100, "            : Suyash Kumar Dubli  (1PE17CS158)");
    output(900, 50, "            : Sushil Kumar AS  (1PE17CS157)");
    glutSwapBuffers();


    glClear(GL_COLOR_BUFFER_BIT);
    if (value == 11){
        glPushMatrix();
        glColor3d(1.0, 0.0, 0.0);
        //glutDisplayFunc(display2);
        //glutPostRedisplay();
        glClear(GL_COLOR_BUFFER_BIT);
        output(500, 675, " COMPUTER GRAPHICS PROJECT ");
        output(600, 625, "DRAWING TOOL");
        output(565, 400, "RIGHT Click to Navigate");
        output(850, 150, "Submitted by: Vasista D  (1PE17CS168)");
        output(900, 100, "            : Suyash Kumar Dubli  (1PE17CS158)");
        output(900, 50, "            : Sushil Kumar AS  (1PE17CS157)");
        glutSwapBuffers();
        glPopMatrix();
    }

    else if (value == 12){
        glPushMatrix();
        glColor3d(0.0, 0.0, 0.0);
        //glutDisplayFunc(display2);
        glClear(GL_COLOR_BUFFER_BIT);
        output(500, 675, " COMPUTER GRAPHICS PROJECT ");
        output(600, 625, "DRAWING TOOL");
        output(565, 400, "RIGHT Click to Navigate");
        output(850, 150, "Submitted by: Vasista D  (1PE17CS168)");
        output(900, 100, "            : Suyash Kumar Dubli  (1PE17CS158)");
        output(900, 50, "            : Sushil Kumar AS  (1PE17CS157)");
        glutSwapBuffers();
        glPopMatrix();
    }

    else if (value == 13){
        glPushMatrix();
        glColor3d(0.0, 0.0, 1.0);
        //glutDisplayFunc(display2);
        glClear(GL_COLOR_BUFFER_BIT);
        output(500, 675, " COMPUTER GRAPHICS PROJECT ");
        output(600, 625, "DRAWING TOOL");
        output(565, 400, "RIGHT Click to Navigate");
        output(850, 150, "Submitted by: Vasista D  (1PE17CS168)");
        output(900, 100, "            : Suyash Kumar Dubli  (1PE17CS158)");
        output(900, 50, "            : Sushil Kumar AS  (1PE17CS157)");
        glutSwapBuffers();
        glPopMatrix();
    }

    else if (value == 14){
        glPushMatrix();
        glColor3d(0.0, 1.0, 0.0);
        //glutDisplayFunc(display2);
        glClear(GL_COLOR_BUFFER_BIT);
        output(500, 675, " COMPUTER GRAPHICS PROJECT ");
        output(600, 625, "DRAWING TOOL");
        output(565, 400, "RIGHT Click to Navigate");
        output(850, 150, "Submitted by: Vasista D  (1PE17CS168)");
        output(900, 100, "            : Suyash Kumar Dubli  (1PE17CS158)");
        output(900, 50, "            : Sushil Kumar AS  (1PE17CS157)");
        glutSwapBuffers();
        glPopMatrix();
    }

    else if (value == 21)
    {
        glPushMatrix();
        glColor3d(1.0,0.0,0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        output(475, 680, "A GUIDE TO PAINT TOOL!!");
        output(100, 650, "List of Commands:");
        output(100, 615, "1: Right click -> show menu ");
        output(100, 590, "2: Left click  -> choose option ");
        output(100, 565, "3: Menu \"Color\"  -> You can choose Red, Green, Blue, Yellow or Random, the default color is Red.");
        output(100, 540, "4: Menu \"Shapes\"  -> The default shape is Point.");
        output(100, 515, "5: Point  -> draw a dot at the point clicked with the mouse. Clicking and dragging will draw points constantly like free-form ");
        output(115, 490, " drawing.");
        output(100, 465, "6: Line  -> Draw a linebetween two subseauently clicked points.(Bresenham's algorithm is used here.");
        output(100, 440, "7: Rectangle  -> draw a rectangle with top-left corner specified by the first click and the bottom-right corner specified ");
        output(115, 415, " by a second click. If the second click is bottom-left, top-right or top-left comparing to the first click, a warning will show ");
        output(115, 390, " in the console.");
        output(100, 365, "8: Circle  -> draw a circle centered at the position of the first click, with its radius set by a second click.");
        output(100, 340, "9: Airbrush  -> draw multiple points as brush around the clicked point.");
        output(90, 315, "10: Menu \"Eraser\"  -> erase the points by clicking and dragging.");
        output(90, 290, "11: Menu \"Undo\"  -> undo, the history can keep maximum 20 records.");
        output(90, 265, "12: Menu \"Redo\"  ->  redo, the history can keep maximum 20 records. ");
        output(90, 240, "13: Menu \"Clear\"  -> clear all the points and clear the history.  ");
        output(90, 215, "14: Menu \"Quit\"  -> close the window.");
        output(90, 190, "15: Keyboard 'q'  -> close the window.");
        output(90, 165, "16: Keyboard 'esc'  -> close the window. ");
        output(90, 140, "17: Keyboard 'c'    -> clear all the points and clear the history. ");
        output(90, 115, "18: Keyboard '+'    -> larger size of eraser or brush.");
        output(90, 90, "19: Keyboard '-'    -> smaller size of eraser or brush.");
        output(90, 65, "20: Keyboard 'u'    -> undo, the history can keep maximum 20 records.");
        output(90, 40, "21: Keyboard 'r'    -> redo, the history can keep maximum 20 records.");
        glutSwapBuffers();
        glPopMatrix();
    }

    else if (value == 22)
    {
        glPushMatrix();
        glColor3d(0.0,0.0,0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        output(475, 680, "A GUIDE TO PAINT TOOL!!");
        output(100, 650, "List of Commands:");
        output(100, 615, "1: Right click -> show menu ");
        output(100, 590, "2: Left click  -> choose option ");
        output(100, 565, "3: Menu \"Color\"  -> You can choose Red, Green, Blue, Yellow or Random, the default color is Red.");
        output(100, 540, "4: Menu \"Shapes\"  -> The default shape is Point.");
        output(100, 515, "5: Point  -> draw a dot at the point clicked with the mouse. Clicking and dragging will draw points constantly like free-form ");
        output(115, 490, " drawing.");
        output(100, 465, "6: Line  -> Draw a linebetween two subseauently clicked points.(Bresenham's algorithm is used here.");
        output(100, 440, "7: Rectangle  -> draw a rectangle with top-left corner specified by the first click and the bottom-right corner specified ");
        output(115, 415, " by a second click. If the second click is bottom-left, top-right or top-left comparing to the first click, a warning will show ");
        output(115, 390, " in the console.");
        output(100, 365, "8: Circle  -> draw a circle centered at the position of the first click, with its radius set by a second click.");
        output(100, 340, "9: Airbrush  -> draw multiple points as brush around the clicked point.");
        output(90, 315, "10: Menu \"Eraser\"  -> erase the points by clicking and dragging.");
        output(90, 290, "11: Menu \"Undo\"  -> undo, the history can keep maximum 20 records.");
        output(90, 265, "12: Menu \"Redo\"  ->  redo, the history can keep maximum 20 records. ");
        output(90, 240, "13: Menu \"Clear\"  -> clear all the points and clear the history.  ");
        output(90, 215, "14: Menu \"Quit\"  -> close the window.");
        output(90, 190, "15: Keyboard 'q'  -> close the window.");
        output(90, 165, "16: Keyboard 'esc'  -> close the window. ");
        output(90, 140, "17: Keyboard 'c'    -> clear all the points and clear the history. ");
        output(90, 115, "18: Keyboard '+'    -> larger size of eraser or brush.");
        output(90, 90, "19: Keyboard '-'    -> smaller size of eraser or brush.");
        output(90, 65, "20: Keyboard 'u'    -> undo, the history can keep maximum 20 records.");
        output(90, 40, "21: Keyboard 'r'    -> redo, the history can keep maximum 20 records.");
        glutSwapBuffers();
        glPopMatrix();
    }

    else if (value == 24)
    {
        glPushMatrix();
        glColor3d(0.0,1.0,0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        output(475, 680, "A GUIDE TO PAINT TOOL!!");
        output(100, 650, "List of Commands:");
        output(100, 615, "1: Right click -> show menu ");
        output(100, 590, "2: Left click  -> choose option ");
        output(100, 565, "3: Menu \"Color\"  -> You can choose Red, Green, Blue, Yellow or Random, the default color is Red.");
        output(100, 540, "4: Menu \"Shapes\"  -> The default shape is Point.");
        output(100, 515, "5: Point  -> draw a dot at the point clicked with the mouse. Clicking and dragging will draw points constantly like free-form ");
        output(115, 490, " drawing.");
        output(100, 465, "6: Line  -> Draw a linebetween two subseauently clicked points.(Bresenham's algorithm is used here.");
        output(100, 440, "7: Rectangle  -> draw a rectangle with top-left corner specified by the first click and the bottom-right corner specified ");
        output(115, 415, " by a second click. If the second click is bottom-left, top-right or top-left comparing to the first click, a warning will show ");
        output(115, 390, " in the console.");
        output(100, 365, "8: Circle  -> draw a circle centered at the position of the first click, with its radius set by a second click.");
        output(100, 340, "9: Airbrush  -> draw multiple points as brush around the clicked point.");
        output(90, 315, "10: Menu \"Eraser\"  -> erase the points by clicking and dragging.");
        output(90, 290, "11: Menu \"Undo\"  -> undo, the history can keep maximum 20 records.");
        output(90, 265, "12: Menu \"Redo\"  ->  redo, the history can keep maximum 20 records. ");
        output(90, 240, "13: Menu \"Clear\"  -> clear all the points and clear the history.  ");
        output(90, 215, "14: Menu \"Quit\"  -> close the window.");
        output(90, 190, "15: Keyboard 'q'  -> close the window.");
        output(90, 165, "16: Keyboard 'esc'  -> close the window. ");
        output(90, 140, "17: Keyboard 'c'    -> clear all the points and clear the history. ");
        output(90, 115, "18: Keyboard '+'    -> larger size of eraser or brush.");
        output(90, 90, "19: Keyboard '-'    -> smaller size of eraser or brush.");
        output(90, 65, "20: Keyboard 'u'    -> undo, the history can keep maximum 20 records.");
        output(90, 40, "21: Keyboard 'r'    -> redo, the history can keep maximum 20 records.");
        glutSwapBuffers();
        glPopMatrix();
    }

    else if (value == 23)
    {
        glPushMatrix();
        glColor3d(0.0,0.0,1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        output(475, 680, "A GUIDE TO PAINT TOOL!!");
        output(100, 650, "List of Commands:");
        output(100, 615, "1: Right click -> show menu ");
        output(100, 590, "2: Left click  -> choose option ");
        output(100, 565, "3: Menu \"Color\"  -> You can choose Red, Green, Blue, Yellow or Random, the default color is Red.");
        output(100, 540, "4: Menu \"Shapes\"  -> The default shape is Point.");
        output(100, 515, "5: Point  -> draw a dot at the point clicked with the mouse. Clicking and dragging will draw points constantly like free-form ");
        output(115, 490, " drawing.");
        output(100, 465, "6: Line  -> Draw a linebetween two subseauently clicked points.(Bresenham's algorithm is used here.");
        output(100, 440, "7: Rectangle  -> draw a rectangle with top-left corner specified by the first click and the bottom-right corner specified ");
        output(115, 415, " by a second click. If the second click is bottom-left, top-right or top-left comparing to the first click, a warning will show ");
        output(115, 390, " in the console.");
        output(100, 365, "8: Circle  -> draw a circle centered at the position of the first click, with its radius set by a second click.");
        output(100, 340, "9: Airbrush  -> draw multiple points as brush around the clicked point.");
        output(90, 315, "10: Menu \"Eraser\"  -> erase the points by clicking and dragging.");
        output(90, 290, "11: Menu \"Undo\"  -> undo, the history can keep maximum 20 records.");
        output(90, 265, "12: Menu \"Redo\"  ->  redo, the history can keep maximum 20 records. ");
        output(90, 240, "13: Menu \"Clear\"  -> clear all the points and clear the history.  ");
        output(90, 215, "14: Menu \"Quit\"  -> close the window.");
        output(90, 190, "15: Keyboard 'q'  -> close the window.");
        output(90, 165, "16: Keyboard 'esc'  -> close the window. ");
        output(90, 140, "17: Keyboard 'c'    -> clear all the points and clear the history. ");
        output(90, 115, "18: Keyboard '+'    -> larger size of eraser or brush.");
        output(90, 90, "19: Keyboard '-'    -> smaller size of eraser or brush.");
        output(90, 65, "20: Keyboard 'u'    -> undo, the history can keep maximum 20 records.");
        output(90, 40, "21: Keyboard 'r'    -> redo, the history can keep maximum 20 records.");
        glutSwapBuffers();
        glPopMatrix();
    }

    else if(value == 30)
    {
    glutDestroyWindow(window_ID);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Paint");
	callbackInit();
	init();
	printGuide();
	createOurMenu();
	glutMainLoop();
    }
glFlush();
}

int main(int argc, char **argv)
{

    int i,color_submenu;

    glutInit(&argc, argv);
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-mono")) {
            font = GLUT_BITMAP_9_BY_15;
        }
    }
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1500, 900);
    window_ID = glutCreateWindow("Drawing Tool");
    //glutFullScreen();
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glColor3f(0.0,0.0,0.0);
    glutDisplayFunc(displayf);
    glutReshapeFunc(reshape);
    createMenu();
    glutMainLoop();


	return (0);

}
