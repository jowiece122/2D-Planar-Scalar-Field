#include "GL/freeglut.h"
#include "GLFW/glfw3.h"
#include <stdio.h>
#include <map>
#include <iostream>
#include <list>
#include <vector>
#include <array>
#include <math.h>

struct mypoints {
	double x1;
	double y1;
	double x2;
	double y2;
	int group;
};
struct myLine {
	//L(x,y) = (ux+vy+w)/h = 0
	double u; //y1-y2
	double v; //x2-x1
	double w; //x1y2 - x2y1
	double h; //sqrt(u^2+v^2);
};
struct singlePoint {
	double x;
	double y;
	int color;
};
std::list<mypoints> curve;
std::list<singlePoint> field;

int width = 800;
int height = 800;
int clickTimes = 0;
double oX[50];
double oY[50];
int mode = -1;
int lineFound = -1;

std::list<mypoints> pointlist;
double foundX;
double foundY;
double xOnLine;
double yOnLine;
singlePoint selector;
int selected = -1;
double s = -1;
double n = 30;
std::list<myLine> lineA;
std::list<myLine> lineB;

double getSlope(mypoints points) {
	double dx = points.x2 - points.x1;
	double dy = points.y2 - points.y1;
	double slope = dy / dx;
	return slope;
}
double getIntercept(mypoints points) {
	double slope = getSlope(points);
	double intercept = points.y1 - slope * points.x1;
	return intercept;
}

void drawLine(mypoints points) {
	double slope = getSlope(points);
	double intercept = getIntercept(points);
	if (points.group == 1) {
		glColor3f(1.0, 0.0, 1.0);
	}
	else if (points.group == 2) {
		glColor3f(1.0, 1.0, 0.0);
	}
	else { glColor3f(0.0, 1.0, 1.0); }
	glLineWidth(2);
	glBegin(GL_LINE_STRIP);
	for (double t = -800; t < 800; t += 1) {
		glVertex2d(t, slope * t + intercept);
	}
	glEnd();
}

void drawCurve(mypoints points) {
	glColor3f(0.0, 1.0, 1.0); glLineWidth(2);
	glLineWidth(2);
	glBegin(GL_LINE_STRIP);
	glVertex2d(points.x1, points.y1);
	glVertex2d(points.x2, points.y2);
	glEnd();
}

void drawPoint(double x, double y, int color) {
	if (color == -1 || color == 1) {
		glPointSize(1);
	}
	else {
		glPointSize(5);
	}
	glBegin(GL_POINTS);
	if (color == -1) {
		glColor3f(1.0f, 0.0f, 0.0f);
	}
	else if (color == 1) {
		glColor3f(1.0f, 1.0f, 0.0f);
	}
	else { glColor3f(0.3f, 0.5f, 0.0f); }
	glVertex2d(x, y);
	glEnd();
}

int findline(double x, double y) {
	for (std::list<mypoints>::iterator it = pointlist.begin(); it != pointlist.end(); ++it) {
		double intercept = getIntercept(*it);
		double slope = getSlope(*it);
		double tempy = slope * x + intercept;
		if (y<tempy + 0.05 && y > tempy - 0.05) {
			xOnLine = x;
			yOnLine = y;
			foundX = it->x1;
			foundY = it->y1;
			return 1;
		}
	}
	return 0;
}
void refresh() {
	glPointSize(10);
	glBegin(GL_POINTS);
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex2d(0, 0);
	glEnd();
}

myLine normalizeLine(mypoints point) {
	//L(x,y) = (ux+vy+w)/h = 0
	double u = point.y1 - point.y2; //y1-y2
	double v = point.x2 - point.x1; //x2-x1
	double w = point.x1 * point.y2 - point.x2 * point.y1;//x1y2 - x2y1
	double h = sqrt(u * u + v * v); //sqrt(u^2+v^2);
	myLine newline = { u,v,w,h };
	return newline;
}

void findS() {
	if (selected == 1) {
		double A = 0;
		double B = 0;
		double qx = selector.x;
		double qy = selector.y;
		//L(x,y) = (ux+vy+w)/h = 0
		for (std::list<mypoints>::iterator it = pointlist.begin(); it != pointlist.end(); ++it) {
			if (it->group == 1) {
				myLine newLine = normalizeLine(*it);
				lineA.push_back(newLine);
				if (A == 0) {
					A = (newLine.u * qx + newLine.v * qy + newLine.w) / newLine.h;
				}
				else {
					A = A * ((newLine.u * qx + newLine.v * qy + newLine.w) / newLine.h);
				}
			}
			else if (it->group == 2) {
				myLine newLine = normalizeLine(*it);
				lineB.push_back(newLine);
				if (B == 0) {
					B = (newLine.u * qx + newLine.v * qy + newLine.w) / newLine.h;
				}
				else {
					B = B * ((newLine.u * qx + newLine.v * qy + newLine.w) / newLine.h);
				}
			}
		}

		s = A / (A - B);
		//F(x,y) = (1-s)A+s(B)
	}
}

double findF(double x, double y) {
	double A = 0;
	double B = 0;
	for (std::list<myLine>::iterator it = lineA.begin(); it != lineA.end(); ++it) {
		if (A == 0) {
			A = (it->u * x + it->v * y + it->w) / it->h;
		}
		else {
			A = A * (it->u * x + it->v * y + it->w) / it->h;
		}
	}
	for (std::list<myLine>::iterator it = lineB.begin(); it != lineB.end(); ++it) {
		if (B == 0) {
			B = (it->u * x + it->v * y + it->w) / it->h;
		}
		else {
			B = B * (it->u * x + it->v * y + it->w) / it->h;
		}
	}
	double F = ((1 - s) * A) + s * B;
	return F;
}
void evaluate() {
	for (double x = -1.0; x <= 1.0; x += 1 / n) {
		for (double y = -1.0; y <= 1.0; y += 1 / n) {
			double unit = 1 / n;
			double x1 = x;
			double x2 = x + 1 / n;
			double x3 = x + 1 / n;
			double y1 = y + 1 / n;
			double y2 = y + 1 / n;
			double y3 = y;

			double r0 = findF(x, y);
			double r1 = findF(x1, y1);
			double r2 = findF(x2, y2);
			double r3 = findF(x3, y3);

			double p1_x = -3.0;
			double p1_y = -3.0;
			double p2_x = -3.0;
			double p2_y = -3.0;
			double p3_x = -3.0;
			double p3_y = -3.0;
			double p4_x = -3.0;
			double p4_y = -3.0;

			double temp1;
			double temp2;
			double temp3;
			double temp4;

			//1 + - + +
			if (r0 > 0 && r1 < 0 && r2 > 0 && r3 > 0) {
				p1_x = x;
				p2_y = y2;

				temp1 = r0 / (r0 + fabs(r1));
				temp2 = r2 / (r2 + fabs(r1));

				p1_y = y + (temp1 * unit);
				p2_x = x2 - (temp2 * unit);
			}
			// 2 + + - +
			else if (r0 > 0 && r1 > 0&& r2 < 0  && r3 > 0) {
				p1_y = y1;
				p2_x = x3;

				temp1 = r1 / (r1 + fabs(r2));
				temp2 = r3 / (r3 + fabs(r2));

				p1_x = x1 + (temp1 * unit);
				p2_y = y3 + (temp2 * unit);
			}
			// 3 + + + -
			else if (r0 > 0 && r1 > 0 && r2 > 0 && r3 < 0 ) {
				p1_y = y;
				p2_x = x3;

				temp1 = r0 / (r0 + fabs(r3));
				temp2 = r2 / (r2 + fabs(r3));

				p1_x = x + (temp1 * unit);
				p2_y = y2 - (temp2 * unit);
			}
			// 4 + - - +
			else if (r0 > 0 && r1 < 0 && r2 < 0 && r3 > 0) {
				p1_x = x;
				p2_x = x3;

				temp1 = r0 / (r0 + fabs(r1));
				temp2 = r3 / (r3 + fabs(r2));

				p1_y = y + temp1 * unit;
				p2_y = y3 + temp2 * unit;
			}
			// 5 + + - -
			else if (r0 > 0 && r1 > 0 && r2 < 0 && r3 < 0) {
				p1_y = y;
				p2_y = y1;

				temp1 = r0 / (r0 + fabs(r3));
				temp2 = r1 / (r1 + fabs(r2));

				p1_x = x + temp1 * unit;
				p2_x = x1 + temp2 * unit;
			}
			// 6 + - - -
			else if (r0 > 0 && r1 < 0 && r2 < 0 && r3 < 0) {
				p1_y = y;
				p2_x = x;

				temp1 = r0 / (fabs(r3) + r0);
				temp2 = r0 / (fabs(r1) + r0);

				p1_x = x + temp1 * unit;
				p2_y = y + temp2 * unit;
			}
			// 7 - + + +
			else if (r0 < 0 && r1 > 0 && r2 > 0 && r3 > 0) {
				p1_x = x;
				p2_y = y;

				temp1 = r1 / (fabs(r0) + r1);
				temp2 = r3 / (fabs(r0) + r3);

				p1_y = y1 - (temp1 * unit);
				p2_x = x3 - (temp2 * unit);
			}
			// 8- - + +
			else if (r0 < 0 && r1 < 0 && r2 > 0 && r3 > 0) {
				p1_y = y1;
				p2_y = y;

				temp1 = r2 / (fabs(r1) + r2);
				temp2 = r3 / (fabs(r0) + r3);

				p1_x = x2 - (temp1 * unit);
				p2_x = x3 - (temp2 * unit);
			}
			// 9 - + + -
			else if (r0 < 0 && r1 > 0 && r2 > 0 && r3 < 0) {
				p1_x = x1;
				p2_x = x2;

				temp1 = r1 / (fabs(r0) + r1);
				temp2 = r2 / (fabs(r3) + r2);

				p1_y = y1 - (temp1 * unit);
				p2_y = y2 - (temp2 * unit);
			}
			// 10 - - - +
			else if (r0 < 0 && r1 < 0 && r2 < 0 && r3 > 0) {
				p1_y = y;
				p2_x = x3;

				temp1 = r3 / (fabs(r0) + r3);
				temp2 = r3 / (fabs(r2) + r3);

				p1_x = x3 - (temp1 * unit);
				p2_y = y3 + (temp2 * unit);

			}
			// 11 - + - -
			else if (r0 < 0 && r1 > 0 && r2 < 0 && r3 < 0) {
			p1_x = x1;
			p2_y = y1;

			temp1 = r1 / (fabs(r0) + r1);
			temp2 = r1 / (fabs(r2) + r1);

			p1_y = y1 - (temp1 * unit);
			p2_x = x1 + (temp2 * unit);

			}
			// 12 - - + -
			else if (r0 < 0 && r1 < 0 && r2 > 0 && r3 < 0) {
				p1_y = y1;
				p2_x = x3;

				temp1 = r2 / (fabs(r1) + r2);
				temp2 = r2 / (fabs(r3) + r2);

				p1_x = x2 - (temp1 * unit);
				p2_y = y2 - (temp2 * unit);
			}
			// 13 + - + -
			else if (r0 > 0 && r1 < 0 && r2 > 0 && r3 < 0) {

				p1_x = x;
				p4_x = x2;
				p3_y = y2;
				p2_y = y;

				temp1 = r0 / (fabs(r1) + r0);
				temp2 = r2 / (fabs(r3) + r2);
				temp3 = r2 / (fabs(r1) + r2);
				temp4 = r0 / (fabs(r3) + r0);

				p1_y = y + (temp1 * unit);
				p4_y = y2 - (temp2 * unit);
				p3_x = x2 - (temp3 * unit);
				p2_x = x + (temp4 * unit);

				mypoints newseg = { p3_x,p3_y,p4_x,p4_y,3 };
				curve.push_back(newseg);
			}
			// 14 - + - +
			else if (r0 < 0 && r1 > 0 && r2 < 0 && r3 > 0) {

				p1_x = x1;
				p2_y = y1;
				p3_y = y3;
				p4_x = x3;

				temp1 = r1 / (fabs(r0) + r1);
				temp2 = r1 / (fabs(r2) + r1);
				temp3 = r3 / (fabs(r0) + r3);
				temp4 = r3 / (fabs(r2) + r3);

				p1_y = y1 - (temp1 * unit);
				p2_x = x1 + (temp2 * unit);
				p3_x = x3 - (temp3 * unit);
				p4_y = y3 + (temp4 * unit);
				mypoints newseg = { p3_x,p3_y,p4_x,p4_y,3 };
				curve.push_back(newseg);
			}
			if (p1_x != -3.0 && p2_x != -3.0 && p1_y != -3.0 && p2_y != -3.0) {
				mypoints newseg = { p1_x,p1_y,p2_x,p2_y,3 };
				curve.push_back(newseg);
			}
		}
	}
}

void drawfield() {
	for (double x = -1; x <= 1; x += 1 / n) {
		for (double y = -1; y <= 1; y += 1 / n) {
			double F = findF(x, y);
			if (F > 0) {
				singlePoint newpoint = { x,y,-1 };
				field.push_back(newpoint);
			}
			else if (F < 0) {
				singlePoint newpoint = { x,y,1 };
				field.push_back(newpoint);
			}
		}
	}
}

void mouse(int button, int state, int x, int y)
{
	double xd = (double)(x - width / 2) / (double)(width / 2);
	double yd = (double)(height / 2 - y) / (double)(height / 2);
	if (mode == 0) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			clickTimes++;
			oX[clickTimes] = xd;
			oY[clickTimes] = yd;
			drawPoint(xd, yd, 0);
			if ((clickTimes % 2) == 0) {
				double x1 = oX[clickTimes - 1];
				double y1 = oY[clickTimes - 1];
				double x2 = oX[clickTimes];
				double y2 = oY[clickTimes];
				mypoints newpoint = { x1,y1,x2,y2 };
				pointlist.push_back(newpoint);
			}
		}
	}
	else if (mode == 1) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			for (std::list<mypoints>::iterator it = pointlist.begin(); it != pointlist.end(); ++it) {
				double x1 = it->x1;
				double x2 = it->x2;
				double y1 = it->y1;
				double y2 = it->y2;

				if ((xd < (x1 + 0.05)) && (xd > (x1 - 0.05)) && (yd < (y1 + 0.05)) && (yd > (y1 - 0.05))) {
					lineFound = 1;
					foundX = x1;
					foundY = y1;
					break;
				}
				else if ((xd < (x2 + 0.05)) && (xd > (x2 - 0.08)) && (yd < (y2 + 0.08)) && (yd > (y2 - 0.05))) {
					lineFound = 1;
					foundX = x2;
					foundY = y2;
					break;
				}
				else if (findline(xd, yd) == 1) {
					lineFound = 2;
					break;
				}
			}
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glLoadIdentity();
			glutSwapBuffers();
		}
		else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && lineFound == 1) {
			std::list<mypoints>::iterator it = pointlist.begin();
			while (it != pointlist.end()) {
				if (it->x1 == foundX && it->y1 == foundY) {
					mypoints newpoint = { xd, yd, it->x2, it->y2,it->group };
					it = pointlist.erase(it);
					pointlist.push_back(newpoint);
					break;
				}
				else if (it->x2 == foundX && it->y2 == foundY) {
					mypoints newpoint = { it->x1, it->y1, xd, yd,it->group };
					it = pointlist.erase(it);
					pointlist.push_back(newpoint);
					break;
				}
				else {
					it++;
				}
			}
			foundX = 0;
			foundY = 0;
			lineFound = -1;
			if (selected == 1) {
				lineA.clear();
				lineB.clear();
				curve.clear();
				findS();
				drawfield();
				evaluate();
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glLoadIdentity();
				refresh();
				glutSwapBuffers();
			}
		}
		else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && lineFound == 2) {
			std::list<mypoints>::iterator it = pointlist.begin();
			while (it != pointlist.end()) {
				if (it->x1 == foundX && it->y1 == foundY) {
					double xmove = xd - xOnLine;
					double ymove = yd - yOnLine;
					mypoints newpoint = { it->x1 + xmove, it->y1 + ymove, it->x2 + xmove, it->y2 + ymove,it->group };
					it = pointlist.erase(it);
					pointlist.push_back(newpoint);
					break;
				}
				else {
					it++;
				}
			}
			xOnLine = 0;
			yOnLine = 0;
			foundX = 0;
			foundY = 0;
			lineFound = -1;
			if (selected == 1) {
				lineA.clear();
				lineB.clear();
				curve.clear();
				findS();
				printf("s:%f\n", s);
				drawfield();
				evaluate();
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glLoadIdentity();
				refresh();
				glutSwapBuffers();
			}
		}
	}
	else if (mode == 2) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && findline(xd, yd) == 1) {
			std::list<mypoints>::iterator it = pointlist.begin();
			while (it != pointlist.end()) {
				if (it->x1 == foundX && it->y1 == foundY) {
					it = pointlist.erase(it);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glLoadIdentity();
					refresh();
					glutSwapBuffers();
				}
				else {
					it++;
				}
			}
			xOnLine = 0;
			yOnLine = 0;
			foundX = 0;
			foundY = 0;
		}
	}
	else if (mode == 6) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && findline(xd, yd) == 1) {
			std::list<mypoints>::iterator it = pointlist.begin();
			while (it != pointlist.end()) {
				if (it->x1 == foundX && it->y1 == foundY) {
					mypoints newpoint = { it->x1 , it->y1 , it->x2 , it->y2 ,1 };
					it = pointlist.erase(it);
					pointlist.push_back(newpoint);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glLoadIdentity();
					refresh();
					glutSwapBuffers();
				}
				else {
					it++;
				}
			}
			xOnLine = 0;
			yOnLine = 0;
			foundX = 0;
			foundY = 0;
		}
	}
	else if (mode == 7) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && findline(xd, yd) == 1) {
			std::list<mypoints>::iterator it = pointlist.begin();
			while (it != pointlist.end()) {
				if (it->x1 == foundX && it->y1 == foundY) {
					mypoints newpoint = { it->x1 , it->y1 , it->x2 , it->y2 ,2 };
					it = pointlist.erase(it);
					pointlist.push_back(newpoint);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glLoadIdentity();
					refresh();
					glutSwapBuffers();
				}
				else {
					it++;
				}
			}
			xOnLine = 0;
			yOnLine = 0;
			foundX = 0;
			foundY = 0;
		}
	}
	else if (mode == 8) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && findline(xd, yd) == 1) {
			std::list<mypoints>::iterator it = pointlist.begin();
			while (it != pointlist.end()) {
				if (it->x1 == foundX && it->y1 == foundY) {
					mypoints newpoint = { it->x1 , it->y1 , it->x2 , it->y2 ,0 };
					it = pointlist.erase(it);
					pointlist.push_back(newpoint);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glLoadIdentity();
					refresh();
					glutSwapBuffers();
				}
				else {
					it++;
				}
			}
			xOnLine = 0;
			yOnLine = 0;
			foundX = 0;
			foundY = 0;
		}
	}
	else if ((mode == 9 || mode == 13) && button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		lineA.clear();
		lineB.clear();

		curve.clear();
		singlePoint newpoint = { xd, yd, 0 };
		selector.x = xd;
		selector.y = yd;
		selected = 1;
		findS();
		printf("s:%f\n", s);
		drawfield();
		evaluate();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		refresh();
		glutSwapBuffers();
	}
	else if (mode == 10 && button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		if (xd<selector.x + 0.05 && xd>selector.x - 0.05 && yd<selector.y + 0.05 && yd>selector.y - 0.05) {
			lineA.clear();
			lineB.clear();
			field.clear();
			curve.clear();
			selected = -1;
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glLoadIdentity();
			refresh();
			glutSwapBuffers();
		}
	}
}

void menu(int num) {  // num: id of the added item
	printf("menu called with arg %d\n", num);
	if (num == 13) {
		std::cout << "Please input your N Value: ";
		std::cin >> n;
	}
}

void sub_defines(int num) {  // num: id of the added item
	printf("menu called with arg %d\n", num);
	switch (num)
	{
	case 0:
		mode = 0;
		break;
	case 1:
		mode = 1;
		break;
	case 2:
		mode = 2;
		break;
	case 3:
		mode = 3;
		break;
	case 4:
		mode = 4;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		pointlist.clear();
		curve.clear();
		lineA.clear();
		lineB.clear();
		field.clear();
		clickTimes = 0;
		selected = -1;
		s = -1;
		n = 20;
		glLoadIdentity();
		printf("reset!\n");
		refresh();
		glutSwapBuffers();
		break;
	case 5:
		exit(0);
		break;
	}
}

void sub_selects(int num) {
	printf("menu called with arg %d\n", num);
	switch (num)
	{
	case 6:
		mode = 6;
		break;
	case 7:
		mode = 7;
		break;
	case 8:
		mode = 8;
		break;
	case 9:
		mode = 9;
		break;
	case 10:
		mode = 10;
		break;
	case 13:
		mode = 13;
		break;
	}
}

void createMenu(void) {

	int sub_menu_defines = glutCreateMenu(sub_defines);
	glutAddMenuEntry("Define Line", 0);     // 
	glutAddMenuEntry("Move Line", 1);  //
	glutAddMenuEntry("Delete Line", 2);
	glutAddMenuEntry("Do Nothing", 3); //
	glutAddMenuEntry("Reset", 4);
	glutAddMenuEntry("Exit", 5);

	int sub_menu_selects = glutCreateMenu(sub_selects);
	glutAddMenuEntry("Select A", 6);
	glutAddMenuEntry("Select B", 7);
	glutAddMenuEntry("Unselect Group", 8);
	glutAddMenuEntry("Define Selectors", 9);
	glutAddMenuEntry("Move Selectors", 13);
	glutAddMenuEntry("Remove Selector by click", 10);


	glutCreateMenu(menu);
	glutAddMenuEntry("Root", 0);
	glutAddSubMenu("Defines", sub_menu_defines);
	glutAddSubMenu("Selects", sub_menu_selects);
	glutAddMenuEntry("Change Density", 14);
	glutAttachMenu(GLUT_RIGHT_BUTTON);  // bind to the event: clicking the right button
}

void display(void) {
	for (std::list<mypoints>::iterator it = pointlist.begin(); it != pointlist.end(); ++it) {
		drawPoint(it->x1, it->y1, 0);
		drawPoint(it->x2, it->y2, 0);
		drawLine(*it);
	}
	for (std::list<mypoints>::iterator it = curve.begin(); it != curve.end(); ++it) {
		drawCurve(*it);
	}
	if (selected == 1) {
		drawPoint(selector.x, selector.y, 0);
	}
	for (std::list<singlePoint>::iterator it = field.begin(); it != field.end(); ++it) {
		drawPoint(it->x, it->y, it->color);
	}
	glutSwapBuffers();
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Scalar Fields");
	createMenu();
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMainLoop();
	return 0;
}