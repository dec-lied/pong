#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

#include "ScoreKeeper.h"
#include "GameObject.h"

#include <iostream>
#include <string>
#include <random>
#include <ctime>

unsigned WINDOWWIDTH = 1280;
unsigned WINDOWHEIGHT = 720;

#define CENTERLINELR 0.0075f
#define CENTERLINETB 0.035f

// timestep init
float prevTime = 0.0f, currTime, deltaTime;
float anchorTime = (float)glfwGetTime() + 3.0f;

void processInput(GLFWwindow* window, GameObject2D* lPaddle, GameObject2D* rPaddle)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		if (lPaddle->top < 1.0f)
			lPaddle->yDist += lPaddle->speedY * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		if (lPaddle->bottom > -1.0f)
			lPaddle->yDist -= lPaddle->speedY * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		if (rPaddle->top < 1.0f)
			rPaddle->yDist += rPaddle->speedY * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		if (rPaddle->bottom > -1.0f)
			rPaddle->yDist -= rPaddle->speedY * deltaTime;
}

/* return values:
0: nothing
1: left wall
2: right wall
3: ceiling / floor
4: top side of left paddle,		5: bottom side of left paddle
6: top side of right paddle,	7: bottom side of right paddle
8: top of left paddle,			9: bottom of left paddle
10: top of right paddle,		11: bottom of right paddle
*/
int checkCollisions(GameObject2D* lPaddle, GameObject2D* rPaddle, GameObject2D* ball)
{
	lPaddle->update();
	rPaddle->update();
	ball->update();

	if (ball->left <= -1.0f)
		return 1;
	else if (ball->right >= 1.0f)
		return 2;
	else if (ball->top >= 1.0f || ball->bottom <= -1.0f)
		return 3;
	else if (ball->left <= lPaddle->right + (lPaddle->width / 6.0f) && ball->right > lPaddle->left) // inside lPaddle x-range
	{
		if (ball->yDist >= lPaddle->yDist && ball->bottom < lPaddle->top)
			return 4;
		else if (ball->yDist < lPaddle->yDist && ball->top > lPaddle->bottom)
			return 5;
		else if (ball->bottom <= lPaddle->top + (lPaddle->height / 25.0f) && ball->bottom > lPaddle->top)
			return 8;
		else if (ball->top >= lPaddle->bottom - (lPaddle->height / 25.0f) && ball->top < lPaddle->bottom)
			return 9;
		else
			return 0;
	}
	else if (ball->right >= rPaddle->left - (rPaddle->width / 6.0f) && ball->left < rPaddle->right) // inside rPaddle x-range
	{
		if (ball->yDist >= rPaddle->yDist && ball->bottom < rPaddle->top)
			return 6;
		else if (ball->yDist < rPaddle->yDist && ball->top > rPaddle->bottom)
			return 7;
		else if (ball->bottom <= rPaddle->top + (rPaddle->height / 25.0f) && ball->bottom > rPaddle->top)
			return 10;
		else if (ball->top >= rPaddle->bottom - (rPaddle->height / 25.0f) && ball->top < rPaddle->bottom)
			return 11;
		else
			return 0;
	}
	else
		return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	WINDOWWIDTH = width;
	WINDOWHEIGHT = height;

	glViewport(0, 0, width, height);
}

int main()
{
	// setup
	::srand((unsigned)::time(0));
	if (!glfwInit())
	{
		std::cout << "failed to initialize glfw" << std::endl;
		return -1;
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOWWIDTH, WINDOWHEIGHT, "Pong", nullptr, nullptr);
	if (!window)
	{
		std::cout << "faild to create window" << std::endl;
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(0); // vsync

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "failed to initialize glad" << std::endl;
		return -1;
	}

	// for transparent texture backgrounds
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	stbi_set_flip_vertically_on_load(true);

	// shaders
	const char* vertShaderSource
	{ 
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aColor;\n"
		"out vec3 vertexColor;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 proj;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = proj * view * model * vec4(aPos, 1.0f);\n"
		"	vertexColor = aColor;\n"
		"}\0"
	};

	const char* fragShaderSource
	{ 
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 vertexColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = vec4(vertexColor, 1.0f);\n"
		"}\0"
	};

	const char* textVertShaderSource
	{
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aColor;\n"
		"layout (location = 2) in vec2 aTexCoord;\n"
		"out vec3 ourColor;\n"
		"out vec2 TexCoord;\n"
		"void main()\n"
		"{\n"
			"gl_Position = vec4(aPos, 1.0);\n"
			"ourColor = aColor;\n"
			"TexCoord = aTexCoord;\n"
		"}\0"
	};

	const char* textFragShaderSource
	{
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 ourColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D ourTexture;\n"
		"void main()\n"
		"{\n"
			"FragColor = texture(ourTexture, TexCoord);\n"
		"}\0"
	};

	// gameobject shaders
	GLuint vertShader, fragShader, shaderProgram = glCreateProgram();

	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertShaderSource, nullptr);
	glCompileShader(vertShader);

	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragShaderSource, nullptr);
	glCompileShader(fragShader);

	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	// texture shaders
	GLuint textVertShader, textFragShader, textShaderProgram = glCreateProgram();

	textVertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(textVertShader, 1, &textVertShaderSource, nullptr);
	glCompileShader(textVertShader);

	textFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(textFragShader, 1, &textFragShaderSource, nullptr);
	glCompileShader(textFragShader);

	glAttachShader(textShaderProgram, textVertShader);
	glAttachShader(textShaderProgram, textFragShader);
	glLinkProgram(textShaderProgram);

	glDeleteShader(textVertShader);
	glDeleteShader(textFragShader);

	// creating gameobjects
	GameObject2D lPaddle(-0.625f, 0.0f, 0.2f, 0.015f, 0.0f, 5.25f);
	GameObject2D rPaddle(0.625f, 0.0f, 0.2f, 0.015f, 0.0f, 5.25f);

	// dont want them on the stack for the entire runtime of the program. might as well just heap allocate and delete
	int* directionY = new int(rand() % 2);
	int* directionX = new int(rand() % 2);

	GameObject2D ball(0.0f, 0.0f, 0.05f, 0.025f, (directionX == 0) ? -1.4f : 1.4f, (directionY == 0) ? - 1.3f : 1.3f);

	delete directionX;
	delete directionY;

	// creating texture gameobjects for simplification
	GameObject2D lScore(-0.15f, 0.85f, 0.2f, 0.2f * (9.0f / 16.0f), 0.0f, 0.0f);
	GameObject2D rScore(0.15f, 0.85f, 0.2f, 0.2f * (9.0f / 16.0f), 0.0f, 0.0f);

	// inintializing left score texture
	int width, height, nrChannels;
	unsigned char* zero = stbi_load("scoreImages/0.png", &width, &height, &nrChannels, 0);

	GLuint lScoreText;
	glGenTextures(1, &lScoreText);

	glBindTexture(GL_TEXTURE_2D, lScoreText);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (!zero)
		std::cout << "failed to load zero (1)" << std::endl;
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);

	// inintializing right score texture
	GLuint rScoreText;
	glGenTextures(1, &rScoreText);

	glBindTexture(GL_TEXTURE_2D, rScoreText);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (!zero)
		std::cout << "failed to load zero (2)" << std::endl;
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);

	stbi_image_free(zero);

	// initializing left win text texture
	unsigned char* lText = stbi_load("scoreImages/lWin.png", &width, &height, &nrChannels, 0);

	GLuint lWinText;
	glGenTextures(1, &lWinText);

	glBindTexture(GL_TEXTURE_2D, lWinText);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (!lText)
		std::cout << "failed to load lText" << std::endl;
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, lText);

	stbi_image_free(lText);

	// initializing left win text texture
	unsigned char* rText = stbi_load("scoreImages/rWin.png", &width, &height, &nrChannels, 0);

	GLuint rWinText;
	glGenTextures(1, &rWinText);

	glBindTexture(GL_TEXTURE_2D, rWinText);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (!rText)
		std::cout << "failed to load rText" << std::endl;
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rText);

	stbi_image_free(rText);

	// gameobject arrays
	GLfloat vertices[]
	{
		lPaddle.left,	lPaddle.top,	0.0f,	1.0f, 1.0f, 1.0f, // top left
		lPaddle.right,  lPaddle.top,    0.0f,	1.0f, 1.0f, 1.0f, // top right
		lPaddle.left,	lPaddle.bottom, 0.0f,	1.0f, 1.0f, 1.0f, // bottom left
		lPaddle.right,	lPaddle.bottom, 0.0f,	1.0f, 1.0f, 1.0f, // bottom right

		rPaddle.left,	rPaddle.top,    0.0f,	1.0f, 1.0f, 1.0f, // top left
		rPaddle.right,  rPaddle.top,    0.0f,	1.0f, 1.0f, 1.0f, // top right
		rPaddle.left,	rPaddle.bottom, 0.0f,	1.0f, 1.0f, 1.0f, // bottom left
		rPaddle.right,	rPaddle.bottom, 0.0f,	1.0f, 1.0f, 1.0f, // bottom right

		ball.left,  ball.top,    0.0f,			1.0f, 1.0f, 1.0f, // top left
		ball.right, ball.top,    0.0f,			1.0f, 1.0f, 1.0f, // top right
		ball.left,  ball.bottom, 0.0f,			1.0f, 1.0f, 1.0f, // bottom left
		ball.right, ball.bottom, 0.0f,			1.0f, 1.0f, 1.0f, // bottom right

		// center lines
		// CENTERLINETB increases / reduces distance from the origin (number to left of operand)
		// padding: 0.075
		-CENTERLINELR,  0.925f - CENTERLINETB,  0.0f,			1.0f, 0.82f, 0.82f,
		 CENTERLINELR,  0.925f - CENTERLINETB,  0.0f,			1.0f, 0.82f, 0.82f,
		-CENTERLINELR,  0.725f + CENTERLINETB,  0.0f,			1.0f, 0.82f, 0.82f,
		 CENTERLINELR,  0.725f + CENTERLINETB,  0.0f,			1.0f, 0.82f, 0.82f,

		-CENTERLINELR,  0.650f - CENTERLINETB,  0.0f,			0.98f, 0.78f, 0.60f,
		 CENTERLINELR,  0.650f - CENTERLINETB,  0.0f,			0.98f, 0.78f, 0.60f,
		-CENTERLINELR,  0.450f + CENTERLINETB,  0.0f,			0.98f, 0.78f, 0.60f,
		 CENTERLINELR,  0.450f + CENTERLINETB,  0.0f,			0.98f, 0.78f, 0.60f,

		-CENTERLINELR,  0.375f - CENTERLINETB,  0.0f,			0.98f, 1.0f, 0.87f,
		 CENTERLINELR,  0.375f - CENTERLINETB,  0.0f,			0.98f, 1.0f, 0.87f,
		-CENTERLINELR,  0.175f + CENTERLINETB,  0.0f,			0.98f, 1.0f, 0.87f,
		 CENTERLINELR,  0.175f + CENTERLINETB,  0.0f,			0.98f, 1.0f, 0.87f,

		-CENTERLINELR,  0.100f - CENTERLINETB,  0.0f,			0.84f, 1.0f, 0.86f,
		 CENTERLINELR,  0.100f - CENTERLINETB,  0.0f,			0.84f, 1.0f, 0.86f,
		-CENTERLINELR, -0.100f + CENTERLINETB,  0.0f,			0.84f, 1.0f, 0.86f,
		 CENTERLINELR, -0.100f + CENTERLINETB,  0.0f,			0.84f, 1.0f, 0.86f,

		-CENTERLINELR, -0.175f - CENTERLINETB,  0.0f,			0.88f, 1.0f, 0.99f,
		 CENTERLINELR, -0.175f - CENTERLINETB,  0.0f,			0.88f, 1.0f, 0.99f,
		-CENTERLINELR, -0.375f + CENTERLINETB,  0.0f,			0.88f, 1.0f, 0.99f,
		 CENTERLINELR, -0.375f + CENTERLINETB,  0.0f,			0.88f, 1.0f, 0.99f,

		-CENTERLINELR, -0.450f - CENTERLINETB,  0.0f,			0.82f, 0.82f, 0.99f,
		 CENTERLINELR, -0.450f - CENTERLINETB,  0.0f,			0.82f, 0.82f, 0.99f,
		-CENTERLINELR, -0.650f + CENTERLINETB,  0.0f,			0.82f, 0.82f, 0.99f,
		 CENTERLINELR, -0.650f + CENTERLINETB,  0.0f,			0.82f, 0.82f, 0.99f,

		-CENTERLINELR, -0.725f - CENTERLINETB,  0.0f,			0.97f, 0.87f, 1.0f,
		 CENTERLINELR, -0.725f - CENTERLINETB,  0.0f,			0.97f, 0.87f, 1.0f,
		-CENTERLINELR, -0.925f + CENTERLINETB,  0.0f,			0.97f, 0.87f, 1.0f,
		 CENTERLINELR, -0.925f + CENTERLINETB,  0.0f,			0.97f, 0.87f, 1.0f
	};

	GLuint indices[]
	{
		// lPaddle
		0, 1, 2,
		1, 2, 3,

		// rPaddle
		4, 5, 6,
		5, 6, 7,

		// ball
		8, 9, 10,
		9, 10, 11,

		// center lines
		12, 13, 14,
		13, 14, 15,

		16, 17, 18,
		17, 18, 19,

		20, 21, 22,
		21, 22, 23,

		24, 25, 26,
		25, 26, 27,

		28, 29, 30,
		29, 30, 31,

		32, 33, 34,
		33, 34, 35,

		36, 37, 38,
		37, 38, 39
	};
	
	// score arrays
	GLfloat textVertices[]
	{
		// left score texture
		lScore.left,	lScore.top,		0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 1.0f, // tl
		lScore.right,	lScore.top,		0.0f,	1.0f, 1.0f, 1.0f,	1.0f, 1.0f, // tr
		lScore.left,	lScore.bottom,  0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, // bl
		lScore.right,	lScore.bottom,	0.0f,	1.0f, 1.0f, 1.0f,	1.0f, 0.0f, // tr

		// right score texture
		rScore.left,	rScore.top,		0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 1.0f, // tl
		rScore.right,	rScore.top,		0.0f,	1.0f, 1.0f, 1.0f,	1.0f, 1.0f, // tr
		rScore.left,	rScore.bottom,	0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, // bl
		rScore.right,	rScore.bottom,	0.0f,	1.0f, 1.0f, 1.0f,	1.0f, 0.0f,  // tr

		// left paddle win text
		-0.575f,		0.25f,			0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 1.0f, // tl
		-0.075f,		0.25f,			0.0f,	1.0f, 1.0f, 1.0f,	1.0f, 1.0f, // tr
		-0.575f,		-0.25f,			0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, // bl
		-0.075f,		-0.25f,			0.0f,	1.0f, 1.0f, 1.0f,	1.0f, 0.0f,  // tr

		// right paddle win text
		0.075f,			0.25f,			0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 1.0f, // tl
		0.575f,			0.25f,			0.0f,	1.0f, 1.0f, 1.0f,	1.0f, 1.0f, // tr
		0.075f,			-0.25f,			0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, // bl
		0.575f,			-0.25f,			0.0f,	1.0f, 1.0f, 1.0f,	1.0f, 0.0f  // tr
	};

	GLuint textIndices[]
	{
		// left score texture
		0, 1, 2,
		1, 2, 3,

		// right score texture
		4, 5, 6,
		5, 6, 7,

		// left paddle win text
		8, 9, 10,
		9, 10, 11,

		// right paddle win text
		12, 13, 14,
		13, 14, 15,
	};

	// buffer objects for vertices and indices
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0x00);
	glEnableVertexAttribArray(0);

	// colors
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// buffer objects for textVertices and textIndices
	GLuint textVAO, textVBO, textEBO;
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glGenBuffers(1, &textEBO);

	glBindVertexArray(textVAO);

	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textVertices), textVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(textIndices), textIndices, GL_STATIC_DRAW);

	// positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0x00);
	glEnableVertexAttribArray(0);

	// colors
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// texture positions
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// initializing matrices
	const glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 ballView = glm::mat4(1.0f);
	glm::mat4 lPaddleView = glm::mat4(1.0f);
	glm::mat4 rPaddleView = glm::mat4(1.0f);

	glm::mat4 proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);

	// getting matrix uniform handles
	int modelLoc = glGetUniformLocation(shaderProgram, "model");
	int viewLoc = glGetUniformLocation(shaderProgram, "view");
	int projLoc = glGetUniformLocation(shaderProgram, "proj");

	// creating scorekeeper object
	int gameState = 0;
	ScoreKeeper scoreKeeper(lScoreText, rScoreText, &currTime, &anchorTime, &gameState);

	glViewport(0, 0, WINDOWWIDTH, WINDOWHEIGHT);
	while (!glfwWindowShouldClose(window))
	{
		// timestep update
		currTime = (float)glfwGetTime();
		deltaTime = currTime - prevTime;
		prevTime = currTime;

		// game logic
		processInput(window, &lPaddle, &rPaddle);

		float scalarX = 1.0f + (((rand() % 40) + 5) * 0.01f);
		float scalarY = 1.0f + ((rand() % 20) * 0.01f);
		switch (checkCollisions(&lPaddle, &rPaddle, &ball))
		{
			case 1:
				scoreKeeper.score(false);
				ball.xDist = 0.0f;
				ball.yDist = 0.0f;
				break;
			case 2:
				scoreKeeper.score(true);
				ball.xDist = 0.0f;
				ball.yDist = 0.0f;
				break;
			case 3:
				if (ball.speedY > 0)
					ball.speedY = -::abs(ball.speedY);
				else
					ball.speedY = ::abs(ball.speedY);
				break;
			case 8:
			case 4:
				ball.speedY = ::abs(ball.rootSpeedY) * scalarY;
				ball.speedX = ::abs(ball.rootSpeedX) * scalarX;
				break;
			case 9:
			case 5:
				ball.speedY = -::abs(ball.rootSpeedY) * scalarY;
				ball.speedX = ::abs(ball.rootSpeedX) * scalarX;
				break;
			case 10:
			case 6:
				ball.speedY = ::abs(ball.rootSpeedY) * scalarY;
				ball.speedX = -::abs(ball.rootSpeedX) * scalarX;
				break;
			case 11:
			case 7:
				ball.speedY = -::abs(ball.rootSpeedY) * scalarY;
				ball.speedX = -::abs(ball.rootSpeedX) * scalarX;
				break;
		}

		if (currTime > anchorTime)
		{
			// scale ball's new position with deltatime
			ball.xDist += ball.speedX * deltaTime;
			ball.yDist += ball.speedY * deltaTime;

			gameState = 0;
		}

		// background
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// setting up score textures render
		glUseProgram(textShaderProgram);
		glBindVertexArray(textVAO);

		// lScore
		glBindTexture(GL_TEXTURE_2D, lScoreText);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0x00);

		// rScore
		glBindTexture(GL_TEXTURE_2D, rScoreText);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(GLuint)));

		if (gameState == 1) // left paddle won
		{
			// render left paddle wins text
			glBindTexture(GL_TEXTURE_2D, lWinText);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(12 * sizeof(GLuint)));
		}
		else if (gameState == 2) // right paddle won
		{
			// render right paddle wins text
			glBindTexture(GL_TEXTURE_2D, rWinText);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(18 * sizeof(GLuint)));
		}

		// setting up gameobjects render
		glUseProgram(shaderProgram);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
		glBindVertexArray(VAO);

		// updating matrices
		ballView = glm::translate(model, glm::vec3(ball.xDist, ball.yDist, 0.0f));
		lPaddleView = glm::translate(model, glm::vec3(0.0f, lPaddle.yDist, 0.0f));
		rPaddleView = glm::translate(model, glm::vec3(0.0f, rPaddle.yDist, 0.0f));

		// center lines
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 42, GL_UNSIGNED_INT, (void*)(18 * sizeof(GLuint)));

		// lPaddle
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(lPaddleView));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0x00);

		// rPaddle
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(rPaddleView));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(GLuint)));

		// ball 
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(ballView));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(12 * sizeof(GLuint)));

		// render
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	// cleanup
	glDeleteProgram(textShaderProgram);
	glDeleteProgram(shaderProgram);
	glDeleteTextures(1, &lScoreText);
	glDeleteTextures(1, &rScoreText);
	glDeleteTextures(1, &lWinText);
	glDeleteTextures(1, &rWinText);
	glDeleteVertexArrays(1, &textVAO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &textVBO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &textEBO);
	glDeleteBuffers(1, &EBO);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}