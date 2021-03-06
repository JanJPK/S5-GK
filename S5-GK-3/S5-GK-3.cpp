#include "stdafx.h"
#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <random>
#include <array>
#include <cmath>


// Generator Mersenne Twister.
static std::random_device source;
static std::mt19937 rng(source());
static std::uniform_real_distribution<float> colorRandom(0.0f, 1.0f);

#pragma region Variables
typedef float point3[3];

// Needed for spinning.
static GLfloat theta[] = { 0.0, 0.0, 0.0 };
int spinMode = 1;

#pragma region Egg
const int N = 30;
int renderMode = 1;
bool bowEgg = false;

struct Coordinates2
{
	long double x;
	long double y;
};

struct Coordinates3
{
	long double x;
	long double y;
	long double z;
};

struct Color
{
	float r;
	float g;
	float b;
};

std::array<std::array<Color, N>, N> colors;
#pragma endregion
#pragma endregion

#pragma region Axes
void Axes(void)
{
	point3 x_min = { -5.0, 0.0, 0.0 };
	point3 x_max = { 5.0, 0.0, 0.0 };
	// początek i koniec obrazu osi x

	point3 y_min = { 0.0, -5.0, 0.0 };
	point3 y_max = { 0.0, 5.0, 0.0 };
	// początek i koniec obrazu osi y

	point3 z_min = { 0.0, 0.0, -5.0 };
	point3 z_max = { 0.0, 0.0, 5.0 };
	//  początek i koniec obrazu osi y
	glColor3f(1.0f, 0.0f, 0.0f); // kolor rysowania osi - czerwony
	glBegin(GL_LINES); // rysowanie osi x
	glVertex3fv(x_min);
	glVertex3fv(x_max);
	glEnd();

	glColor3f(0.0f, 1.0f, 0.0f); // kolor rysowania - zielony
	glBegin(GL_LINES); // rysowanie osi y
	glVertex3fv(y_min);
	glVertex3fv(y_max);
	glEnd();

	glColor3f(0.0f, 0.0f, 1.0f); // kolor rysowania - niebieski
	glBegin(GL_LINES); // rysowanie osi z
	glVertex3fv(z_min);
	glVertex3fv(z_max);
	glEnd();
}
#pragma endregion

#pragma region Render
float rC()
{
	// Returns 1 or 0 in float.
	return rand() % 2;
}

void SpinEgg()
{
	switch (spinMode)
	{
	case 1:
	{
		break;
	}
	case 2:
	{
		theta[0] -= 0.5;
		if (theta[0] > 360.0) theta[0] -= 360.0;

		theta[1] -= 0.5;
		if (theta[1] > 360.0) theta[1] -= 360.0;

		theta[2] -= 0.05;
		if (theta[2] > 360.0) theta[2] -= 360.0;
		break;
	}

	case 3:
	{
		theta[0] -= 0.1;
		if (theta[0] > 360.0) theta[0] -= 360.0;

		theta[1] -= 0.2;
		if (theta[1] > 360.0) theta[0] -= 360.0;

		theta[2] -= 0.3;
		if (theta[2] > 360.0) theta[2] -= 360.0;
		break;
	}
	}


	glutPostRedisplay();
}

Coordinates3 PlaneTransform(Coordinates2 coordinates)
{
	long double u = coordinates.x;
	long double v = coordinates.y;
	long double longTransformation = -90 * pow(u, 5.0) + 225 * pow(u, 4.0) - 270 * pow(u, 3.0) + 180 * pow(u, 2.0) - 45 * u;
	// Transformation:
	// x(u, v) = (-90u^5 + 225u^4 - 270u^3 + 180u^2 - 45u)cos(pi*v)
	// y(u, v) = 160u^4 - 320u^3 + 160u^2
	// z(u, v) = (-90u^5 + 225u^4 - 270^3 + 180u^2 - 45u)sin(pi*v)
	//		where:
	//			0 <= u <= 1
	//			0 <= v <= 1

	Coordinates3 transformed = { longTransformation * cos(std::_Pi * v), 160 * pow(u, 4.0) - 320 * pow(u, 3.0) + 160 * pow(u, 2.0),
		longTransformation * sin(std::_Pi * v) };

	return transformed;
}

void FillColorMatrix()
{
	for (int i = 0; i < N - 1; i++)
	{
		for (int j = 0; j < N - 1; j++)
		{
			colors[i][j] = { colorRandom(rng), colorRandom(rng), colorRandom(rng) };
		}
	}
}

void RenderEgg(float x, float y, float z)
{
	// Matrices with coordinates of points.
	std::array<std::array<Coordinates2, N>, N> plane2D;
	std::array<std::array<Coordinates3, N>, N> plane3D;

	// Populating the 2D matrix.
	double castedN = N - 1;
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			plane2D[i][j] = { static_cast<long double>(i / castedN), static_cast<long double>(j / castedN) };
		}
	}

	// Populating the 3D matrix.
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			plane3D[i][j] = PlaneTransform(plane2D[i][j]);
		}
	}

	// Rendering points.
	//glTranslatef(0.0, -5.0f, 0.0); // Moves the egg down so its centered.
	glTranslatef(x, y, z);
	if (bowEgg)
	{
		glRotatef(20.0, 1.0f, 0.0f, 0.0f); // Slight egg rotation.
	}
	switch (renderMode)
	{
		// Points only.
	case 1:
	{
		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_POINTS);
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				glVertex3f(plane3D[i][j].x, plane3D[i][j].y, plane3D[i][j].z);
			}
		}
		glEnd();
		break;
	}
	// Horizontal stripes.
	case 2:
	{
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		for (int i = 0; i < N - 1; i++)
		{
			for (int j = 0; j < N - 1; j++)
			{
				glVertex3f(plane3D[i][j].x, plane3D[i][j].y, plane3D[i][j].z);
				glVertex3f(plane3D[i][j + 1].x, plane3D[i][j + 1].y, plane3D[i][j + 1].z);
			}
		}
		glEnd();
		break;
	}
	// Vertical stripes.
	case 3:
	{
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		for (int i = 0; i < N - 1; i++)
		{
			for (int j = 0; j < N - 1; j++)
			{
				glVertex3f(plane3D[i][j].x, plane3D[i][j].y, plane3D[i][j].z);
				glVertex3f(plane3D[i + 1][j].x, plane3D[i + 1][j].y, plane3D[i + 1][j].z);
			}
		}
		glEnd();
		break;
	}
	// Horizontal & vertical stripes.
	case 4:
	{
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		for (int i = 0; i < N - 1; i++)
		{
			for (int j = 0; j < N - 1; j++)
			{
				glVertex3f(plane3D[i][j].x, plane3D[i][j].y, plane3D[i][j].z);
				glVertex3f(plane3D[i][j + 1].x, plane3D[i][j + 1].y, plane3D[i][j + 1].z);

				glVertex3f(plane3D[i][j].x, plane3D[i][j].y, plane3D[i][j].z);
				glVertex3f(plane3D[i + 1][j].x, plane3D[i + 1][j].y, plane3D[i + 1][j].z);
			}
		}
		glEnd();
		break;
	}
	// Weird egg.
	case 5:
	{
		glColor3f(0.5f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		for (int i = 0; i < N - 2; i++)
		{
			for (int j = 0; j < N - 2; j++)
			{
				//glVertex3f(plane3D[i + 1][j].x, plane3D[i + 1][j].y, plane3D[i + 1][j].z);
				//glVertex3f(plane3D[i][j].x, plane3D[i][j].y, plane3D[i][j].z);
				//glVertex3f(plane3D[i][j + 1].x, plane3D[i][j + 1].y, plane3D[i][j + 1].z);
				glVertex3f(plane3D[i + 1][j + 1].x, plane3D[i + 1][j + 1].y, plane3D[i + 1][j + 1].z);
				glVertex3f(plane3D[i][j + 2].x, plane3D[i][j + 2].y, plane3D[i][j + 2].z);
				glVertex3f(plane3D[i][j].x, plane3D[i][j].y, plane3D[i][j].z);
			}
		}
		glEnd();
		break;
	}
	// Wire triangle egg.
	case 6:
	{
		glColor3f(1.0f, 0.0f, 0.5f);
		glBegin(GL_LINES);
		for (int i = 0; i < N - 1; i++)
		{
			for (int j = 0; j < N - 1; j++)
			{
				glVertex3f(plane3D[i][j].x, plane3D[i][j].y, plane3D[i][j].z);

				glVertex3f(plane3D[i + 1][j].x, plane3D[i + 1][j].y, plane3D[i + 1][j].z);

				glVertex3f(plane3D[i][j + 1].x, plane3D[i][j + 1].y, plane3D[i][j + 1].z);
			}
		}
		glEnd();

		glBegin(GL_LINES);
		for (int i = 0; i < N - 1; i++)
		{
			for (int j = 0; j < N - 1; j++)
			{
				glVertex3f(plane3D[i + 1][j + 1].x, plane3D[i + 1][j + 1].y, plane3D[i + 1][j + 1].z);

				glVertex3f(plane3D[i + 1][j].x, plane3D[i + 1][j].y, plane3D[i + 1][j].z);

				glVertex3f(plane3D[i][j + 1].x, plane3D[i][j + 1].y, plane3D[i][j + 1].z);
			}
		}
		glEnd();
		break;
	}
	// Triangle textured egg.
	case 7:
	{
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < N - 1; i++)
		{
			for (int j = 0; j < N - 1; j++)
			{
				glColor3f(colors[i][j].r, colors[i][j].g, colors[i][j].b);
				glVertex3f(plane3D[i][j].x, plane3D[i][j].y, plane3D[i][j].z);

				glColor3f(colors[i + 1][j].r, colors[i + 1][j].g, colors[i + 1][j].b);
				glVertex3f(plane3D[i + 1][j].x, plane3D[i + 1][j].y, plane3D[i + 1][j].z);

				glColor3f(colors[i][j + 1].r, colors[i][j + 1].g, colors[i][j + 1].b);
				glVertex3f(plane3D[i][j + 1].x, plane3D[i][j + 1].y, plane3D[i][j + 1].z);
			}
		}
		glEnd();

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < N - 1; i++)
		{
			for (int j = 0; j < N - 1; j++)
			{
				glColor3f(colors[i + 1][j + 1].r, colors[i + 1][j + 1].g, colors[i + 1][j + 1].b);
				glVertex3f(plane3D[i + 1][j + 1].x, plane3D[i + 1][j + 1].y, plane3D[i + 1][j + 1].z);

				glColor3f(colors[i + 1][j].r, colors[i + 1][j].g, colors[i + 1][j].b);
				glVertex3f(plane3D[i + 1][j].x, plane3D[i + 1][j].y, plane3D[i + 1][j].z);

				glColor3f(colors[i][j + 1].r, colors[i][j + 1].g, colors[i][j + 1].b);
				glVertex3f(plane3D[i][j + 1].x, plane3D[i][j + 1].y, plane3D[i][j + 1].z);
			}
		}
		glEnd();
		break;
		//glBegin(GL_TRIANGLES);
		//for (int i = 0; i < N - 1; i++)
		//{
		//	for (int j = 0; j < N - 1; j++)
		//	{
		//		glColor3f(0.0f, 1.0f, 0.0f);
		//		glVertex3f(plane3D[i][j].x, plane3D[i][j].y, plane3D[i][j].z);

		//		glColor3f(0.0f, 1.0f, 0.0f);
		//		glVertex3f(plane3D[i + 1][j].x, plane3D[i + 1][j].y, plane3D[i + 1][j].z);

		//		glColor3f(0.0f, 1.0f, 0.0f);
		//		glVertex3f(plane3D[i][j + 1].x, plane3D[i][j + 1].y, plane3D[i][j + 1].z);

		//	}
		//}
		//glEnd();
		//glBegin(GL_TRIANGLES);
		//for (int i = 0; i < N - 1; i++)
		//{
		//	for (int j = 0; j < N - 1; j++)
		//	{
		//		glColor3f(1.0f, 0.0f, 0.0f);
		//		glVertex3f(plane3D[i + 1][j + 1].x, plane3D[i + 1][j + 1].y, plane3D[i + 1][j + 1].z);

		//		glColor3f(1.0f, 0.0f, 0.0f);
		//		glVertex3f(plane3D[i + 1][j].x, plane3D[i + 1][j].y, plane3D[i + 1][j].z);

		//		glColor3f(1.0f, 0.0f, 0.0f);
		//		glVertex3f(plane3D[i][j + 1].x, plane3D[i][j + 1].y, plane3D[i][j + 1].z);

		//	}
		//}
		//glEnd();
		//break;

	}
	}
}

void RenderScene()
{
	// glClear — clear buffers to preset values.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	Axes();

	glRotatef(theta[0], 1.0, 0.0, 0.0);
	glRotatef(theta[1], 0.0, 1.0, 0.0);
	glRotatef(theta[2], 0.0, 0.0, 1.0);

	// Teapot.
	//glColor3f(0.0f, 1.0f, 0.0f); // Ustawienie koloru rysowania na biały
	//glutWireTeapot(3.0); // Narysowanie obrazu czajnika do herbaty

	RenderEgg(0.0f, -5.0f, 0.0f);
	//RenderEgg(0.0f, -5.0f, 10.0f);

	// glFlush — force execution of GL commands in finite time.
	// Different GL implementations buffer commands in several different locations, including network buffers and the graphics accelerator itself. 
	// glFlush empties all of these buffers, causing all issued commands to be executed as quickly as they are accepted by the actual rendering engine. 
	// Though this execution may not be completed in any particular time period, it does complete in finite time.
	glFlush();
}
#pragma endregion

#pragma region Basics
void KeyReader(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'p':
	{
		renderMode = 1;
		break;
	}
	case 'w':
	{
		renderMode = 2;
		break;
	}
	case 's':
	{
		renderMode = 3;
		break;
	}
	case 'z':
	{
		spinMode = 1;
		break;
	}
	case 'x':
	{
		spinMode = 3;
		break;
	}
	case '1':
	{
		renderMode = 1;
		break;
	}
	case '2':
	{
		renderMode = 2;
		break;
	}
	case '3':
	{
		renderMode = 3;
		break;
	}
	case '4':
	{
		renderMode = 4;
		break;
	}
	case '5':
	{
		renderMode = 5;
		break;
	}
	case '6':
	{
		renderMode = 6;
		break;
	}
	case '7':
	{
		renderMode = 7;
		break;
	}
	}
}

void CustomInitialization(void)
{
	// glClearColor — specify clear values for the color buffers.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black.
										  //glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // Grey.
}

void ChangeSize(GLsizei horizontal, GLsizei vertical)
{
	GLfloat AspectRatio;

	if (vertical == 0)
	{
		vertical = 1;
	}

	// glViewport — set the viewport.
	// Specify the lower left corner of the viewport rectangle, in pixels. The initial value is (0,0).
	// Specify the width and height of the viewport.When a GL context is first attached to a window, width and height are set to the dimensions of that window.
	glViewport(0, 0, horizontal, vertical);

	// glMatrixMode — specify which matrix is the current matrix.
	// Specifies which matrix stack is the target for subsequent matrix operations. 
	// Three values are accepted: GL_MODELVIEW, GL_PROJECTION, and GL_TEXTURE. 
	// The initial value is GL_MODELVIEW. Additionally, if the ARB_imaging extension is supported, GL_COLOR is also accepted.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	AspectRatio = (GLfloat)horizontal / (GLfloat)vertical;

	// glOrtho — multiply the current matrix with an orthographic matrix.
	// glOrtho describes a transformation that produces a parallel projection.
	// The current matrix(see glMatrixMode) is multiplied by this matrix and the result replaces the current matrix.
	if (horizontal <= vertical)
	{
		glOrtho(-7.5, 7.5, -7.5 / AspectRatio, 7.5 / AspectRatio, 10.0, -10.0);
	}
	else
	{
		glOrtho(-7.5 * AspectRatio, 7.5 * AspectRatio, -7.5, 7.5, 10.0, -10.0);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
#pragma endregion

void main(int argc, char* argv[])
{
	FillColorMatrix();
	// glutInitDisplayMode sets the initial display mode.
	// https://www.opengl.org/resources/libraries/glut/spec3/node12.html
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	// GLUT_SINGLE
	// Bit mask to select a single buffered window.This is the default if neither GLUT_DOUBLE or GLUT_SINGLE are specified.
	// GLUT_RGBA
	// Bit mask to select an RGBA mode window.This is the default if neither GLUT_RGBA nor GLUT_INDEX are specified.
	// GLUT_DEPTH
	// Bit mask to select a window with a depth buffer.

	// glutInit is used to initialize the GLUT library.
	glutInit(&argc, argv);

	glutInitWindowSize(300, 300);

	// glutCreateWindow creates a top-level window.
	glutCreateWindow("E G G");

	// glutReshapeFunc sets the reshape callback for the current window.
	glutReshapeFunc(ChangeSize);

	// glutDisplayFunc sets the display callback for the current window.
	glutDisplayFunc(RenderScene);

	glEnable(GL_DEPTH_TEST);

	// glutIdleFunc sets the global idle callback.
	glutIdleFunc(SpinEgg);

	// glutKeyboardFunc sets the keyboard callback for the current window.
	glutKeyboardFunc(KeyReader);

	// Preparing before start.
	CustomInitialization();

	// glutMainLoop enters the GLUT event processing loop.
	// glutMainLoop enters the GLUT event processing loop. This routine should be called at most once in a GLUT program. 
	// Once called, this routine will never return. It will call as necessary any callbacks that have been registered.
	glutMainLoop();
}