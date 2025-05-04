/*=================================================================================================================================*/
/*-------------------------------------------------------// Start //---------------------------------------------------------------*/
/*=================================================================================================================================*/
// Includes
#include <GL/glew.h>
#include <GL/glut.h>
#include <Shader.h>
#include <Vector.h>
#include <Matrix.h>
#include <Mesh.h>
#include <Texture.h>
#include <SphericalCameraManipulator.h>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>
#include <map>

/*---------------------------------------------------// Function Prototypes //-----------------------------------------------------*/
bool initGL(int argc, char **argv);
void initShader();
void display(void);
void keyboard(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void specialKeyboard(int key, int x, int y);
void specialKeyUp(int key, int x, int y);
void handleKeys();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void Timer(int value);
void initTexture(std::string filename, GLuint &textureID);
void reshape(int width, int height);
void loadMaze(const std::string &filename, int level);
void DrawMaze();
void DrawTank(float x, float y, float z);
void render2dText(std::string text, float r, float g, float b, float x, float y);
void updateTankMovement(Vector3f &tankVelocity, float &tankAngle);
void updateCameraPosition();
void updateTurretRotation();
void DrawBall(float x, float y, float z);
void updateBallPosition();
void fireBall();
void updateSteeringAngle(float deltaTime);
void drawHUD();
void resetGame();
void updateParticles(float deltaTime);
void drawParticles();
void checkfall();

// Screen size
int screenWidth = 1080;
int screenHeight = 1080;

/*----------------------------------------------------// Global Variables //-------------------------------------------------------*/

// Environment Variables
float specularPower = 10.0f;
float g = -9.81f;
bool isPaused = false;
bool showMenu = false;
const int finalLevel = 3;
bool gameWon = false;
bool mainMenu = true;
bool levelCompleted[3] = {false, false, false};

// Coin Variables
float coinRotationAngle = 0.0f;
float coinBounce = 0.0f;
int coinsCollected = 0;
int totalCoins = 0;

// Maze Variables
int currentLevel = 1;
int selectedLevel = currentLevel;
const int MAZE_WIDTH = 15;
const int MAZE_HEIGHT = 15;
float centerX = (MAZE_WIDTH - 1) * 2.0f / 2;
float centerZ = (MAZE_HEIGHT - 1) * 2.0f / 2;
int MAZE[MAZE_HEIGHT][MAZE_WIDTH];
bool isGameOver = false;

// Tank Variables
float tankRotation = 0.0f;
float moveSpeed = 2.0f;
float rotationSpeed = 1.0f;
Vector3f tankPosition(centerX, 0.0f, centerZ);
Vector3f tankVelocity(0.0f, 0.0f, 0.0f);

// Movement Variables
float maxSpeed = 10.0f;
float friction = 3.0f;
float moveDirection = 0.0f;
float turnDirection = 0.0f;
float deltaTime = 0.016f;
float wheelRotation = 0.0f;
float steeringAngle = 0.0f;
float tankY = 0.0f;
float verticalVelocity = 0.0f;
float steeringSpeed = 10.0f;
float maxSteeringAngle = 30.0f;
float wheelRadius = 0.001f;
float wheelRotationRad = 0.0f;
bool isfalling = false;


// Jumping Variables
bool isJumping = false;
bool isOnGround = true;
float jumpVelocity = 0.0f;
const float initialJumpVelocity = 5.0f;

// Camera Variables
float cameraDistance = 7.0f;
float cameraPan = 0.0f;
bool isFirstPerson = false;
bool aiming = false;

// Turret Controls
float turretBaseRotation = 0.0f;
float targetTurretRotation = 0.0f;

// Ball Variables
float ballPosX = 0.0f;
float ballPosY = 6.0f;
float ballPosZ = 0.0f;
float ballRotationAngle = 0.0f;
bool ballActive = true;
bool isBallFired = false;
float ballLifeTime = 0.0f; // Time since ball was fired
float gravityDelay = 0.2f; // Delay in seconds before gravity starts
Vector3f ballDirection;

// Timer Variables
float remainingTime = 200.0f;

GLuint shaderProgramID;
GLuint vertexPositionAttribute; // Vertex Position Attribute Location
GLuint vertexNormalAttribute;
GLuint vertexTexcoordAttribute; // Vertex Texcoord Attribute Location

// Material Properties
GLuint LightPositionUniformLocation; // Light Position Uniform
GLuint AmbientUniformLocation;
GLuint SpecularUniformLocation;
GLuint SpecularPowerUniformLocation;

// Lighting
Vector3f lightPosition = Vector3f(20.0, 20.0, 20.0);
Vector3f ambient = Vector3f(0.1, 0.1, 0.1);
Vector3f specular = Vector3f(1.0, 0.5, 0.0);

// Viewing
SphericalCameraManipulator cameraManip;
Matrix4x4 ModelViewMatrix;		  // ModelView Matrix
GLuint MVMatrixUniformLocation;	  // ModelView Matrix Uniform
Matrix4x4 ProjectionMatrix;		  // Projection Matrix
GLuint ProjectionUniformLocation; // Projection Matrix Uniform Location
GLuint TextureMapUniformLocation; // Texture Map Location

// Gluint Textures
GLuint crateTexture;
GLuint coinTexture;
GLuint tankTexture;
GLuint ballTexture;
GLuint donutTexture;
GLuint shadowTexture;

// Component Meshes
Mesh crateMesh; // Crate Mesh
Mesh coinMesh;	// Coin Mesh
Mesh donutMesh; // Donut Mesh
Mesh shadowMesh;

// Tank Mesh
Mesh chassisMesh;	 // Chassis Mesh
Mesh turretMesh;	 // Turret Mesh
Mesh frontWheelMesh; // Front Wheel Mesh
Mesh backWheelMesh;	 // Back Wheel Mesh

// Ball Mesh
Mesh ballMesh;

// Array of key states
bool keyStates[256];

std::map<std::pair<int, int>, int> donutFallTimers;

struct Particle
{
	Vector3f position;
	Vector3f velocity;
	float life;
};

const int MAX_PARTICLES = 100;
std::vector<Particle> particles;
bool spawnParticles = false;
Vector3f particleOrigin;

// Function to read through a text file to load the maze
void loadMaze(const std::string &filename, int level)
{
	std::ifstream file(filename);
	if (!file)
	{
		std::cerr << "Error: could not open maze file: " << filename << std::endl;
		return;
	}

	std::string line;
	bool levelFound = false;
	std::string targetLevel = "LEVEL " + std::to_string(level);

	// Search for the LEVEL header
	while (std::getline(file, line))
	{
		if (line.find(targetLevel) != std::string::npos)
		{
			levelFound = true;
			break;
		}
	}

	if (!levelFound)
	{
		std::cerr << "Error: Level " << level << " not found in file." << std::endl;
		return;
	}

	totalCoins = 0;

	// Clear current maze
	for (int i = 0; i < MAZE_HEIGHT; ++i)
		for (int j = 0; j < MAZE_WIDTH; ++j)
			MAZE[i][j] = 0;

	// Read the maze layout after the LEVEL header
	for (int i = 0; i < MAZE_HEIGHT && std::getline(file, line); ++i)
	{
		std::istringstream iss(line);
		for (int j = 0; j < MAZE_WIDTH; ++j)
		{
			iss >> MAZE[i][j];

			if (MAZE[i][j] == 2)
			{
				totalCoins++;
			}
		}
	}

	file.close();
	std::cout << "Level " << level << " loaded with " << totalCoins << " coins." << std::endl;
}

/*------------------------------------------------------// Switching Levels Function //--------------------------------------------*/
void switchLevel(int direction)
{
	currentLevel += direction;

	if (currentLevel < 1)
	{
		currentLevel = 3;
	}
	if (currentLevel > 3)
	{
		gameWon = true;
	}

	if (currentLevel > finalLevel)
	{
		gameWon = true;
		currentLevel > finalLevel;
		return;
	}

	// Reset Coins for new level
	coinsCollected = 0; 

	// Reset tank position to the center of the maze
	tankPosition.x = centerX;
	tankPosition.z = centerZ;
	tankY = 0.0f;

	// Reset Time Remaining for new level
	remainingTime = 200;

	loadMaze("maze.txt", currentLevel);
	std::cout << "Switched to level" << currentLevel << std::endl;
}

void reshape(int width, int height)
{
	screenWidth = width;
	screenHeight = height;

	// Update OpenGL viewport
	glViewport(0, 0, screenWidth, screenHeight);

	// Update Projection Matrix
	ProjectionMatrix.perspective(90, (float)screenWidth / screenHeight, 0.0001, 100.0);
	glUniformMatrix4fv(ProjectionUniformLocation, 1, false, ProjectionMatrix.getPtr());
}

// Main Program Entry
int main(int argc, char **argv)
{
	// Load Maze
	loadMaze("maze.txt", currentLevel);

	// init OpenGL
	if (!initGL(argc, argv))
		return -1;

	// Init OpenGL Shader
	initShader();

	// Init Key States to false;
	for (int i = 0; i < 256; i++)
		keyStates[i] = false;

	// Init Mesh Geometry
	// Crate
	crateMesh.loadOBJ("../models/cube.obj");
	initTexture("../models/brick.bmp", crateTexture);

	// Coin
	coinMesh.loadOBJ("../models/cube.obj");
	initTexture("../models/block.bmp", coinTexture);
	shadowMesh.loadOBJ("../models/cube.obj");
	initTexture("../models/shadow.bmp", shadowTexture);

	// Tank
	chassisMesh.loadOBJ("../models/chassis.obj");
	turretMesh.loadOBJ("../models/turret.obj");
	frontWheelMesh.loadOBJ("../models/front_wheel.obj");
	backWheelMesh.loadOBJ("../models/back_wheel.obj");
	initTexture("../models/hamvee.bmp", tankTexture);

	// Ball
	ballMesh.loadOBJ("../models/ball.obj");
	initTexture("../models/ball.bmp", ballTexture);

	donutMesh.loadOBJ("../models/cube.obj");
	initTexture("../models/donut.bmp", donutTexture);

	// Start main loop
	glutMainLoop();

	// Clean-Up
	glDeleteProgram(shaderProgramID);

	return 0;
}

// Function to Initlise OpenGL
bool initGL(int argc, char **argv)
{
	// Init GLUT
	glutInit(&argc, argv);

	// Set Display Mode
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

	// Set Window Size
	glutInitWindowSize(screenWidth, screenHeight);

	// Window Position
	glutInitWindowPosition(200, 200);

	// Create Window
	glutCreateWindow("Tank Assignment");

	// Init GLEW
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return false;
	}

	// Set Display function
	glutDisplayFunc(display);

	glutReshapeFunc(reshape);

	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyUp);

	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// Start start timer function after 100 milliseconds
	glutTimerFunc(100, Timer, 0);

	return true;
}

// Init Shader
void initShader()
{
	// Create shader
	shaderProgramID = Shader::LoadFromFile("shader.vert", "shader.frag");

	// Get a handle for our vertex position buffer
	vertexPositionAttribute = glGetAttribLocation(shaderProgramID, "aVertexPosition");
	vertexNormalAttribute = glGetAttribLocation(shaderProgramID, "aVertexNormal");
	vertexTexcoordAttribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");

	MVMatrixUniformLocation = glGetUniformLocation(shaderProgramID, "MVMatrix_uniform");
	ProjectionUniformLocation = glGetUniformLocation(shaderProgramID, "ProjMatrix_uniform");
	LightPositionUniformLocation = glGetUniformLocation(shaderProgramID, "LightPosition_uniform");
	AmbientUniformLocation = glGetUniformLocation(shaderProgramID, "Ambient_uniform");
	SpecularUniformLocation = glGetUniformLocation(shaderProgramID, "Specular_uniform");
	SpecularPowerUniformLocation = glGetUniformLocation(shaderProgramID, "SpecularPower_uniform");
	TextureMapUniformLocation = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");
}

void initTexture(std::string filename, GLuint &textureID)
{
	// Generate texture and bind
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Get texture Data
	int width, height;
	char *data;
	Texture::LoadBMP(filename, width, height, data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Cleanup data - copied to GPU
	delete[] data;
}

/*-----------------------------------------------// Display Loop //----------------------------------------------------------------*/
void display(void)
{
	// Handle keys
	handleKeys();

	// Set Viewport
	glViewport(0, 0, screenWidth, screenHeight);

	// Clear the screen
	glClearColor(0.2f, 0.3f, 0.6f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use shader
	glUseProgram(shaderProgramID);

	// Projection Matrix - Perspective Projection
	ProjectionMatrix.perspective(90, 1.0, 0.0001, 100.0);

	// Set Projection Matrix
	glUniformMatrix4fv(
		ProjectionUniformLocation,	// Uniform location
		1,							// Number of Uniforms
		false,						// Transpose Matrix
		ProjectionMatrix.getPtr()); // Pointer to ModelViewMatrixValues

	glUniform3f(LightPositionUniformLocation, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
	glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
	glUniform1f(SpecularPowerUniformLocation, specularPower);

	// Apply the camera view using gluLookAt
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	DrawMaze();
	DrawTank(0.0f, 3.0f, 0.0f);
	updateBallPosition();
	DrawBall(0.0f, 6.0f, 0.0f);
	drawParticles();
	updateParticles(deltaTime);

	updateSteeringAngle(deltaTime);
	updateTurretRotation();
	updateCameraPosition();

	glutSetCursor(GLUT_CURSOR_CROSSHAIR);

	// Tank collecting coins
	int tankTileX = (int)((tankPosition.x + 1.0f) / 2.0f);
	int tankTileZ = (int)((tankPosition.z + 1.0f) / 2.0f);
	if (tankTileZ >= 0 && tankTileX < MAZE_HEIGHT && tankTileX >= 0 && tankTileZ < MAZE_WIDTH)
	{
		if (MAZE[tankTileX][tankTileZ] == 2)
		{
			MAZE[tankTileX][tankTileZ] = 1; // Remove coin
			coinsCollected++;				// Increment counter

			system("canberra-gtk-play -f coins.wav &");

			std::cout << "Coins collected: " << coinsCollected << std::endl;

			// If all coins are collected switch level
			if (coinsCollected == totalCoins)
			{
				levelCompleted[currentLevel - 1] = true;
				switchLevel(1);
				coinsCollected = 0;
			}
		}
	}

	// Unuse Shader
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_DEPTH_TEST);

	// Set up orthogonal projection for 2D text
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, 0, screenHeight);

	// Switch back to modelview matrix for 2D rendering
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	drawHUD();
	// Redraw frame
	glutPostRedisplay();
	glutSwapBuffers();
}

/*-----------------------------------------------// Draw Maze Function //----------------------------------------------------------*/
void DrawMaze()
{

	int tankRow = (int)((tankPosition.x + 1.0f) / 2.0f);
	int tankCol = (int)((tankPosition.z + 1.0f) / 2.0f);

	for (int i = 0; i < MAZE_HEIGHT; i++)
	{
		for (int j = 0; j < MAZE_WIDTH; j++)
		{
			if (MAZE[i][j] == 1 || MAZE[i][j] == 2)
			{
				// Apply Camera Manipluator to Set Model View Matrix on GPU
				ModelViewMatrix.toIdentity();
				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
				glUniformMatrix4fv(
					MVMatrixUniformLocation, // Uniform location
					1,						 // Number of Uniforms
					false,					 // Transpose Matrix
					m.getPtr());			 // Pointer to Matrix Values

				// Set Colour after program is in use
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, crateTexture);
				glUniform1i(TextureMapUniformLocation, 0);

				// First crate
				m.translate(i * 2.0, 0.0f, j * 2.0);
				glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());
				crateMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
			}

			if (MAZE[i][j] == 2)
			{
				// Set up transformation matrix
				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);

				// 1. Draw shadow
				{
					Matrix4x4 shadowMatrix = m;
					shadowMatrix.translate(i * 2.0, 1.4f, j * 2.0); // Slightly above the floor
					shadowMatrix.scale(0.3f, 0.01f, 0.3f);			// Flat and wide
					shadowMatrix.rotate(coinRotationAngle, 0.0f, 1.0f, 0.0f);

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, shadowTexture); // Or shadow texture
					glUniform1i(TextureMapUniformLocation, 0);

					glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, shadowMatrix.getPtr());
					shadowMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
				}

				// 2. Draw coin (bouncing)
				float bounceHeight = 0.1f * sin(coinBounce);
				m.translate(i * 2.0, 2.0f + bounceHeight, j * 2.0);
				m.scale(0.3f, 0.3f, 0.3f);
				m.rotate(coinRotationAngle, 0.0f, 1.0f, 0.0f);
				glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, coinTexture);
				glUniform1i(TextureMapUniformLocation, 0);
				coinMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
			}

			if (MAZE[i][j] == 3)
			{
				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, donutTexture);
				glUniform1i(TextureMapUniformLocation, 0);

				std::pair<int, int> key = std::make_pair(i, j);
				int fallFrame = donutFallTimers[key];

				// If the tank is on this tile, start shaking/falling
				if (tankRow == i && tankCol == j)
				{
					fallFrame++;
					donutFallTimers[key] = fallFrame;
				}
				else if (fallFrame > 0)
				{
					fallFrame++;
					donutFallTimers[key] = fallFrame;
				}

				// Animate shaking
				float shakeOffset = 0.0f;
				float dropOffset = 0.0f;
				if (fallFrame > 0 && fallFrame < 60)
				{
					shakeOffset = 0.1f * sin(fallFrame * 0.5f);
				}
				else if (fallFrame >= 60)
				{
					dropOffset = -((fallFrame - 60) * 0.05f);
				}

				// After 100 frames, delete the tile
				if (fallFrame >= 100)
				{
					MAZE[i][j] = 0;
					donutFallTimers.erase(key);
					continue; // skip drawing
				}

				m.translate(i * 2.0 + shakeOffset, dropOffset, j * 2.0);
				glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());
				donutMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
			}
		}
	}
}

/*------------------------------------------------// Draw Tank Function //---------------------------------------------------------*/
void DrawTank(float x, float y, float z)
{

	Matrix4x4 m = cameraManip.apply(ModelViewMatrix);

	m.translate(tankPosition.x, tankPosition.y, tankPosition.z);
	m.rotate(tankRotation, 0.0f, 1.0f, 0.0f);
	m.scale(0.3f, 0.3f, 0.3f);

	// Bind the tank
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tankTexture);
	glUniform1i(TextureMapUniformLocation, 0);

	/*-------------------------------------------------// Draw Chassis //--------------------------------------------------------------*/
	m.translate(x, y, z);
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());
	chassisMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);

	/*-------------------------------------------------// Draw Turret //---------------------------------------------------------------*/
	Matrix4x4 turretMatrix = m;
	turretMatrix.translate(0.0f, 0.0f, 0.0f);
	turretMatrix.rotate(turretBaseRotation, 0.0f, 1.0f, 0.0f);
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, turretMatrix.getPtr());
	turretMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);

	/*-------------------------------------------------// Draw Front Wheeels //--------------------------------------------------------*/
	Matrix4x4 frontWheelMatrix = m;
	frontWheelMatrix.translate(-0.1f, 1.0f, 2.2f);
	frontWheelMatrix.rotate(steeringAngle, 0.0f, 1.0f, 0.0f);
	frontWheelMatrix.rotate(wheelRotation, 1.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, frontWheelMatrix.getPtr());
	frontWheelMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);

	/*-------------------------------------------------// Draw Back Wheels //----------------------------------------------------------*/
	Matrix4x4 backWheelMatrix = m;
	backWheelMatrix.translate(-0.1f, 1.1f, -1.3f);
	backWheelMatrix.rotate(-steeringAngle, 0.0f, 1.0f, 0.0f);
	backWheelMatrix.rotate(wheelRotation, 1.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, backWheelMatrix.getPtr());
	backWheelMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}

/*------------------------------------------------// Draw Ball //-------------------------------------------------------------------*/
void DrawBall(float x, float y, float z)
{
	if (!ballActive)
		return;
	if (!isBallFired)
		return;

	ballRotationAngle += 270.0f * deltaTime;

	if (ballRotationAngle > 360.0f)
		ballRotationAngle -= 360.0f;

	Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
	m.translate(ballPosX, ballPosY, ballPosZ);
	m.scale(0.18f, 0.18f, 0.18f);
	m.rotate(ballRotationAngle, 1.0f, 0.0f, 0.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ballTexture);
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());
	ballMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}

void updateBallPosition()
{
	if (!ballActive)
		return;
	if (!isBallFired)
		return;

	ballLifeTime += deltaTime;

	ballPosX += ballDirection.x * deltaTime;
	ballPosZ += ballDirection.z * deltaTime;

	if (ballLifeTime >= gravityDelay)
	{
		float t = (ballLifeTime - gravityDelay);
		float easedGravity = g * (1.0f - expf(-3.0f * t));
		verticalVelocity += easedGravity * 100.0f * deltaTime;
	}

	ballPosY += verticalVelocity * deltaTime;

	if (ballPosY <= 0.0f)
	{
		ballPosY = 0.0f;
		ballActive = false;
		isBallFired = false;
	}

	int ballTileX = (int)((ballPosX + 1.0f) / 2.0f);
	int ballTileZ = (int)((ballPosZ + 1.0f) / 2.0f);

	if (ballTileZ >= 0 && ballTileX < MAZE_HEIGHT && ballTileX >= 0 && ballTileZ < MAZE_WIDTH)
	{
		if (MAZE[ballTileX][ballTileZ] == 2)
		{
			MAZE[ballTileX][ballTileZ] = 1; // Remove Coin
			coinsCollected++;

			system("canberra-gtk-play -f coins.wav &");

			// Spawn visual particles
			spawnParticles = true;
			particleOrigin = Vector3f(ballTileX, ballPosY, ballPosZ);
			particles.clear();
			for (int i = 0; i < MAX_PARTICLES; ++i)
			{
				Particle p;
				p.position = particleOrigin;
				p.velocity = Vector3f(
					(rand() % 100 - 50) / 50.0f,
					(rand() % 100) / 50.0f,
					(rand() % 100 - 50) / 50.0f);
				p.life = 1.0f; // Seconds
				particles.push_back(p);
			}

			std::cout << "Coins Collected: " << coinsCollected << std::endl;

			// If all coins are collected switch level
			if (coinsCollected == totalCoins)
			{
				levelCompleted[currentLevel - 1] = true;
				switchLevel(1);
				coinsCollected = 0;
			}
		}
	}
}

void updateParticles(float deltaTime)
{
	if (!spawnParticles)
		return;

	for (auto &p : particles)
	{
		p.life -= deltaTime;
		p.position = p.position + p.velocity * deltaTime;
	}

	bool anyAlive = false;
	for (const auto &p : particles)
	{
		if (p.life > 0.0f)
		{
			anyAlive = true;
			break;
		}
	}
	if (!anyAlive)
		spawnParticles = false;
}

void drawParticles()
{
	if (!spawnParticles)
		return;

	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPointSize(4.0f);
	glBegin(GL_POINTS);

	for (const auto &p : particles)
	{
		if (p.life > 0.0f)
		{
			glColor4f(1.0f, 0.0f, 0.2f, p.life); // Fades with life
			glVertex3f(p.position.x, p.position.y, p.position.z);
		}
	}
	glEnd();
	glColor4f(1, 1, 1, 1); // reset color
}

void fireBall()
{
	if (!isBallFired)
	{
		// Set initial position to the turret
		ballPosX = tankPosition.x;
		ballPosY = tankPosition.y + 2.0f;
		ballPosZ = tankPosition.z;

		// Calculate direction based on tank and turret rotation
		float angle = (tankRotation + turretBaseRotation) * (M_PI / 180.0f);
		float dirX = sin(angle);
		float dirZ = cos(angle);

		float length = sqrt(dirX * dirX + dirZ * dirZ);
		if (length != 0.0f)
		{
			dirX /= length;
			dirZ /= length;
		}

		// Fire slightly in front of tank barrel
		float offset = 0.8f;
		ballPosX = tankPosition.x + dirX * offset;
		ballPosZ = tankPosition.z + dirZ * offset;

		// Assign directional speed
		ballDirection = Vector3f(dirX, 0.0f, dirZ) * 12.0f;
		verticalVelocity = 0.0f;
		ballLifeTime = 0.0f;

		isBallFired = true;
		ballActive = true;
	}
}

/*------------------------------------------------// Tank Movement Function //-----------------------------------------------------*/
void updateTankMovement(Vector3f &tankVelocity, float &tankAngle)
{
	// Apply Turning
	tankRotation = tankRotation + turnDirection * rotationSpeed;

	// Convert rotation to direction
	float rad = tankRotation * (M_PI / 180.0f);
	Vector3f forward(sin(rad), 0.0f, cos(rad));

	// Update velocity
	tankVelocity = forward * (moveSpeed * moveDirection);

	// Apply Friction if no input
	if (moveDirection == 0.0f)
	{
		tankVelocity = tankVelocity * 0.9f;
	}

	// Update tank Position
	tankPosition = tankPosition + tankVelocity * deltaTime;

	// Calculate rotation amount
	wheelRotation += (moveSpeed * moveDirection * deltaTime) / wheelRadius;
	wheelRotationRad = wheelRotation * (M_PI / 180.0f);


	if (!isOnGround)
	{
		// Apply gravity
		jumpVelocity += g * deltaTime;
		tankPosition.y += jumpVelocity * deltaTime;

		// Check if tank lands
		if (tankPosition.y <= 0.0f)
		{
			tankPosition.y = 0.0f;
			isOnGround = true;
			isJumping = false;
			jumpVelocity = 0.0f;

			checkfall();
		}
	}
}

/*------------------------------------------------// Tank Falling Function //------------------------------------------------------*/
void checkfall()
{
	// Convert tank world position to maze indices
	int i = static_cast<int>(round(tankPosition.x / 2.0f));
	int j = static_cast<int>(round(tankPosition.z / 2.0f));

	bool onCrate = false;

	if (!isOnGround)
		return;

	if (i >= 0 && j >= 0 && i < MAZE_WIDTH && j < MAZE_HEIGHT)
	{
		if (MAZE[i][j] >= 1)
		{
			onCrate = true;
		}
	}

	if (!onCrate)
	{
		isfalling = true;
	}
	else
	{
		isfalling = false;
		tankPosition.y = 0.0f;
		verticalVelocity = 0.0f;
	}

	if (isfalling)
	{
		verticalVelocity += g * 10.0f * deltaTime;
		tankPosition.y = verticalVelocity * deltaTime;
		isGameOver = true;
	}
}

/*-----------------------------------------------------// Camera Function //-------------------------------------------------------*/
void updateCameraPosition()
{
	if (!isfalling)
	{
		if (isFirstPerson)
		{
			// Calculate tanks orientation in world space
			float targetpan = (tankRotation + turretBaseRotation) * (M_PI / 180.0f);
			float smoothing = 10.0f;
			cameraPan += (targetpan - cameraPan) * smoothing * deltaTime;

			// Cockpit view
			float tilt = -1.3f;
			float radius = 0.2f;

			float verticalOffset = 2.0f;
			float cameraY = tankPosition.y + verticalOffset;
			float cameraZ = tankPosition.z;
			float cameraX = tankPosition.x;

			Vector3f focusPoint(cameraX, cameraY, cameraZ);

			cameraManip.setPanTiltRadius(cameraPan, tilt, radius);
			cameraManip.setFocus(focusPoint);
		}
		else
		{
			// Calculate tanks orientation in world space
			float targetpan = (tankRotation + turretBaseRotation) * (M_PI / 180.0f);
			float smoothing = 5.0f;
			cameraPan += (targetpan - cameraPan) * smoothing * deltaTime;

			// Third person view
			float radius = cameraDistance;
			float tilt = -1.0f;

			cameraManip.setPanTiltRadius(cameraPan, tilt, radius);
			cameraManip.setFocus(tankPosition);
		}
	}
}

/*--------------------------------------------------------// Steering Function //-------------------------------------------------*/
void updateSteeringAngle(float deltaTime)
{
	const float maxSteeringAngle = 15.0f; // degrees
	const float returnSpeed = 60.0f;	  // degrees per second

	// Turn right
	if (turnDirection > 0.0f)
	{
		steeringAngle += steeringSpeed * deltaTime;
		if (steeringAngle > maxSteeringAngle)
			steeringAngle = maxSteeringAngle;
	}
	// Turn left
	else if (turnDirection < 0.0f)
	{
		steeringAngle -= steeringSpeed * deltaTime;
		if (steeringAngle < -maxSteeringAngle)
			steeringAngle = -maxSteeringAngle;
	}
	// Return to center if no input
	else
	{
		if (steeringAngle > 0.0f)
		{
			steeringAngle -= returnSpeed * deltaTime;
			if (steeringAngle < 0.0f)
				steeringAngle = 0.0f;
		}
		else if (steeringAngle < 0.0f)
		{
			steeringAngle += returnSpeed * deltaTime;
			if (steeringAngle > 0.0f)
				steeringAngle = 0.0f;
		}
	}

	// Clamp
	if (steeringAngle > maxSteeringAngle)
		steeringAngle = maxSteeringAngle;
	if (steeringAngle < -maxSteeringAngle)
		steeringAngle = -maxSteeringAngle;
}

/*----------------------------------------------------// KeyBoard Interaction //---------------------------------------------------*/
void keyboard(unsigned char key, int x, int y)
{
	// Quits program when esc is pressed
	if (key == 27) // esc key code
	{
		if (!gameWon && !isGameOver && !mainMenu)
		{
			showMenu = !showMenu;
			isPaused = showMenu;
		}
	}

	if (showMenu)
	{
		if (key == '1')
		{
			selectedLevel = 1;
			currentLevel = 1;
			loadMaze("maze.txt", selectedLevel);
			showMenu = false;
			isPaused = false;
		}
		else if (key == '2')
		{
			if (levelCompleted[0])
			{
				selectedLevel = 2;
				currentLevel = 2;
				loadMaze("maze.txt", selectedLevel);
				showMenu = false;
				isPaused = false;
			}
		}
		else if (key == '3')
		{
			if (levelCompleted[0] && levelCompleted[1])
			{
				selectedLevel = 3;
				currentLevel = 3;
				loadMaze("maze.txt", selectedLevel);
				showMenu = false;
				isPaused = false;
			}
		}
		else if (key == 'r' || key == 'R')
		{
			loadMaze("maze.txt", currentLevel);
			showMenu = false;
			isPaused = false;
			resetGame();
			mainMenu = true;
		}
		else if (key == 'q' || key == 'Q')
		{
			exit(0);
		}
	}
	if (mainMenu)
	{
		if (key == '1')
		{
			selectedLevel = 1;
			currentLevel = 1;
			loadMaze("maze.txt", selectedLevel);
			showMenu = false;
			isPaused = false;
			mainMenu = false;
		}
		else if (key == '2')
		{
			if (levelCompleted[0])
			{
				selectedLevel = 2;
				currentLevel = 2;
				loadMaze("maze.txt", selectedLevel);
				showMenu = false;
				isPaused = false;
				mainMenu = false;
			}
		}
		else if (key == '3')
		{
			if (levelCompleted[0] && levelCompleted[1])
			{
				selectedLevel = 3;
				currentLevel = 3;
				loadMaze("maze.txt", selectedLevel);
				showMenu = false;
				isPaused = false;
				mainMenu = false;
			}
		}
		else if (key == 'r' || key == 'R')
		{
			loadMaze("maze.txt", currentLevel);
			showMenu = false;
			isPaused = false;
			mainMenu = false;
			resetGame();
		}
		else if (key == 'q' || key == 'Q')
		{
			exit(0);
		}
	}
	else if (!isPaused && !isGameOver)
	{

		if (key == 'n' || key == 'N')
		{
			if (currentLevel < 3)
			{
				switchLevel(1);
			}
		}

		if (key == 'p' || key == 'P')
		{
			switchLevel(-1);
		}

		if (key == 'c' || key == 'C')
		{
			isFirstPerson = true;
		}
		if (key == 'v' || key == 'V')
		{
			isFirstPerson = false;
		}
	}
	if ((isGameOver || gameWon) && (key == 'r' || key == 'R'))
	{
		resetGame();
		gameWon = false;
		mainMenu = true;
	}

	if ((isGameOver || gameWon) && (key == 'q' || key == 'Q'))
	{
		exit(0);
		gameWon = false;
	}

	// Set key status
	keyStates[key] = true;
}

// Handle key up situation
void keyUp(unsigned char key, int x, int y)
{
	keyStates[key] = false;
}

/*---------------------------------------------------// HandleKeys Funuction //----------------------------------------------------*/
void handleKeys()
{
	if (isfalling)
	{
		moveDirection = 0.0f;
		turnDirection = 0.0f;
		checkfall();
		updateTankMovement(tankVelocity, tankRotation);

		return;
	}

	if (mainMenu)
		return;
	if (isPaused)
		return;

	if (keyStates['w'])
	{
		moveDirection = 1.0f;
	}
	else if (keyStates['s'])
	{
		moveDirection = -1.0f;
	}
	else
	{
		moveDirection = 0.0f;
	}

	if (tankVelocity.length() > maxSpeed)
	{
		tankVelocity = tankVelocity - tankVelocity * friction * deltaTime;
	}

	tankPosition.x += tankVelocity.x * deltaTime;
	tankPosition.y += tankVelocity.y * deltaTime;
	tankPosition.z += tankVelocity.z * deltaTime;

	if (keyStates['a'])
	{
		turnDirection = 1.0f;
	}
	else if (keyStates['d'])
	{
		turnDirection = -1.0f;
	}
	else
	{
		turnDirection = 0.0f;
	}

	if (keyStates[' '])
	{
		if (isOnGround)
		{
			isJumping = true;
			isOnGround = false;
			jumpVelocity = initialJumpVelocity;
		}
	}

	updateTankMovement(tankVelocity, tankRotation);
	checkfall();
}

/*--------------------------------------------------------// Mouse Interaction //--------------------------------------------------*/
void mouse(int button, int state, int x, int y)
{
	if (isPaused || isGameOver || mainMenu)
		return;
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			fireBall();
		}
	}

	if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			aiming = true;
		} else {
			aiming = false;
		}
	}

	glutPostRedisplay();
}

// Mouse Interaction
void motion(int x, int y)
{
	if (!aiming) return;
	if (aiming){
	if (mainMenu || isPaused)
		return;
	if (x == screenWidth / 2 && y == screenHeight / 2)
		return;

	float deltaX = -(x - screenWidth / 2);
	float dy = (screenHeight / 2 - y);

	targetTurretRotation = atan2(deltaX, dy) * (180.0f / M_PI);

	if (targetTurretRotation > 180.0f)
		targetTurretRotation -= 360.0f;
	if (targetTurretRotation < -180.0f)
		targetTurretRotation += 360.0f;
}
}

void updateTurretRotation()
{
	if (isfalling)
		return;
	float angleDifference = targetTurretRotation - turretBaseRotation;

	if (angleDifference > 180.0f)
		angleDifference -= 360.0f;
	if (angleDifference < -180.0f)
		angleDifference += 360.0f;

	if (fabs(angleDifference) > 0.01f)
	{
		turretBaseRotation += angleDifference * 0.1f;
	}
}

void specialKeyboard(int key, int x, int y)
{
	keyStates[key] = true;
}

void specialKeyUp(int key, int x, int y)
{
	keyStates[key] = false;
}

/*-------------------------------------------------------// Timer Function //------------------------------------------------------*/
void Timer(int value)
{
	if (mainMenu == 0 && !isGameOver && !isPaused && !gameWon)
	{
		remainingTime -= 0.01f;
		// Check if the time is up
		if (remainingTime < 0)
		{
			remainingTime = 0;
			isGameOver = true;
		}
	}

	// Update the coin rotation and bounce
	coinRotationAngle += 2.0f;

	if (coinRotationAngle >= 360.0f)
	{
		coinRotationAngle -= 360.0f;
	}

	coinBounce += 0.1f;

	// Redisplay the scene
	glutPostRedisplay();

	// Call function again after 10 milli seconds
	glutTimerFunc(10, Timer, 0);
}

void drawBorderBox(float x, float y, float width, float height, float r, float g, float b, float alpha = 1.0f, float lineWidth = 2.0f)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, 0, screenHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(r, g, b, alpha);
	glLineWidth(lineWidth);
	glBegin(GL_LINE_LOOP);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void drawTextBox(float x, float y, float width, float height, float r, float g, float b, float alpha = 0.5f)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, 0, screenHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(0.0f, 0.0f, 0.0f, alpha);
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void drawHUD()
{
	const int padding = 20;
	const int extraPadding = 30;
	const int charWidth = 8;
	const int statusBoxHeight = 30;
	const int helpBoxHeight = 25;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, 0, screenHeight);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	/*---------------------------------------------// Render Text //---------------------------------------------------------------*/
	if (mainMenu == 0)
	{
		if (!gameWon)
		{
			// === Top-Left: Level & Coin Status ===
			std::string levelText = "Level: " + std::to_string(currentLevel);
			std::string coinText = "Coins: " + std::to_string(coinsCollected) + "/" + std::to_string(totalCoins);
			std::string statusText = levelText + "   " + coinText;
			int statusWidth = charWidth * statusText.length();
			drawTextBox(10, screenHeight - 40, statusWidth + 2 * padding, statusBoxHeight, 1.0f, 1.0f, 1.0f, 0.8);
			render2dText(statusText, 1.0f, 1.0f, 1.0f, 10 + padding, screenHeight - 28);

			// === Top-Right: Time ===
			std::string timeText = "Time: " + std::to_string(static_cast<int>(remainingTime)) + "s";
			int timeWidth = charWidth * timeText.length();
			drawTextBox(screenWidth - timeWidth - 2 * padding - 10, screenHeight - 40, timeWidth + 2 * padding, statusBoxHeight, 1.0f, 1.0f, 1.0f, 0.8);
			render2dText(timeText, 1.0f, 1.0f, 1.0f, screenWidth - timeWidth - padding - 10, screenHeight - 28);

			// === Bottom-Left: Controls ===
			std::string helpText = "WASD: Move  |  Mouse: Aim  |  LMB: Shoot |	RMB: Aim | C: Cockpit View | V: Third-Person View";
			int helpWidth = charWidth * helpText.length();
			drawTextBox(10, 10, helpWidth + 3 * padding + extraPadding, helpBoxHeight, 1.0f, 1.0f, 1.0f, 0.8);
			render2dText(helpText, 1.0f, 1.0f, 1.0f, 10 + padding, 20);
		}
	}
	if (isGameOver)
	{
		if (!gameWon)
		{
			std::string gameOverText = "GAME OVER";
			std::string resetText = "Press R to reset";

			int GamerOverWidth = 12 * gameOverText.length(); // adjust if needed for your font size
			int resetWidth = 10 * resetText.length();
			int centerX = screenWidth / 2;
			int centerY = screenHeight / 2;
			int boxWidth = std::max(GamerOverWidth, resetWidth) + 40;
			int boxHeight = 70;

			drawTextBox(centerX - boxWidth / 2, centerY - 30, boxWidth, boxHeight, 1.0f, 1.0f, 1.0f, 0.6f);
			render2dText(gameOverText, 1.0f, 0.0f, 0.0f, centerX - GamerOverWidth / 2, centerY + 10);
			render2dText(resetText, 1.0f, 1.0f, 1.0f, centerX - resetWidth / 2 + 15, centerY - 15);
		}
	}

	if (showMenu)
	{
		std::string title = "PAUSED - Select Level";

		// Dynamic level text based on completion
		std::string level2Text = levelCompleted[0] ? "2: Level 2 (unlocked)" : "2: Level 2 (locked)";
		std::string level3Text = (levelCompleted[0] && levelCompleted[1]) ? "3: Level 3 (unlocked)" : "3: Level 3 (locked)";

		std::string options =
			"1: Level 1    " + level2Text + "     " + level3Text + "\n"
																   "R: Restart	   Q: Quit    ESC: Resume";

		int boxWidth = 10 * options.length(); // Adjust if needed
		int centerX = screenWidth / 2;
		int centerY = screenHeight / 2;

		drawTextBox(centerX - boxWidth / 2 - 10, centerY - 20, boxWidth, 90, 1.0f, 1.0f, 1.0f, 0.8);
		render2dText(title, 1.0f, 1.0f, 1.0f, centerX - title.length() * 6, centerY + 50);
		render2dText("1: Level 1", 0.8f, 0.8f, 0.8f, centerX - 230, centerY + 20);
		render2dText(level2Text, 0.8f, 0.8f, 0.8f, centerX - 220 + 130, centerY + 20);
		render2dText(level3Text, 0.8f, 0.8f, 0.8f, centerX - 160 + 260, centerY + 20);
		render2dText("R: Restart    Q: Quit    ESC: Resume", 0.8f, 0.8f, 0.8f, centerX - 150, centerY - 10);
	}

	if (gameWon)
	{
		std::string winText = "CONGRATULATIONS!";
		std::string subText = "You completed all levels!";
		std::string instructionText = "Press R to restart or Q to quit.";

		int winWidth = 400;
		int boxHeight = 100;
		int centerX = screenWidth / 2;
		int centerY = screenHeight / 2;

		// White border outline
		drawBorderBox(centerX - winWidth / 2, centerY - 22, winWidth, boxHeight, 1.0f, 1.0f, 1.0f, 1.0f, 3.0f);

		// Inner white filled box
		drawTextBox(centerX - winWidth / 2, centerY - 22, winWidth, boxHeight, 1.0f, 1.0f, 1.0f, 0.8f);
		render2dText(winText, 0.0f, 1.0f, 0.0f, centerX - winText.length() - 80, centerY + 50);
		render2dText(subText, 1.0f, 1.0f, 1.0f, centerX - subText.length() - 80, centerY + 20);
		render2dText(instructionText, 1.0f, 1.0f, 1.0f, centerX - instructionText.length() - 90, centerY);
	}

	if (mainMenu)
	{
		std::string title = "Andreas Tank Game - Select Level";

		int winWidth = 795;
		int boxHeight = 95;
		// Dynamic level text based on completion
		std::string level2Text = levelCompleted[0] ? "2: Level 2 (unlocked)" : "2: Level 2 (locked)";
		std::string level3Text = (levelCompleted[0] && levelCompleted[1]) ? "3: Level 3 (unlocked)" : "3: Level 3 (locked)";

		std::string options =
			"1: Level 1    " + level2Text + "     " + level3Text + "\n"
																   "R: Restart	   Q: Quit";

		int boxWidth = 10 * options.length(); // Adjust if needed
		int centerX = screenWidth / 2;
		int centerY = screenHeight / 2;

		// White border outline
		drawBorderBox(centerX - winWidth / 2 - 10, centerY - 22, boxWidth + 5, boxHeight, 1.0f, 1.0f, 1.0f, 1.0f, 3.0f);

		drawTextBox(centerX - boxWidth / 2 - 10, centerY - 20, boxWidth, 90, 1.0f, 1.0f, 1.0f, 0.8);
		render2dText(title, 1.0f, 1.0f, 1.0f, centerX - title.length() - 120, centerY + 50);
		render2dText("1: Level 1", 0.8f, 0.8f, 0.8f, centerX - 200, centerY + 20);
		render2dText(level2Text, 0.8f, 0.8f, 0.8f, centerX - 220 + 130, centerY + 20);
		render2dText(level3Text, 0.8f, 0.8f, 0.8f, centerX - 160 + 260, centerY + 20);
		render2dText("R: Restart            Q: Quit Game", 0.8f, 0.8f, 0.8f, centerX - 120, centerY - 10);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void resetGame()
{
	isGameOver = false;
	remainingTime = 200;
	coinsCollected = 0;
	currentLevel = 1;
	mainMenu = true;
	loadMaze("maze.txt", currentLevel);

	// Reset tank position to the center of the maze
	tankPosition.x = centerX;
	tankPosition.z = centerZ;
	tankY = 0.0f;
}

/*------------------------------------------------// Set Up Render 2d Text Function //---------------------------------------------*/
void render2dText(std::string text, float r, float g, float b, float x, float y)
{
	glColor3f(r, g, b);	 // Set text Colour
	glRasterPos2f(x, y); // Set position in window coordinates

	for (unsigned int i = 0; i < text.size(); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
	}
}
/*=================================================================================================================================*/
/*--------------------------------------------------------------// END //----------------------------------------------------------*/
/*=================================================================================================================================*/