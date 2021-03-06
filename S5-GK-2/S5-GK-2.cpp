#include "stdafx.h"
#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <random>

// Sterowanie stopniem deformacji.
float perturbationRatio = 0.75f;

// Generator Mersenne Twister.
static std::random_device source;
static std::mt19937 rng(source());
static std::uniform_real_distribution<float> perturbationRandom(-perturbationRatio, perturbationRatio);

// Rozmiar obiektów.
float length = 175.0f;

// Ilość rekurencji.
int desiredRecurrency = 3;

#pragma region RenderSquare
float rC()
{
	// 1 albo 0 we float.
	return rand() % 2;
}

void RenderSquare(float x, float y, float length)
{
	glBegin(GL_POLYGON);

	// Bez perturbacji.
	//// (X, Y)
	//glColor3f(rC(), rC(), rC());
	//glVertex2f(x, y);

	//// (X + L, Y)
	//glColor3f(rC(), rC(), rC());
	//glVertex2f(x, y + length);

	//// (X + L, Y + L)
	//glColor3f(rC(), rC(), rC());
	//glVertex2f(x + length, y + length);

	//// (X + L, Y)
	//glColor3f(rC(), rC(), rC());
	//glVertex2f(x + length, y);

	// Perturbacje Mersenne Twister.
	// (X, Y)
	glColor3f(rC(), rC(), rC());
	glVertex2f(x + perturbationRandom(rng), y + perturbationRandom(rng));

	// (X + L, Y)
	glColor3f(rC(), rC(), rC());
	glVertex2f(x + perturbationRandom(rng), y + perturbationRandom(rng) + length);

	// (X + L, Y + L)
	glColor3f(rC(), rC(), rC());
	glVertex2f(x + perturbationRandom(rng) + length, y + perturbationRandom(rng) + length);

	// (X + L, Y)
	glColor3f(rC(), rC(), rC());
	glVertex2f(x + perturbationRandom(rng) + length, y + perturbationRandom(rng));

	glEnd();
}
#pragma endregion

#pragma region DeleteSquare
// Rysowanie kwadratu w kolorze tła.
void DeleteSquare(float x, float y, float size)
{
	glBegin(GL_POLYGON);

	// (X, Y)
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
	glVertex2f(x, y);
	// (X + L, Y)
	glVertex2f(x, y + size);
	// (X, Y + L)
	glVertex2f(x + size, y + size);
	// (X + L, Y + L)
	glVertex2f(x + size, y);

	glEnd();
}
#pragma endregion

#pragma region RecurrentRendering
// Rysowanie 8 kwadratów, usuwanie środkowego, wywołanie rekruencyjne dla kwadratów.
void RecurrentRendering(float x, float y, float size, int currentRecurrency)
{
	// Kwadrat (1x1) => 9 kwadratów (3x3).
	// Rozmiar podzielony przez 3.
	size = size / 3;

	// Rysowanie przeciwnie do ruchu zegara od kwadratu w rogu (0,0).
	// OOO
	// OOO
	// OOO
	RenderSquare(x, y, size);
	RenderSquare(x, y + size, size);
	RenderSquare(x, y + 2 * size, size);
	// OOO
	// OOO
	// XXX
	RenderSquare(x + size, y + 2 * size, size);
	// OOO
	// OOX
	// XXX
	RenderSquare(x + 2 * size, y + 2 * size, size);
	RenderSquare(x + 2 * size, y + size, size);
	RenderSquare(x + 2 * size, y, size);
	// XXX
	// OOX
	// XXX
	RenderSquare(x + size, y, size);
	// XXX
	// XDX
	// XXX
	DeleteSquare(x + size, y + size, size);

	currentRecurrency++;
	if (currentRecurrency < desiredRecurrency)
	{
		// Wywoływanie rekurencji w sposób analogiczny do poprzedniego fragmentu.
		RecurrentRendering(x, y, size, currentRecurrency);
		RecurrentRendering(x, y + size, size, currentRecurrency);
		RecurrentRendering(x, y + 2 * size, size, currentRecurrency);
		RecurrentRendering(x + size, y + 2 * size, size, currentRecurrency);
		RecurrentRendering(x + 2 * size, y + 2 * size, size, currentRecurrency);
		RecurrentRendering(x + 2 * size, y + size, size, currentRecurrency);
		RecurrentRendering(x + 2 * size, y, size, currentRecurrency);
		RecurrentRendering(x + size, y, size, currentRecurrency);
	}
}
#pragma endregion

#pragma region RenderCarpet
void RenderCarpet()
{
	// glClear — clear buffers to preset values.
	glClear(GL_COLOR_BUFFER_BIT);

	// Start w odpowiednim rogu okna.
	float halfSize = length / 2;
	//RenderSquare(-halfSize, -halfSize, length);

	// "Dywanowanie".
	RecurrentRendering(-halfSize, -halfSize, length, 0);

	// glFlush — force execution of GL commands in finite time.
	// Different GL implementations buffer commands in several different locations, including network buffers and the graphics accelerator itself. 
	// glFlush empties all of these buffers, causing all issued commands to be executed as quickly as they are accepted by the actual rendering engine. 
	// Though this execution may not be completed in any particular time period, it does complete in finite time.
	glFlush();
}
#pragma endregion

#pragma region CustomInitialization
// Inicjalizacja przed renderowaniem.
void CustomInitialization(void)
{
	// glClearColor — specify clear values for the color buffers.
	// Parametry - red, green, blue, alpha.
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}
#pragma endregion

#pragma region ChangeSize
void ChangeSize(GLsizei horizontal, GLsizei vertical)
{
	GLfloat AspectRatio;

	// Zabezpieczenie przed dzieleniem przez 0.
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
	// Określenie układu współrzędnych obserwatora.
	glMatrixMode(GL_PROJECTION);

	// Określenie przestrzeni ograniczającej.
	glLoadIdentity();

	// Wyznaczenie współczynnika proporcji okna.
	AspectRatio = (GLfloat)horizontal / (GLfloat)vertical;

	// glOrtho — multiply the current matrix with an orthographic matrix.
	// glOrtho describes a transformation that produces a parallel projection.
	// The current matrix(see glMatrixMode) is multiplied by this matrix and the result replaces the current matrix.
	if (horizontal <= vertical)
	{
		glOrtho(-100.0, 100.0, -100.0 / AspectRatio, 100.0 / AspectRatio, 1.0, -1.0);
	}
	else
	{
		glOrtho(-100.0*AspectRatio, 100.0*AspectRatio, -100.0, 100.0, 1.0, -1.0);
	}

	// Określenie układu współrzędnych.    
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
}
#pragma endregion

void main(int argc, char* argv[])
{
	// glutInitDisplayMode sets the initial display mode.
	// https://www.opengl.org/resources/libraries/glut/spec3/node12.html
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	// GLUT_SINGLE
	// Bit mask to select a single buffered window.This is the default if neither GLUT_DOUBLE or GLUT_SINGLE are specified.
	// GLUT_RGBA
	// Bit mask to select an RGBA mode window.This is the default if neither GLUT_RGBA nor GLUT_INDEX are specified.

	// glutInit is used to initialize the GLUT library.
	glutInit(&argc, argv);

	// glutCreateWindow creates a top-level window.
	// Wyświetlanie okna - parametr to nagłówek.
	glutCreateWindow("Program GK2");

	// glutReshapeFunc sets the reshape callback for the current window.
	glutReshapeFunc(ChangeSize);

	// glutDisplayFunc sets the display callback for the current window.
	// RenderScene będzie wywoływane przy każdej zmianie zawartości okna.
	glutDisplayFunc(RenderCarpet);

	// Przygotowanie przed renderowaniem.
	CustomInitialization();

	// glutMainLoop enters the GLUT event processing loop.
	// glutMainLoop enters the GLUT event processing loop. This routine should be called at most once in a GLUT program. 
	// Once called, this routine will never return. It will call as necessary any callbacks that have been registered.
	glutMainLoop();
}
