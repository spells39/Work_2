#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shaders.h"
#include "Camera.h"
#include "stb_image.h"

#define CUBS_COUNT 2
#define SPHERE_COUNT 6



float xR = 0.001f, yR = 0.001f, zR = 0.001f;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(-1.2f, -0.25f, 3.0f));

bool firstMouse = true;
bool flagColisionOut = false;
bool startGame = false;

float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;

const int Y_segments = 50;
const int X_segments = 50;
const GLfloat PI = 3.14159265358979323846f;

float spin = 1.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

typedef struct
{
	float x, y, z;
} TCell;

typedef struct
{
	float r, g, b;
} TColor;

#define mapW 100
#define mapH 100
TCell map[mapW][mapH];
TColor mapCol[mapW][mapH];
GLuint mapInd[mapW - 1][mapH - 1][6];
int mapIndCnt = sizeof(mapInd) / sizeof(GLuint);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, glm::vec3* cubeRotation, size_t& taktFlag);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadTexture(const char* path);
void boxMove(glm::vec3& cubePositions, glm::vec3& cubeRotation, glm::vec3* cubeLightPos);
void sphereMove(glm::vec3& spherePositions, glm::vec3& sphereRotation, glm::vec3* cubeLightPos, float* sphereSize, size_t sphereNumber);
bool boxCollision(glm::vec3* cubePositions, size_t curCubeId, glm::vec3* cubeRotation, glm::vec3* spherePositions, glm::vec3* sphereRotation, float* sphereSize);
bool sphereCollision(glm::vec3* spherePositions, size_t curSphereId, glm::vec3* sphereRotation, float* sphereSize);
void ChangeDierction(glm::vec3* cubeRotation, size_t cubeNumber);
//void Map_Init();
//void Map_Show();

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Work_2", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader lightingShader("../colors.vs", "../colors.fs");
	Shader sphereShader("../colors.vs", "../colors.fs");
	Shader lampShader("../shader.vs", "../shader.fs");

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	glm::vec3 spherePositions[] = {
		glm::vec3(1.0f,  0.4f,  -0.1f),
		glm::vec3(-1.1f,  1.7f, 1.0f),
		glm::vec3(-1.0f, 2.0f, -2.5f),
		glm::vec3(-3.0f, -2.5f, 2.3f),
		glm::vec3(2.4f, 0.9f, -3.0f),
		glm::vec3(1.2f,  -3.0f, -1.9f),
		glm::vec3(0.6f, -0.7f, -2.5f),
		glm::vec3(1.1f,  2.3f, -1.2f),
		glm::vec3(0.5f,  0.2f, -1.5f),
		glm::vec3(-1.7f,  1.5f, -1.0f)
	};

	float sphereSize[] = {
		0.45f, 0.95f, 1.1f, 1.4f
	};

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  1.0f),
		glm::vec3(2.0f,  2.0f, 3.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -2.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -1.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	glm::vec3 cubeLightPos[] = {
		glm::vec3(4.5f, 3.5f, 4.0f),
		glm::vec3(-4.5f, -3.5f, -4.0f),
		glm::vec3(4.5f, 3.5f, -4.0f),
		glm::vec3(4.5f, -3.5f, -4.0f),
		glm::vec3(4.5f, -3.5f, 4.0f),
		glm::vec3(-4.5f, -3.5f, 4.0f),
		glm::vec3(-4.5f, 3.5f, -4.0f),
		glm::vec3(-4.5f, 3.5f, 4.0f)
	};

	glm::vec3 cubeRotation[] = {
		glm::vec3(0.0009f,  0.0009f,  0.0009f),
		glm::vec3(0.0009f,  -0.0009f,  -0.0009f),
		glm::vec3(0.0009f,  0.0009f,  0.0009f),
		glm::vec3(0.0009f,  0.0009f,  0.0009f),
		glm::vec3(0.0007f,  0.0007f,  0.0007f),
		glm::vec3(0.0007f,  0.0007f,  0.0007f),
		glm::vec3(0.0007f,  0.0007f,  0.0007f),
		glm::vec3(0.0007f,  0.0007f,  0.0007f),
		glm::vec3(0.0007f,  0.0007f,  0.0007f),
		glm::vec3(0.0007f,  0.0007f,  0.0007f)
	};

	glm::vec3 sphereRotation[] = {
		glm::vec3(0.0009f,  0.0009f,  -0.0009f),
		glm::vec3(-0.0009f,  -0.0009f,  0.0009f),
		glm::vec3(-0.0009f,  0.0009f,  0.0009f),
		glm::vec3(-0.0009f,  -0.0009f,  -0.0009f),
		glm::vec3(0.0009f,  0.0009f,  0.0009f),
		glm::vec3(0.0009f,  0.0009f,  0.0009f),
		glm::vec3(0.0009f,  0.0009f,  0.0009f),
		glm::vec3(0.0009f,  0.0009f,  0.0009f),
		glm::vec3(0.0009f,  0.0009f,  0.0009f),
		glm::vec3(0.0009f,  0.0009f,  0.0009f)
	};

	std::vector<float> SphereVertices;
	std::vector<int> SphereIndices;

	for (int y = 0; y <= Y_segments; y++)
		for (int x = 0; x <= X_segments; x++)
		{
			float xSegment = (float)x / (float)X_segments;
			float ySegment = (float)y / (float)Y_segments;
			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			SphereVertices.push_back(xPos);
			SphereVertices.push_back(yPos);
			SphereVertices.push_back(zPos);
		}

	for (int i = 0; i < Y_segments; i++)
		for (int j = 0; j < X_segments; j++)
		{
			SphereIndices.push_back(i * (X_segments + 1) + j);
			SphereIndices.push_back((i + 1) * (X_segments + 1) + j);
			SphereIndices.push_back((i + 1) * (X_segments + 1) + j + 1);
			SphereIndices.push_back(i * (X_segments + 1) + j);
			SphereIndices.push_back((i + 1) * (X_segments + 1) + j + 1);
			SphereIndices.push_back(i * (X_segments + 1) + j + 1);
		}

	unsigned int VBO, cubeVAO, sphereVAO, sphereVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);

	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, SphereVertices.size() * sizeof(float), &SphereVertices[0], GL_STATIC_DRAW);

	glBindVertexArray(sphereVAO);
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, SphereIndices.size() * sizeof(int), &SphereIndices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int diffuseMap = loadTexture("../textures/1-5.png");
	unsigned int specularMap = loadTexture("../textures/3-4.png");
	unsigned int specularMap1 = loadTexture("../textures/12.jpg");
	unsigned int diffuseMap1 = loadTexture("../textures/11.jpeg");
	unsigned int emissionMap = loadTexture("../textures/7.jpg");
	lightingShader.use();
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);
	sphereShader.use();
	sphereShader.setInt("material.diffuse", 0);
	sphereShader.setInt("material.specular", 1);
	//lightingShader.setInt("material.emission", 2);

	size_t taktFlag = 0;

	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) startGame = true;
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;
		processInput(window, cubeRotation, taktFlag);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lightingShader.use();
		lightingShader.setVec3("viewPos", camera.Position);
		lightingShader.setFloat("material.shininess", 32.0f);
		sphereShader.setFloat("material.shininess", 32.0f);

		lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);


		lightingShader.setVec3("pointLights[0].position", cubeLightPos[0]);
		lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[0].constant", 1.0f);
		lightingShader.setFloat("pointLights[0].linear", 0.09);
		lightingShader.setFloat("pointLights[0].quadratic", 0.032);


		lightingShader.setVec3("pointLights[1].position", cubeLightPos[1]);
		lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[1].constant", 1.0f);
		lightingShader.setFloat("pointLights[1].linear", 0.09);
		lightingShader.setFloat("pointLights[1].quadratic", 0.032);


		lightingShader.setVec3("pointLights[2].position", cubeLightPos[2]);
		lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[2].constant", 1.0f);
		lightingShader.setFloat("pointLights[2].linear", 0.09);
		lightingShader.setFloat("pointLights[2].quadratic", 0.032);


		lightingShader.setVec3("pointLights[3].position", cubeLightPos[3]);
		lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[3].constant", 1.0f);
		lightingShader.setFloat("pointLights[3].linear", 0.09);
		lightingShader.setFloat("pointLights[3].quadratic", 0.032);

		lightingShader.setVec3("pointLights[4].position", cubeLightPos[4]);
		lightingShader.setVec3("pointLights[4].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[4].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[4].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[4].constant", 1.0f);
		lightingShader.setFloat("pointLights[4].linear", 0.09);
		lightingShader.setFloat("pointLights[4].quadratic", 0.032);

		lightingShader.setVec3("pointLights[5].position", cubeLightPos[5]);
		lightingShader.setVec3("pointLights[5].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[5].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[5].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[5].constant", 1.0f);
		lightingShader.setFloat("pointLights[5].linear", 0.09);
		lightingShader.setFloat("pointLights[5].quadratic", 0.032);

		lightingShader.setVec3("pointLights[6].position", cubeLightPos[6]);
		lightingShader.setVec3("pointLights[6].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[6].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[6].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[6].constant", 1.0f);
		lightingShader.setFloat("pointLights[6].linear", 0.09);
		lightingShader.setFloat("pointLights[6].quadratic", 0.032);

		lightingShader.setVec3("pointLights[7].position", cubeLightPos[7]);
		lightingShader.setVec3("pointLights[7].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[7].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[7].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[7].constant", 1.0f);
		lightingShader.setFloat("pointLights[7].linear", 0.09);
		lightingShader.setFloat("pointLights[7].quadratic", 0.032);

		lightingShader.setVec3("spotLight.position", camera.Position);
		lightingShader.setVec3("spotLight.direction", camera.Front);
		lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("spotLight.constant", 1.0f);
		lightingShader.setFloat("spotLight.linear", 0.09);
		lightingShader.setFloat("spotLight.quadratic", 0.032);
		lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);
		glm::vec3 col(0.5f, 0.3f, 0.5f);
		lightingShader.setVec3("material.color", col);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);

		if (startGame) {
			for (int i = 0; i < CUBS_COUNT; i++) {
				boxCollision(cubePositions, i, cubeRotation, spherePositions, sphereRotation, sphereSize);
				boxMove(cubePositions[i], cubeRotation[i], cubeLightPos);
			}
			for (int i = 0; i < SPHERE_COUNT; i++) {
				sphereCollision(spherePositions, i, sphereRotation, sphereSize);
				sphereMove(spherePositions[i], sphereRotation[i], cubeLightPos, sphereSize, i);
			}
		}

		glBindVertexArray(cubeVAO);
		for (unsigned int i = 0; i < CUBS_COUNT; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * spin;

			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			lightingShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		spin += 0.0009f;

		//sphere render	
		for (int i = 0; i < SPHERE_COUNT; i++)
		{
			glm::mat4 model_sphere = glm::mat4(1.0f);
			glm::vec3 col1(1.0f, 1.0f, 1.0f);
			if (i <= 1)
				col1 = glm::vec3(1.0f, 0.5f, 0.5f);
			else if (i <= 3)
				col1 = glm::vec3(0.5f, 1.0f, 0.5f);
			else if (i <= 5)
				col1 = glm::vec3(0.5f, 0.5f, 1.0f);
			else
				col1 = glm::vec3(1.0f, 0.45f, 0.0f);
			sphereShader.setMat4("model", model_sphere);
			sphereShader.setVec3("material.color", col1);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuseMap1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, specularMap1);
			glBindVertexArray(sphereVAO);
			model_sphere = glm::mat4(1.0f);
			model_sphere = glm::translate(model_sphere, spherePositions[i]);
			if (i <= 1)
				model_sphere = glm::scale(model_sphere, glm::vec3(sphereSize[0]));
			else if (i <= 3)
				model_sphere = glm::scale(model_sphere, glm::vec3(sphereSize[1]));
			else if (i <= 5)
				model_sphere = glm::scale(model_sphere, glm::vec3(sphereSize[0]));
			else
				model_sphere = glm::scale(model_sphere, glm::vec3(sphereSize[3]));
			sphereShader.setMat4("model", model_sphere);
			//glPointSize(5);
			//glDrawElements(GL_POINTS, X_segments * Y_segments * 6, GL_UNSIGNED_INT, 0);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElements(GL_TRIANGLES, X_segments * Y_segments * 6, GL_UNSIGNED_INT, 0);
		}
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);

		glBindVertexArray(lightVAO);
		for (unsigned int i = 0; i < 8; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, cubeLightPos[i]);
			model = glm::scale(model, glm::vec3(0.2f));
			lampShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &sphereVAO);
	glDeleteBuffers(1, &sphereVBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, glm::vec3* cubeRotation, size_t& taktFlag)
{
	const float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if ((glfwGetKey(window, GLFW_KEY_1) && (taktFlag > 1100)) == GLFW_PRESS) {
		ChangeDierction(cubeRotation, 0);
		taktFlag = 0;
	}
	if ((glfwGetKey(window, GLFW_KEY_2) && (taktFlag > 1100)) == GLFW_PRESS) {
		ChangeDierction(cubeRotation, 1);
		taktFlag = 0;
	}
	taktFlag++;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}

void boxMove(glm::vec3& cubePositions, glm::vec3& cubeRotation, glm::vec3* cubeLightPos) {

	if ((cubePositions.x + 0.5f + cubeRotation.x >= cubeLightPos[0].x) || (cubePositions.x + 0.5f + cubeRotation.x <= cubeLightPos[1].x))
	{
		cubeRotation.x = -cubeRotation.x;

	}
	if ((cubePositions.y + 0.5f + cubeRotation.y >= cubeLightPos[0].y) || (cubePositions.y + 0.5f + cubeRotation.y <= cubeLightPos[1].y))
	{
		cubeRotation.y = -cubeRotation.y;
	}
	if ((cubePositions.z + 0.5f + cubeRotation.z >= cubeLightPos[0].z) || (cubePositions.z + 0.5f + cubeRotation.z <= cubeLightPos[1].z))
	{
		cubeRotation.z = -cubeRotation.z;
	}
	cubePositions.x += cubeRotation.x;
	cubePositions.y += cubeRotation.y;
	cubePositions.z += cubeRotation.z;
}

void sphereMove(glm::vec3& spherePositions, glm::vec3& sphereRotation, glm::vec3* cubeLightPos, float* sphereSize, size_t sphereNumber) {
	float rad = sphereSize[sphereNumber / 2];
	if ((spherePositions.x + rad + sphereRotation.x >= cubeLightPos[0].x) || (spherePositions.x + rad + sphereRotation.x <= cubeLightPos[1].x))
	{
		sphereRotation.x = -sphereRotation.x;

	}
	if ((spherePositions.y + rad + sphereRotation.y >= cubeLightPos[0].y) || (spherePositions.y + rad + sphereRotation.y <= cubeLightPos[1].y))
	{
		sphereRotation.y = -sphereRotation.y;
	}
	if ((spherePositions.z + rad + sphereRotation.z >= cubeLightPos[0].z) || (spherePositions.z + rad + sphereRotation.z <= cubeLightPos[1].z))
	{
		sphereRotation.z = -sphereRotation.z;
	}
	spherePositions.x += sphereRotation.x;
	spherePositions.y += sphereRotation.y;
	spherePositions.z += sphereRotation.z;
}

bool boxCollision(glm::vec3* cubePositions, size_t curCubeId, glm::vec3* cubeRotation, glm::vec3* spherePositions, glm::vec3* sphereRotation, float* sphereSize) {
	bool res = false;
	for (size_t i = curCubeId + 1; i < CUBS_COUNT; i++) {
		if (abs((cubePositions[curCubeId].x) - (cubePositions[i].x)) <= 1.0f)
			if (abs((cubePositions[curCubeId].y) - (cubePositions[i].y)) <= 1.0f)
				if (abs((cubePositions[curCubeId].z) - (cubePositions[i].z)) <= 1.0f)
				{
					ChangeDierction(cubeRotation, curCubeId);
					ChangeDierction(cubeRotation, i);
					return true;
				}
	}
	for (size_t i = 0; i < SPHERE_COUNT; i++) {
		if (abs((cubePositions[curCubeId].x) + 0.5f - (spherePositions[i].x)) <= 0.5f + sphereSize[i / 2])
			if (abs((cubePositions[curCubeId].y) + 0.5f - (spherePositions[i].y)) <= 0.5f + sphereSize[i / 2])
				if (abs((cubePositions[curCubeId].z) + 0.5f - (spherePositions[i].z)) <= 0.5f + sphereSize[i / 2])
				{
					ChangeDierction(cubeRotation, curCubeId);
					ChangeDierction(sphereRotation, i);
					return true;
				}
	}
	return false;
}

bool sphereCollision(glm::vec3* spherePositions, size_t curSphereId, glm::vec3* sphereRotation, float* sphereSize) {
	for (size_t i = curSphereId + 1; i < SPHERE_COUNT; i++) {
		if (abs((spherePositions[curSphereId].x) - (spherePositions[i].x)) <= (sphereSize[curSphereId / 2] + sphereSize[curSphereId / 2]))
			if (abs((spherePositions[curSphereId].y) - (spherePositions[i].y)) <= (sphereSize[curSphereId / 2] + sphereSize[curSphereId / 2]))
				if (abs((spherePositions[curSphereId].z) - (spherePositions[i].z)) <= (sphereSize[curSphereId / 2] + sphereSize[curSphereId / 2]))
				{
					ChangeDierction(sphereRotation, curSphereId);
					ChangeDierction(sphereRotation, i);
					return true;
				}
	}
	return false;
}

void ChangeDierction(glm::vec3* cubeRotation, size_t cubeNumber) {
	cubeRotation[cubeNumber].x = -cubeRotation[cubeNumber].x;
	cubeRotation[cubeNumber].y = -cubeRotation[cubeNumber].y;
	cubeRotation[cubeNumber].z = -cubeRotation[cubeNumber].z;
}
