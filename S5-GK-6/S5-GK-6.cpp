#include "stdafx.h"
#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <random>
#include <array>
#include <cmath>
#include <iostream>

#pragma region Variables
// Generator Mersenne Twister.
static std::random_device source;
static std::mt19937 rng(source());
static std::uniform_real_distribution<float> colorRandom(0.0f, 1.0f);
#pragma region Viewer
static GLfloat viewer[] = {0.0, 0.0, 10.0};

static GLfloat rotateTheta[] = {0.0, 0.0};
static GLfloat pixelsToAngle;

enum RotationMode
{
	basic,
	spherical
};

static RotationMode currentRotationMode = spherical;

static GLfloat radius = 10.0;
static GLfloat azimuth = 0.0;
static GLfloat elevation = 0.0;
#pragma endregion

#pragma region Mouse

enum MouseMode
{
	view,
	light
};

static MouseMode currentMouseMode = view;

enum MouseState
{
	none,
	lmb,
	rmb
};

static MouseState currentMouseState = none;

static int xOldMousePosition = 0;
static int yOldMousePosition = 0;
static int rmbOldMousePosition = 0;

static int xDelta = 0;
static int yDelta = 0;
static int rmbDelta = 0;
#pragma endregion

#pragma region Light
//static GLfloat light0_position[] = { 0.0, 0.0, 10.0, 1.0 };
//static GLfloat light1_position[] = { 0.0, 0.0, 10.0, 1.0 };
static GLfloat light_positions[4][4] = {{10.0, 0.0, 0.0, 1.0},{10.0, 0.0, 0.0, 1.0},{-10.0, 0.0, 0.0, 1.0},{-10.0, 0.0, 0.0, 1.0}};

static int currentLight = 0;
static GLfloat light_radius[] = {15.0, 25.0, 17.0, 30.0};
static GLfloat light_azimuth[] = {0.0, 0.0, 0.0, 0.0};
static GLfloat light_elevation[] = {0.0, 0.0, 0.0, 0.0};
#pragma endregion

#pragma region Egg

#pragma region Structs
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

struct Normalized
{
	long double xu;
	long double xv;
	long double yu;
	long double yv;
	long double zu;
	long double zv;
};

struct Color
{
	float r;
	float g;
	float b;
};
#pragma endregion
const int N = 100;

// Matrices with coordinates of points.
std::array<std::array<Coordinates2, N + 1>, N + 1> plane2D;
std::array<std::array<Coordinates3, N + 1>, N + 1> plane3D;
std::array<std::array<Coordinates3, N + 1>, N + 1> normalized;

enum RenderMode
{
	points,
	horizontal,
	vertical,
	hove,
	weird,
	triangles,
	textured,
	striped,
	pulsing
};

static RenderMode currentRenderMode = triangles;

bool tiltEgg = false;
int counter = 0;

typedef float point3[3];
std::array<std::array<Color, N>, N> colors;
Color pulsingEgg = {0.0, 0.0, 0.0};
bool pulsingColor = false;

#pragma endregion

#pragma region Disco
static bool discoTime = false;
static int discoCounter = 0;
static int discoCurrentBackground = 0;
static std::array<Color, 6> discoColors;
#pragma endregion
#pragma endregion

#pragma region Texture
GLbyte* LoadTGAImage(const char* FileName, GLint* ImWidth, GLint* ImHeight, GLint* ImComponents, GLenum* ImFormat)
{
#pragma pack(1)
	typedef struct
	{
		GLbyte idlength;
		GLbyte colormaptype;
		GLbyte datatypecode;
		unsigned short colormapstart;
		unsigned short colormaplength;
		unsigned char colormapdepth;
		unsigned short x_orgin;
		unsigned short y_orgin;
		unsigned short width;
		unsigned short height;
		GLbyte bitsperpixel;
		GLbyte descriptor;
	} TGAHEADER;
#pragma pack(8)

	FILE* pFile;
	TGAHEADER tgaHeader;
	unsigned long lImageSize;
	short sDepth;
	GLbyte* pbitsperpixel = nullptr;

	// Default values.
	*ImWidth = 0;
	*ImHeight = 0;
	*ImFormat = GL_BGR_EXT;
	*ImComponents = GL_RGB8;

	pFile = fopen(FileName, "rb");
	if (pFile == nullptr)
		return nullptr;

	// File header reading.
	fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);

	// Reading width, height and depth.
	*ImWidth = tgaHeader.width;
	*ImHeight = tgaHeader.height;
	sDepth = tgaHeader.bitsperpixel / 8;


	// Checking if depth is 8/24/32 bits.
	if (tgaHeader.bitsperpixel != 8 && tgaHeader.bitsperpixel != 24 && tgaHeader.bitsperpixel != 32)
		return nullptr;

	// Caluclating buffer size.
	lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

	// Memory allocation.
	pbitsperpixel = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));

	if (pbitsperpixel == nullptr)
		return nullptr;

	if (fread(pbitsperpixel, lImageSize, 1, pFile) != 1)
	{
		free(pbitsperpixel);
		return nullptr;
	}

	// OpenGL format selection.
	switch (sDepth)

	{
	case 3:

		*ImFormat = GL_BGR_EXT;

		*ImComponents = GL_RGB8;

		break;

	case 4:

		*ImFormat = GL_BGRA_EXT;

		*ImComponents = GL_RGBA8;

		break;

	case 1:

		*ImFormat = GL_LUMINANCE;

		*ImComponents = GL_LUMINANCE8;

		break;
	};


	fclose(pFile);

	return pbitsperpixel;
}
#pragma endregion

#pragma region Axes
void RenderAxes(void)
{
	// Size of axes
	point3 x_min = {-5.0, 0.0, 0.0};
	point3 x_max = {5.0, 0.0, 0.0};

	point3 y_min = {0.0, -5.0, 0.0};
	point3 y_max = {0.0, 5.0, 0.0};

	point3 z_min = {0.0, 0.0, -5.0};
	point3 z_max = {0.0, 0.0, 5.0};

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

#pragma region Viewer
void ResetViewer()
{
	viewer[0] = 0.0;
	viewer[1] = 0.0;
	viewer[2] = 10.0;
}

void SphericalViewerRotation()
{
	float vectorY;
	// a - azimuth
	// e - elevation
	//X(a, e) = R * cos(a) * cos(e)
	//Y(a, e) = R * sin(e)
	//Z(a, e) = R * sin(a) * cos(e)

	// elevation = <-pi, pi> = vertical rotation.
	if (elevation >= std::_Pi) elevation = std::_Pi;
	else if (elevation <= -std::_Pi) elevation = -std::_Pi;

	// Alternative - rotation without limits:
	//if (elevation >= std::_Pi) elevation -= 2*std::_Pi;
	//else if (elevation <= -std::_Pi) elevation += 2*std::_Pi;

	// radius = <10, 25> = distance from egg.
	if (radius >= 25) radius = 25;
	else if (radius <= 10) radius = 10;

	// Direction of vector Y.
	if (elevation > std::_Pi / 2 || elevation < -std::_Pi / 2) vectorY = -1.0;
	else vectorY = 1.0;

	viewer[0] = radius * cos(azimuth) * cos(elevation);
	viewer[1] = radius * sin(elevation);
	viewer[2] = radius * sin(azimuth) * cos(elevation);

	gluLookAt(viewer[0], viewer[1], viewer[2], 0.0, 0.0, 0.0, 0.0, vectorY, 0.0);
	if (currentMouseState != none && currentMouseMode == view)
	{
		std::cout << "azimuth = " << azimuth << " elevation = " << elevation << " radius = " << radius << "\n";
		std::cout << "viewer[0] = " << viewer[0] << " viewer[1] = " << viewer[1] << " viewer[2] = " << viewer[2] << "\n\n";
	}
}

void BasicRotation()
{
	//gluLookAt — define a viewing transformation
	gluLookAt(viewer[0], viewer[1], viewer[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	//glRotatef(angle, x, y, z)
	glRotatef(rotateTheta[0], 0.0, 1.0, 0.0);
	glRotatef(rotateTheta[1], 1.0, 0.0, 0.0);
}
#pragma endregion

#pragma region Light
void SphericalLightRotation()
{
	for (int i = 0; i < 4; i++)
	{
		//if (light_elevation[i] >= std::_Pi) light_elevation[i] = std::_Pi;
		//else if (light_elevation[i] <= -std::_Pi) light_elevation[i] = -std::_Pi;
		if (light_azimuth[i] >= 2 * std::_Pi) light_azimuth[i] -= 2 * std::_Pi;
		else if (light_azimuth[i] <= 0) light_azimuth[i] += 2 * std::_Pi;

		if (light_elevation[i] >= 2 * std::_Pi) light_elevation[i] -= 2 * std::_Pi;
		else if (light_elevation[i] <= 0) light_elevation[i] += 2 * std::_Pi;

		light_positions[i][0] = light_radius[i] * cos(light_azimuth[i]) * cos(light_elevation[i]);
		light_positions[i][1] = light_radius[i] * sin(light_elevation[i]);
		light_positions[i][2] = light_radius[i] * sin(light_azimuth[i]) * cos(light_elevation[i]);
		if (currentMouseState != none && currentMouseMode == light)
		{
			//std::cout << "light[" << i << "]:" << "\n";
			//std::cout << "azimuth = " << light_azimuth[i] << " elevation = " << light_elevation[i] << " radius = " << light_radius[i] << "\n";
			//std::cout << "viewer[0] = " << light_positions[i][0] << " viewer[1] = " << light_positions[i][1] << " viewer[2] = " << light_positions[i][2] << "\n\n";
		}
	}
	glLightfv(GL_LIGHT0, GL_POSITION, light_positions[0]);
	glLightfv(GL_LIGHT1, GL_POSITION, light_positions[1]);
	glLightfv(GL_LIGHT2, GL_POSITION, light_positions[2]);
	glLightfv(GL_LIGHT3, GL_POSITION, light_positions[3]);
}

#pragma endregion

#pragma region Render

void RenderEgg(float x, float y, float z)
{
#pragma region Display mode.
	// Rendering points.
	//glTranslatef(0.0, -5.0f, 0.0); // Moves the egg down so its centered.
	glTranslatef(x, y, z);
	if (tiltEgg)
	{
		glRotatef(20.0, 1.0f, 0.0f, 0.0f); // Slight egg tilt forward.
	}
	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_TRIANGLES);
	float divisor = N * 1.0f;
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			glNormal3f(normalized[i + 1][j].x, normalized[i + 1][j].y, normalized[i + 1][j].z);
			glTexCoord2f((i + 1) / divisor, j / divisor);
			glVertex3f(plane3D[i + 1][j].x, plane3D[i + 1][j].y, plane3D[i + 1][j].z);

			glNormal3f(normalized[i][j + 1].x, normalized[i][j + 1].y, normalized[i][j + 1].z);
			glTexCoord2f(i / divisor, (j + 1) / divisor);
			glVertex3f(plane3D[i][j + 1].x, plane3D[i][j + 1].y, plane3D[i][j + 1].z);

			glNormal3f(normalized[i + 1][j + 1].x, normalized[i + 1][j + 1].y, normalized[i + 1][j + 1].z);
			glTexCoord2f((i + 1) / divisor, (j + 1) / divisor);
			glVertex3f(plane3D[i + 1][j + 1].x, plane3D[i + 1][j + 1].y, plane3D[i + 1][j + 1].z);

			glNormal3f(normalized[i][j].x, normalized[i][j].y, normalized[i][j].z);
			glTexCoord2f(i / divisor, j / divisor);
			glVertex3f(plane3D[i][j].x, plane3D[i][j].y, plane3D[i][j].z);

			glNormal3f(normalized[i + 1][j].x, normalized[i + 1][j].y, normalized[i + 1][j].z);
			glTexCoord2f((i + 1) / divisor, j / divisor);
			glVertex3f(plane3D[i + 1][j].x, plane3D[i + 1][j].y, plane3D[i + 1][j].z);

			glNormal3f(normalized[i][j + 1].x, normalized[i][j + 1].y, normalized[i][j + 1].z);
			glTexCoord2f(i / divisor, (j + 1) / divisor);
			glVertex3f(plane3D[i][j + 1].x, plane3D[i][j + 1].y, plane3D[i][j + 1].z);
		}
	}
	glEnd();
#pragma endregion
}

void RenderScene()
{
	// glClear — clear buffers to preset values.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	switch (currentMouseMode)
	{
	case view:
		{
			if (currentMouseState == lmb)
			{
				azimuth += xDelta / 80.0;
				elevation += yDelta / 80.0;
			}
			if (currentMouseState == rmb)
			{
				radius += rmbDelta / 10.0;
			}
			//SphericalViewerRotation();
			break;
		}
	case light:
		{
			if (currentMouseState == lmb)
			{
				light_azimuth[0] += xDelta * pixelsToAngle / 40.0;
				light_elevation[0] += yDelta * pixelsToAngle / 40.0;
				std::cout << light_azimuth[0] << " | " << light_elevation[0] << "\n\n";
			}
			if (currentMouseState == rmb)
			{
				light_azimuth[1] += xDelta * pixelsToAngle / 40.0;
				light_elevation[1] += yDelta * pixelsToAngle / 40.0;
				std::cout << light_azimuth[1] << " | " << light_elevation[1] << "\n\n";
			}
			//SphericalLightRotation();
			break;
		}
	}

	SphericalViewerRotation();
	SphericalLightRotation();

	//RenderAxes();
	RenderEgg(0.0f, -5.0f, 0.0f);

	// glFlush — force execution of GL commands in finite time.
	// Different GL implementations buffer commands in several different locations, including network buffers and the graphics accelerator itself. 
	// glFlush empties all of these buffers, causing all issued commands to be executed as quickly as they are accepted by the actual rendering engine. 
	// Though this execution may not be completed in any particular time period, it does complete in finite time.
	glFlush();
	glutSwapBuffers();
}

#pragma endregion

#pragma region Disco

void IncrementLights()
{
	if (discoTime == false) return;

	if(discoCounter == 50)
	{
		discoCurrentBackground++;
		if (discoCurrentBackground > discoColors.size() - 1)
			discoCurrentBackground = 0;
		glClearColor(discoColors[discoCurrentBackground].r, discoColors[discoCurrentBackground].g, discoColors[discoCurrentBackground].b, 1.0f);
		discoCounter = 0;
	}
	else
	{
		discoCounter++;
	}
	 
	for (int i = 0; i < 4; i++)
	{
		float difference = i * 0.07f;
		if (i % 2 == 0)
		{
			light_azimuth[i] += difference;
			light_elevation[i] -= difference;
		}
		else
		{
			light_azimuth[i] -= difference;
			light_elevation[i] += difference;
		}
	}
	glutPostRedisplay();
}

void FillDiscoColorArray()
{
	discoColors[0] = { 0.0f, 0.0f, 0.05f };
	discoColors[1] = { 0.0f, 0.05f, 0.0f };
	discoColors[2] = { 0.05f, 0.0f, 0.0f };
	discoColors[3] = { 0.0f, 0.03f, 0.03f };
	discoColors[4] = { 0.04f, 0.03f, 0.0f };
	discoColors[5] = { 0.01f, 0.02f, 0.03f };
}

void BeginDisco()
{
	FillDiscoColorArray();
	discoTime = true;
	glEnable(GL_LIGHT0); // włączenie źródła o numerze 0
	glEnable(GL_LIGHT1); // włączenie źródła o numerze 1
	glEnable(GL_LIGHT2); // włączenie źródła o numerze 2
	glEnable(GL_LIGHT3); // włączenie źródła o numerze 3
	// PlaySound()
}
#pragma endregion

#pragma region Prepare Egg
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

	Coordinates3 transformed = {longTransformation * cos(std::_Pi * v), 160 * pow(u, 4.0) - 320 * pow(u, 3.0) + 160 * pow(u, 2.0),
		longTransformation * sin(std::_Pi * v)};

	return transformed;
}

Coordinates3 NormalTransform(Coordinates2 coordinates, int i, int j)
{
	long double u = coordinates.x;
	long double v = coordinates.y;
	float longTransformationU = -450 * pow(u, 4.0) + 900 * pow(u, 3.0) - 810 * pow(u, 2.0) + 360 * u - 45;
	float longTransformationV = 90 * pow(u, 5.0) - 225 * pow(u, 4.0) + 270 * pow(u, 3.0) - 180 * pow(u, 2.0) + 45 * u;
	// Transformation:
	// xu = (-450u^4 + 900u^3 - 810u^2 +360u -45)cos(pi*v)
	// xv = pi(90u^5 - 225u^4 + 270u^3 - 180u^2 + 45u)sin(pi*v)
	// yu = 640u^3 - 960u^2 + 320u
	// yv = 0
	// zu = (-450u^4 + 900u^3 - 810u^2 +360u -45)sin(pi*v)
	// zv = -pi(90u^5 - 225u^4 + 270u^3 - 180u^2 + 45u)cos(pi*v)
	float xu = longTransformationU * cos(std::_Pi * v);
	float xv = std::_Pi * longTransformationV * sin(std::_Pi * v);
	float yu = 640 * pow(u, 3.0) - 960 * pow(u, 2.0) + 320 * u;
	float yv = 0;
	float zu = longTransformationU * sin(std::_Pi * v);
	float zv = -1 * std::_Pi * longTransformationV * cos(std::_Pi * v);

	float x = yu * zv - zu * yv;
	float y = zu * xv - xu * zv;
	float z = xu * yv - yu * xv;
	float length = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

	Coordinates3 transformed;
	if (i < N / 2)
	{
		transformed = {x / length, y / length, z / length};
	}
	else if (i > N / 2)
	{
		transformed = {-x / length, -y / length, -z / length};
	}
	else if (i == N / 2) // Top.
	{
		transformed = {0, 1, 0};
	}
	else if (i == 0 || i == N) // Bottom.
	{
		transformed = {0, -1, 0};
	}
	return transformed;
}

void PopulateMatrices()
{
	// Populating the 2D matrix.
	double castedN = N;
	for (int i = 0; i < N + 1; i++)
	{
		for (int j = 0; j < N + 1; j++)
		{
			plane2D[i][j] = {static_cast<long double>(i / castedN), static_cast<long double>(j / castedN)};
		}
	}


	// Populating the 3D matrix.
	for (int i = 0; i < N + 1; i++)
	{
		for (int j = 0; j < N + 1; j++)
		{
			plane3D[i][j] = PlaneTransform(plane2D[i][j]);
			normalized[i][j] = NormalTransform(plane2D[i][j], i, j);
		}
	}
}

void FillColorMatrix()
{
	for (int i = 0; i < N - 1; i++)
	{
		for (int j = 0; j < N - 1; j++)
		{
			colors[i][j] = {colorRandom(rng), colorRandom(rng), colorRandom(rng)};
		}
	}
}
#pragma endregion

#pragma region GLUT utility
void KeyReader(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'r':
		{
			ResetViewer();
			break;
		}
	case 'd':
		{
			BeginDisco();
			break;
		}
	case 'b':
		{
			light_radius[0] += 1.0f;
			std::cout << "light_radius[0]:" << light_radius[0] << "\n";
			break;
		}
	case 'v':
		{
			light_radius[0] -= 1.0f;
			std::cout << "light_radius[0]:" << light_radius[0] << "\n";
			break;
		}
	case 'm':
		{
			light_radius[1] += 1.0f;
			std::cout << "light_radius[1]:" << light_radius[1] << "\n";
			break;
		}
	case 'n':
		{
			light_radius[1] -= 1.0f;
			std::cout << "light_radius[1]:" << light_radius[1] << "\n";
			break;
		}
	case 't':
		{
			azimuth -= 0.05;
			glutPostRedisplay();
			std::cout << "azimuth = " << azimuth << " elevation = " << elevation << " radius = " << radius << "\n";
			std::cout << "viewer[0] = " << viewer[0] << " viewer[1] = " << viewer[1] << " viewer[2] = " << viewer[2] << "\n\n";
			break;
		}
	case 'y':
		{
			azimuth += 0.05;
			glutPostRedisplay();
			std::cout << "azimuth = " << azimuth << " elevation = " << elevation << " radius = " << radius << "\n";
			std::cout << "viewer[0] = " << viewer[0] << " viewer[1] = " << viewer[1] << " viewer[2] = " << viewer[2] << "\n\n";
			break;
		}
	case 'g':
		{
			elevation -= 0.05;
			glutPostRedisplay();
			std::cout << "azimuth = " << azimuth << " elevation = " << elevation << " radius = " << radius << "\n";
			std::cout << "viewer[0] = " << viewer[0] << " viewer[1] = " << viewer[1] << " viewer[2] = " << viewer[2] << "\n\n";
			break;
		}
	case 'h':
		{
			elevation += 0.05;
			glutPostRedisplay();
			std::cout << "azimuth = " << azimuth << " elevation = " << elevation << " radius = " << radius << "\n";
			std::cout << "viewer[0] = " << viewer[0] << " viewer[1] = " << viewer[1] << " viewer[2] = " << viewer[2] << "\n\n";
			break;
		}
	case 'u':
		{
			radius += 1;
			glutPostRedisplay();
			std::cout << "azimuth = " << azimuth << " elevation = " << elevation << " radius = " << radius << "\n";
			std::cout << "viewer[0] = " << viewer[0] << " viewer[1] = " << viewer[1] << " viewer[2] = " << viewer[2] << "\n\n";
			break;
		}
	case 'j':
		{
			radius -= 1;
			glutPostRedisplay();
			std::cout << "azimuth = " << azimuth << " elevation = " << elevation << " radius = " << radius << "\n";
			std::cout << "viewer[0] = " << viewer[0] << " viewer[1] = " << viewer[1] << " viewer[2] = " << viewer[2] << "\n\n";
			break;
		}
	case 'x':
		{
			tiltEgg = !tiltEgg;
			break;
		}
	case 'z':
		{
			if (currentMouseMode == view)
				currentMouseMode = light;
			else
				currentMouseMode = view;
			break;
		}
	case 'c':
		{
			if (currentRotationMode == spherical)
			{
				currentRotationMode = basic;
			}
			else
			{
				currentRotationMode = spherical;
			}
			break;
		}
	case '1':
		{
			currentRenderMode = points;
			break;
		}
	case '2':
		{
			currentRenderMode = horizontal;
			break;
		}
	case '3':
		{
			currentRenderMode = vertical;
			break;
		}
	case '4':
		{
			currentRenderMode = hove;
			break;
		}
	case '5':
		{
			currentRenderMode = triangles;
			break;
		}
	case '6':
		{
			currentRenderMode = textured;
			break;
		}
	case '7':
		{
			currentRenderMode = striped;
			break;
		}
	case '8':
		{
			currentRenderMode = pulsing;
			break;
		}
	case '9':
		{
			//currentRenderMode = 0;
			break;
		}
	}
	RenderScene();
}

void MouseReader(int button, int state, int x, int y)
{
	if (currentMouseMode == view)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			xOldMousePosition = x;
			yOldMousePosition = y;
			currentMouseState = lmb;

			return;
		}

		if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		{
			rmbOldMousePosition = y;
			currentMouseState = rmb;
			return;
		}
	}

	else
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			xOldMousePosition = x;
			yOldMousePosition = y;
			currentMouseState = lmb;
			currentLight = 0;
			return;
		}

		if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		{
			xOldMousePosition = x;
			yOldMousePosition = y;
			currentMouseState = rmb;
			currentLight = 1;
			return;
		}
	}

	currentMouseState = none;
}

void Motion(GLsizei x, GLsizei y)
{
	xDelta = x - xOldMousePosition;
	xOldMousePosition = x;

	yDelta = y - yOldMousePosition;
	yOldMousePosition = y;

	rmbDelta = y - rmbOldMousePosition;
	rmbOldMousePosition = y;
	glutPostRedisplay();
}

void CustomInitialization(void)
{
	// glClearColor — specify clear values for the color buffers.
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // Black.
	//glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // Grey.


#pragma region Light

	// RGBA arrays.
	GLfloat mat_ambient[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_shininess = {20.0};

	GLfloat light0_ambient[] = {0.0, 0.0, 0.5, 1.0};
	GLfloat light0_diffuse[] = {0.0, 0.0, 1.0, 1.0};
	GLfloat light0_specular[] = {0.0, 0.0, 1.0, 1.0};

	GLfloat light1_ambient[] = {0.0, 0.5, 0.0, 1.0};
	GLfloat light1_diffuse[] = {0.0, 1.0, 0.0, 1.0};
	GLfloat light1_specular[] = {0.0, 1.0, 0.0, 1.0};

	GLfloat light2_ambient[] = {0.0, 0.1, 0.35, 1.0};
	GLfloat light2_diffuse[] = {0.0, 0.2, 0.7, 1.0};
	GLfloat light2_specular[] = {0.0, 0.2, 0.7, 1.0};

	GLfloat light3_ambient[] = {0.5, 0.0, 0.0, 1.0};
	GLfloat light3_diffuse[] = {1.0, 0.0, 0.0, 1.0};
	GLfloat light3_specular[] = {1.0, 0.0, 0.0, 1.0};
	//GLfloat att_constant = static_cast<float>(1.0);
	//GLfloat att_linear = static_cast<float>(0.05);
	//GLfloat att_quadratic = static_cast<float>(0.001);

	GLfloat att_constant = {1.0f};
	GLfloat att_linear = {0.001f};
	GLfloat att_quadratic = {0.01f};

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_positions[0]);

	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light_positions[1]);

	glLightfv(GL_LIGHT2, GL_AMBIENT, light2_ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, light2_specular);
	glLightfv(GL_LIGHT2, GL_POSITION, light_positions[2]);

	glLightfv(GL_LIGHT3, GL_AMBIENT, light3_ambient);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, light3_diffuse);
	glLightfv(GL_LIGHT3, GL_SPECULAR, light3_specular);
	glLightfv(GL_LIGHT3, GL_POSITION, light_positions[3]);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, att_linear);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att_quadratic);

	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, att_constant);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, att_linear);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, att_quadratic);

	glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, att_constant);
	glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, att_linear);
	glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, att_quadratic);

	glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, att_constant);
	glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, att_linear);
	glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, att_quadratic);

	glShadeModel(GL_SMOOTH); // właczenie łagodnego cieniowania
	glEnable(GL_LIGHTING); // właczenie systemu oświetlenia sceny 
	glEnable(GL_DEPTH_TEST); // włączenie mechanizmu z-bufora 
#pragma endregion

#pragma region Texture
	GLbyte* pBytes;
	GLint ImWidth, ImHeight, ImComponents;
	GLenum ImFormat;

	//GlEnable(GL_CULL_FACE);

	// Loading texture.
	//pBytes = LoadTGAImage("tekstura.tga", &ImWidth, &ImHeight, &ImComponents, &ImFormat);
	//pBytes = LoadTGAImage("C:\\Users\\JPK\\Documents\\t_1024.tga", &ImWidth, &ImHeight, &ImComponents, &ImFormat);
	pBytes = LoadTGAImage("t_1024.tga", &ImWidth, &ImHeight, &ImComponents, &ImFormat);

	// Defining the 2D texture.
	glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);

	// Releasing memory.
	free(pBytes);

	// Enabling texturing.
	glEnable(GL_TEXTURE_2D);

	// Setting texturing mode.
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Texture overlapping parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#pragma endregion
}

void ChangeSize(GLsizei horizontal, GLsizei vertical)
{
	pixelsToAngle = 360.0 / (float)horizontal; // przeliczenie pikseli azimuth stopnie

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

	// glOrtho — multiply the current matrix with an orthographic matrix.
	// glOrtho describes a transformation that produces a parallel projection.
	// The current matrix(see glMatrixMode) is multiplied by this matrix and the result replaces the current matrix.
	//GLfloat AspectRatio = (GLfloat)horizontal / (GLfloat)vertical;
	//if (horizontal <= vertical)
	//{
	//	glOrtho(-7.5, 7.5, -7.5 / AspectRatio, 7.5 / AspectRatio, 10.0, -10.0);
	//}
	//else
	//{
	//	glOrtho(-7.5 * AspectRatio, 7.5 * AspectRatio, -7.5, 7.5, 10.0, -10.0);
	//}

	// gluPerspective — set up a perspective projection matrix.
	gluPerspective(70, 1.0, 1.0, 30.0);

	if (horizontal <= vertical)
	{
		// glViewport — set the viewport.
		glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);
	}
	else
	{
		glViewport((horizontal - vertical) / 2, 0, vertical, vertical);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
#pragma endregion

void main(int argc, char* argv[])
{
	PopulateMatrices();
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

	glutInitWindowSize(500, 500);

	// glutCreateWindow creates a top-level window.
	glutCreateWindow("E G G");

	// glutReshapeFunc sets the reshape callback for the current window.
	glutReshapeFunc(ChangeSize);

	// glutDisplayFunc sets the display callback for the current window.
	glutDisplayFunc(RenderScene);

	glEnable(GL_DEPTH_TEST);

	// glutKeyboardFunc sets the keyboard callback for the current window.
	glutKeyboardFunc(KeyReader);

	// glutMouseFunc sets the mouse callback for the current window.
	glutMouseFunc(MouseReader);

	glutIdleFunc(IncrementLights);

	// glutMotionFunc and glutPassiveMotionFunc set the motion and passive motion callbacks respectively for the current window.
	glutMotionFunc(Motion);
	// Preparing before start.
	CustomInitialization();

	// glutMainLoop enters the GLUT event processing loop.
	// glutMainLoop enters the GLUT event processing loop. This routine should be called at most once in a GLUT program. 
	// Once called, this routine will never return. It will call as necessary any callbacks that have been registered.
	glutMainLoop();
}
