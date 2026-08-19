#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include "shader.h"
#include "camera.h"
#include "model.h"
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>


int a = 2;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float deltaTime = 0.0;
float lastFrame = 0.0;

float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;

int frameBufferWidth = SCR_WIDTH;
int frameBufferHeight = SCR_HEIGHT;

Camera camera(glm::vec3(0.0f,0.0f,3.0f));


bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	frameBufferWidth = width;
	frameBufferHeight = height;


	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
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

	camera.ProcessMouseMovement(xoffset, yoffset, true);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}


	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}

}

int main()
{
	glfwInit();
	

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(800, 600, "GraphicsPlayground", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glm::vec3 pointLightPositions[] = 
	{
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	glEnable(GL_DEPTH_TEST);
	stbi_set_flip_vertically_on_load(true);


	Shader cubeShader("assets/shaders/3.3.shader.vert", "assets/shaders/cubeshader.frag");

	Model ourModel("assets/models/backpack/backpack.obj");

	cubeShader.use();
	cubeShader.setFloat("shininess", 64.0f);


	cubeShader.use();

	cubeShader.setVec3("pointLights[0].position", pointLightPositions[0]);
	cubeShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	cubeShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	cubeShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	cubeShader.setFloat("pointLights[0].constant", 1.0f);
	cubeShader.setFloat("pointLights[0].linear", 0.09f);
	cubeShader.setFloat("pointLights[0].quadratic", 0.032f);
	// point light 2
	cubeShader.setVec3("pointLights[1].position", pointLightPositions[1]);
	cubeShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	cubeShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	cubeShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	cubeShader.setFloat("pointLights[1].constant", 1.0f);
	cubeShader.setFloat("pointLights[1].linear", 0.09f);
	cubeShader.setFloat("pointLights[1].quadratic", 0.032f);
	// point light 3
	cubeShader.setVec3("pointLights[2].position", pointLightPositions[2]);
	cubeShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	cubeShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
	cubeShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
	cubeShader.setFloat("pointLights[2].constant", 1.0f);
	cubeShader.setFloat("pointLights[2].linear", 0.09f);
	cubeShader.setFloat("pointLights[2].quadratic", 0.032f);
	// point light 4
	cubeShader.setVec3("pointLights[3].position", pointLightPositions[3]);
	cubeShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	cubeShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
	cubeShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
	cubeShader.setFloat("pointLights[3].constant", 1.0f);
	cubeShader.setFloat("pointLights[3].linear", 0.09f);
	cubeShader.setFloat("pointLights[3].quadratic", 0.032f);


	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cubeShader.setVec3("viewPos", camera.Position);

		glm::mat4 view = camera.GetViewMatrix();
		cubeShader.setMat4("view", view);


		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(frameBufferWidth)/ static_cast<float>(frameBufferHeight), 0.1f, 100.0f);
		cubeShader.setMat4("projection", projection);


		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

		cubeShader.setMat4("model", model);


	
		ourModel.Draw(cubeShader);


		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}