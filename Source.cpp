#include <iostream>
#include <algorithm> 
#ifdef WIN32
#include <windows.h>
#endif
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

#define _CRT_SECURE_NO_WARNINGS 1 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1 
#pragma once

#define X .525731112119133606
#define Z .850650808352039932

int WINDOW_WIDTH = 600;
int WINDOW_HEIGHT = 600;


bool mPressed = true;

void trianglesShading();


GLfloat light_position[] = { 0.0, 0.0, 5.0, 1.0 };
GLfloat angle = 100;
GLfloat light_direction[] = { .0, .0, -1.0 };
GLfloat light_x = .0f;
GLfloat light_y = .0f;
GLfloat light_z = 5.0f;
int phong = 0;
bool shearCondition = false;
/*material data*/
GLfloat material_color1[] = { 0.1, 0.7, .0, 1.0 };
GLfloat material_color2[] = { 1.0, 0.0, .0, 1.0 };
GLfloat material_color3[] = { .0, 1.0, 0.0, 1.0 };
GLfloat material_ambient[] = { .5, .5, .5, 1.0 };
GLfloat low_sh[] = { 5.0 };
GLfloat material_specular[] = { .2, .2, .2, 1. };

float shx = 0;
float shy = 0;

float theta_x = 0;
float theta_y = 0;
float theta_z = 0;

/* Define data */
int mouse_down = 0; //Represent the left mouse key is clicked down
int current_x = 0, current_y = 0;
static GLdouble ex = 0.0, ey = 0.0, ez = 5.0, upx = 0.0, upy = 1.0, upz = 0.0, ox = 0.0, oy = 0.0, oz = 0.0;

/*Projection matrix mode*/
int projection = 0; //0 - parallel; 1 - perspective

int shadingParam = 0;

float poi = 50;

using namespace std;


struct Color {
	GLfloat channel[3];
};

Color colorAmbience;
Color colorTeddy;
Color colorPhong;


struct Vertex {
	float x;
	float y;
	float z;
	Color c;


	void normalize2()
	{
		float len = x * x + y * y + z * z;
		x = x / sqrt(len);
		y = y / sqrt(len);
		z = z / sqrt(len);
	}

	const Vertex operator * (const Vertex& vertex) const
	{
		Vertex result;
		result.x = (y * vertex.z) - (z * vertex.y);
		result.y = (z * vertex.x) - (x * vertex.z);
		result.z = (x * vertex.y) - (y * vertex.x);

		return result;
	}


	/*Reload - operator*/
	const Vertex operator - (const Vertex& right) const
	{
		Vertex result;
		/*implement here*/
		result.x = x - right.x;
		result.y = y - right.y;
		result.z = z - right.z;


		return result;
	}




	/*The dot-product */
	const float dot(const Vertex& right) const
	{
		float result;
		/*implement here*/
		result = (x*right.x) + (y*right.y) + (z*right.z);


		return result;
	}


};


struct CustomVector {
	float element[4];
};


struct Triangle {
	int v1;
	int v2;
	int v3;
	Color color[3];
};


std::vector<Vertex> vt_list;
std::vector<Triangle> tg_list;

Vertex cameraPosVer;
Vertex lightVer;
/*for normal vector compute*/
Vertex calcNorm(int a, int b, int c)
{

	Vertex normal = (vt_list[b] - vt_list[a])*(vt_list[c] - vt_list[a]);
	normal.normalize2();
	return normal;
}

/*for light vector compute*/
Vertex calcLightVec(int a)
{
	Vertex l = lightVer - vt_list[a];
	l.normalize2();
	return l;
}

/* reflected vector*/
Vertex reflectVec(Vertex n, Vertex l)
{
	float a = 2 * n.dot(l);

	Vertex temp;
	temp.x = a * n.x;
	temp.y = a * n.y;
	temp.z = a * n.z;

	return (temp - l);
}


Vertex calcView(float a)
{
	Vertex z;

	z.x = cameraPosVer.x - vt_list[a].x;
	z.y = cameraPosVer.y - vt_list[a].y;
	z.z = cameraPosVer.z - vt_list[a].z;
	z.normalize2();
	return z;
}


struct Mat {
	float element[4][4];


	void identityMat()
	{
		for (int i = 0; i <= 3; i++)
		{
			for (int j = 0; j <= 3; j++)
			{
				if (i == j) {
					element[i][j] = 1;
				}
				else
				{
					element[i][j] = 0;
				}
			}
		}
	}


	const Mat operator * (const Mat& right) const
	{
		Mat result;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				result.element[i][j] = 0;
				for (int k = 0; k < 4; k++)
				{
					result.element[i][j] += element[i][k] * right.element[k][j];
				}
			}
		}

		return result;
	}


	const CustomVector operator * (const CustomVector& vec) const
	{
		CustomVector result;

		for (int i = 0; i < 4; i++)
		{

			result.element[i] = 0;
			for (int k = 0; k < 4; k++)
			{
				result.element[i] += element[i][k] * vec.element[k];
			}

		}

		return result;
	}
};





void readAndFillArray() {
	float x = 0;
	float y = 0;
	float z = 0;

	int indx_a = 0;
	int indx_b = 0;
	int indx_c = 0;

	FILE * file = fopen("teddy.obj", "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
	}
	while (1) {
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF) {
			break;
		}

		else if (strcmp(lineHeader, "v") == 0) {
			fscanf(file, "%f %f %f\n", &x, &y, &z);
			Vertex v;
			v.x = x;
			v.y = y;
			v.z = z;
			for (int c = 0; c < 3; c++)
			{
				v.c.channel[c] = 1;
			}
			vt_list.push_back(v);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			fscanf(file, "%d %d %d\n", &indx_a, &indx_b, &indx_c);


			Triangle triangle;
			triangle.v1 = indx_a;
			triangle.v2 = indx_b;
			triangle.v3 = indx_c;

			triangle.color[0] = vt_list[indx_a - 1].c;
			triangle.color[1] = vt_list[indx_b - 1].c;
			triangle.color[2] = vt_list[indx_c - 1].c;
			tg_list.push_back(triangle);

		}
	}







}

void initialize()
{
	/* Use depth buffering for hidden surface elimination. */
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, (GLint)WINDOW_WIDTH, (GLint)WINDOW_HEIGHT);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, &angle);

	cameraPosVer.x = 11111110;
	cameraPosVer.y = 0;  //11111110;
	cameraPosVer.z = 0;


	lightVer.x = 0;//right and left
	lightVer.y = 0;//up and down
	lightVer.z = 10;//front and behind


	lightVer.c.channel[0] = 0.8;//0.0
	lightVer.c.channel[1] = 0.8;//0.0
	lightVer.c.channel[2] = 0.8;//0.0


	colorAmbience.channel[0] = 1;
	colorAmbience.channel[1] = 1;
	colorAmbience.channel[2] = 1;

	colorTeddy.channel[0] = 0.9;
	colorTeddy.channel[1] = 0.7;
	colorTeddy.channel[2] = 0.1;



	colorPhong.channel[0] = 0.1;//.8
	colorPhong.channel[1] = 1;
	colorPhong.channel[2] = 0;





	lightVer.x = 200;//right and left
	lightVer.y = 200;//up and down
	lightVer.z = 200;//front and behind

colorTeddy.channel[0] = 0;
colorTeddy.channel[1] = .5;
colorTeddy.channel[2] = 0;

colorAmbience.channel[0] = 0;
colorAmbience.channel[1] = 1;
colorAmbience.channel[2] = 0;

colorPhong.channel[0] = 50;
colorPhong.channel[1] = 50;
colorPhong.channel[2] = 50;





}




void rotateCalc()
{
	// Rotation matrix
	Mat Rx, Ry, Rz;
	Mat T;

	Rx.identityMat();
	Ry.identityMat();
	Rz.identityMat();
	T.identityMat();

	// x axis 
	Rx.element[1][1] = cos(theta_x);
	Rx.element[1][2] = -sin(theta_x);
	Rx.element[2][1] = sin(theta_x);
	Rx.element[2][2] = cos(theta_x);


	// y axis 
	Ry.element[0][0] = cos(theta_y);
	Ry.element[0][2] = sin(theta_y);
	Ry.element[2][0] = -sin(theta_y);
	Ry.element[2][2] = cos(theta_y);



	/*

	modify this when z press
	// z axis
	Rz.element[0][0] = cos(theta_z);
	Rz.element[0][1] = -sin(theta_z);
	Rz.element[1][0] = sin(theta_z);
	Rz.element[1][1] = cos(theta_z);


	mouse motion ----- theta_z += static_cast<float>(y - current_y) / 100.f;
	at the end theta_z = 0;
	*/



	Mat P = T * Rx * Ry;


	for (int i = 0; i < vt_list.size(); i++)
	{
		CustomVector cur_pt, updated_pt;
		cur_pt.element[0] = vt_list[i].x;
		cur_pt.element[1] = vt_list[i].y;
		cur_pt.element[2] = vt_list[i].z;
		cur_pt.element[3] = 1.0f;
		updated_pt = P * cur_pt;
		vt_list[i].x = updated_pt.element[0] / updated_pt.element[3];
		vt_list[i].y = updated_pt.element[1] / updated_pt.element[3];
		vt_list[i].z = updated_pt.element[2] / updated_pt.element[3];
	}


	theta_x = 0;
	theta_y = 0;
}





void onDisplay()
{

	//for each iteration, clear the canvas
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	/*glMatrixMode(GL_PROJECTION);*/
	glMatrixMode(GL_PROJECTION); //define the camera matrix model
								 //
	glLoadIdentity(); //generate a matrix


	if (projection == 0) //parallel projection
	{

		glOrtho(-40, 40, -40, 40, -100, 100);
		//glOrtho(-3, 3, -1, 1, 0, 100);

	}
	else  //perspective
		gluPerspective(60.0, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, .1, 100.0);


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(ex, ey, ez, ox, oy, oz, upx, upy, upz);

	if (mPressed) {
		glBegin(GL_LINE_LOOP);
	}
	else {
		glBegin(GL_TRIANGLES);
	}



	if (mouse_down == 1) {
		rotateCalc();
	}


	glColor3f(1, 1, 0);
	trianglesShading();



	for (int arraysize = 0; arraysize < tg_list.size(); arraysize++) {

		glColor3f(tg_list[arraysize].color[2].channel[0], tg_list[arraysize].color[2].channel[1], tg_list[arraysize].color[2].channel[2]);
		glVertex3d(vt_list[tg_list[arraysize].v1 - 1].x + (vt_list[tg_list[arraysize].v1 - 1].y*shx), vt_list[tg_list[arraysize].v1 - 1].y + (vt_list[tg_list[arraysize].v1 - 1].x*shy), vt_list[tg_list[arraysize].v1 - 1].z);


		glColor3f(tg_list[arraysize].color[1].channel[0], tg_list[arraysize].color[1].channel[1], tg_list[arraysize].color[1].channel[2]);
		glVertex3d(vt_list[tg_list[arraysize].v2 - 1].x + (vt_list[tg_list[arraysize].v2 - 1].y*shx), vt_list[tg_list[arraysize].v2 - 1].y + (vt_list[tg_list[arraysize].v2 - 1].x*shy), vt_list[tg_list[arraysize].v2 - 1].z);


		glColor3f(tg_list[arraysize].color[2].channel[0], tg_list[arraysize].color[2].channel[1], tg_list[arraysize].color[2].channel[2]);
		glVertex3d(vt_list[tg_list[arraysize].v3 - 1].x + (vt_list[tg_list[arraysize].v3 - 1].y*shx), vt_list[tg_list[arraysize].v3 - 1].y + (vt_list[tg_list[arraysize].v3 - 1].x*shy), vt_list[tg_list[arraysize].v3 - 1].z);

	}


	glEnd();

	glutSwapBuffers();




}

void onMouse(int button, int state, int x, int y)
{

	GLint specialKey = glutGetModifiers();
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			mouse_down = 1;
			current_x = x;
			current_y = y;
			if (specialKey == GLUT_ACTIVE_SHIFT)
			{
				shearCondition = true;
			}
			else
			{
				shearCondition = false;

			}

		}
		else if (state == GLUT_UP)
		{
			mouse_down = 0;
		}
		break;

	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)

			break;

	default:
		break;

	}

}


void onResize(GLint w, GLint h)
{
	//glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective (65.0, (GLfloat) w/(GLfloat) h, 1.0, 100.0);

}


void onIdle()
{

}


void onMouseMotion(int x, int y)
{

	if (mouse_down == 1 && shearCondition == true)
	{



		shx += static_cast<float>(x - current_x) / 100.f;
		shy += static_cast<float>(y - current_y) / 100.f;

		ex += static_cast<float>(x - current_x) / 100.f;
		ey += static_cast<float>(y - current_y) / 100.f;


		current_x = x;
		current_y = y;
	}

	else if (mouse_down == 1)
	{


		//	ex += static_cast<float>(x - current_x) / 100.f;
		//	ey += static_cast<float>(y - current_y) / 100.f;


		theta_y += static_cast<float>(x - current_x) / 100.f;
		theta_x += static_cast<float>(y - current_y) / 100.f;







		current_x = x;
		current_y = y;
	}

	glutPostRedisplay();
}



/* ambient color */
float ambient_calculation(float a, float b)
{
	return a * b;
}

/* diffuse color */
float diffuse_calculation(float a, float b, Vertex n, Vertex l)
{
	float diffuse = a * b*fmax(0, n.dot(l));
	return diffuse;
}

/* phong color */
float phong_calculation(float a, float b, Vertex r, Vertex v, float p)
{
	float phong = a * b*pow(fmax(0, r.dot(v)), p);
	return phong;
}



void trianglesShading()
{
	if (shadingParam == 0)
	{
		/*Make all the triangles the same color*/
		for (int i = 0; i < tg_list.size(); i++) //triangle index
		{
			for (int j = 0; j < 3; j++) //vertex index
			{
				for (int k = 0; k < 3; k++) //color channel index
				{
					tg_list[i].color[j].channel[k] = colorTeddy.channel[k];

				}
			}
		}
	}
	else if (shadingParam == 1) //flat shading without phong term
	{
		for (int i = 0; i < tg_list.size(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					Vertex n = calcNorm(tg_list[i].v1 - 1, tg_list[i].v2 - 1, tg_list[i].v3 - 1);
					Vertex l = calcLightVec(tg_list[i].v1 - 1);
					tg_list[i].color[j].channel[k] = ambient_calculation(colorAmbience.channel[k], colorTeddy.channel[k]) + diffuse_calculation(lightVer.c.channel[k], colorTeddy.channel[k], n, l);

				}
			}
		}
	}
	else if (shadingParam == 2) //flat shading with phong term
	{
		for (int i = 0; i < tg_list.size(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					Vertex n = calcNorm(tg_list[i].v1 - 1, tg_list[i].v2 - 1, tg_list[i].v3 - 1);
					Vertex l = calcLightVec(tg_list[i].v1 - 1);
					Vertex r = reflectVec(n, l);
					Vertex v = calcView(tg_list[i].v1 - 1);
					tg_list[i].color[j].channel[k] = ambient_calculation(colorAmbience.channel[k], colorTeddy.channel[k]) + diffuse_calculation(lightVer.c.channel[k], colorTeddy.channel[k], n, l) + phong_calculation(colorTeddy.channel[k], colorPhong.channel[k], r, v, poi);

				}
			}
		}
	}
}


void onKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(1);

		break;
	case '0':
		mPressed = false;
		shadingParam = 0;
		glutPostRedisplay();
		break;

	case '1':
		mPressed = false;
		shadingParam = 1;
		glutPostRedisplay();
		break;

	case '2':
		mPressed = false;
		shadingParam = 2;
		glutPostRedisplay();
		break;
	case 'p':
		projection = (projection == 0) ? 1 : 0;
		glutPostRedisplay();
		break;
	case 'z':
		ez += 0.2;
		glutPostRedisplay();
		break;
	case 'x':
		ez -= 0.2;
		glutPostRedisplay();
		break;
		/*case 's':
			shading = (shading == 0) ? 1 : 0;
			glutPostRedisplay();
			break;*/

	case 'o':
		phong = (phong == 0) ? 1 : 0;
		glutPostRedisplay();
		break;
	case 'm':
		if (mPressed) {
			mPressed = false;
		}
		else {
			mPressed = true;
		}
		glutPostRedisplay();
		break;
	case 'M':
		if (mPressed) {
			mPressed = false;
		}
		else {
			mPressed = true;
		}
		glutPostRedisplay();
		break;
		break;
	default:
		break;
	}
}




void processSpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case 27:
		break;
	case 100:
		light_x -= 1.2;
		glutPostRedisplay();

		break;
	case 102:
		light_x += 1.2;
		glutPostRedisplay();
		break;
	case 101:
		light_y += 1.2;
		glutPostRedisplay();
		break;
	case 103:
		light_y -= 1.2;
		glutPostRedisplay();
		break;
	}

}


int main(int argc, char** argv)
{
	//Initialization functions
	readAndFillArray();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(0, WINDOW_HEIGHT / 2);
	glutCreateWindow("Draw Teddy");

	initialize(); //optional

	//Call-back functions
	glutDisplayFunc(onDisplay);
	glutKeyboardFunc(onKeyboard);  //waitKey(500)
	glutSpecialFunc(processSpecialKeys);
	glutMouseFunc(onMouse); //mouse click
	glutMotionFunc(onMouseMotion); //mouse movement
	glutReshapeFunc(onResize);
	glutIdleFunc(onIdle); //run something in the backgrond


	//Infinite Loop
	glutMainLoop(); //== while(1)
	return 1;
}