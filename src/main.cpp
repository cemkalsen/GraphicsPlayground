#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include "shader.h"
#include "camera.h"
#include "model.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <map>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float deltaTime = 0.0;
float lastFrame = 0.0;

float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;

const int MIRROR_WIDTH = 240;
const int MIRROR_HEIGHT = 180;

int frameBufferWidth = SCR_WIDTH;
int frameBufferHeight = SCR_HEIGHT;

Camera camera(glm::vec3(0.0f,0.0f,3.0f));

bool firstMouse = true;
bool backpackoutline = false;
bool escPressed = false;
bool blinn = false;
bool blur = false;

unsigned int textureColorBuffer, textureColorBuffer2;
unsigned int rbo, rbo2;

float shine = 64.0f;
glm::vec3 dirLightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
glm::vec3 dirLightAmbient = glm::vec3(0.03f);
glm::vec3 dirLightDiffuse = glm::vec3(0.25f);
glm::vec3 dirLightSpecular = glm::vec3(0.3f);

float powerOfDirectional = 1.0f;

void resizeFramebufferAttachments(int width, int height)
{
	if (!height || !width)
	{
		return;
	}

	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, textureColorBuffer2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, MIRROR_WIDTH, MIRROR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo2);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, MIRROR_WIDTH, MIRROR_HEIGHT);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	frameBufferWidth = width;
	frameBufferHeight = height;


	glViewport(0, 0, width, height);
	resizeFramebufferAttachments(width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
	{
		firstMouse = true;
		return;
	}

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
		if (!escPressed)
		{
			//glfwSetWindowShouldClose(window, true);

			bool disabled = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
			glfwSetInputMode(window, GLFW_CURSOR, (disabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED));
			escPressed = true;
		}

	}
	else
		escPressed = false;


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

unsigned int loadTexture(const char* path, bool gamma = false)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data)
	{

		GLenum format;
		GLenum internalFormat;


		if (nrComponents == 1)
		{
			internalFormat = GL_RED;
			format = GL_RED;
		}
		else if (nrComponents == 3)
		{
			internalFormat = gamma ? GL_SRGB : GL_RGB;
			format = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			internalFormat = gamma ? GL_SRGB_ALPHA : GL_RGBA;
			format = GL_RGBA;
		}


		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load texture at: " << path << std::endl;

		stbi_image_free(data);
	}

	return textureID;
}

struct SceneResources
{
	Shader* sponzaShader;
	Shader* simpleShader;
	Shader* outlineShader;

	Model* sponzaModel;
	Model* backpack;

	std::vector<glm::vec3>* vegetation;
	
	unsigned int cubeTexture;
	unsigned int vegTexture;
	unsigned int floorTexture;

	unsigned int cubeVAO, planeVAO, vegVAO;

};

void RenderScene(const SceneResources& source, const glm::mat4& view, const glm::mat4& projection, const glm::vec3 viewPos)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glEnable(GL_CULL_FACE);
	glStencilMask(0xFF);
	glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);

	// SPONZA SCENE

	source.sponzaShader->use();
	source.sponzaShader->setBool("blinn", blinn);
	source.sponzaShader->setFloat("shininess", shine);
	source.sponzaShader->setVec3("dirLight.direction", dirLightDirection);
	source.sponzaShader->setVec3("dirLight.ambient", dirLightAmbient * powerOfDirectional);
	source.sponzaShader->setVec3("dirLight.diffuse", dirLightDiffuse * powerOfDirectional);
	source.sponzaShader->setVec3("dirLight.specular", dirLightSpecular * powerOfDirectional);

	source.sponzaShader->setVec3("viewPos", viewPos);

	source.sponzaShader->setMat4("view", view);
	source.sponzaShader->setMat4("projection", projection);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.02f));
	source.sponzaShader->setMat4("model", model);
	source.sponzaShader->setMat3("modelMatrix", glm::mat3(glm::transpose(glm::inverse(model))));

	source.sponzaModel->Draw(*(source.sponzaShader));


	// BACKPACK RENDER

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(5.0f, 0.6f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2f));
	source.sponzaShader->setMat4("model", model);
	source.sponzaShader->setMat3("modelMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
	source.backpack->Draw(*(source.sponzaShader));
	glStencilMask(0x00);

	// SPONZA RENDER

	source.simpleShader->use();
	source.simpleShader->setMat4("view", view);
	source.simpleShader->setMat4("projection", projection);

	glBindVertexArray(source.cubeVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, source.cubeTexture);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.5f, -1.0f));
	source.simpleShader->setMat4("model", model);

	glDrawArrays(GL_TRIANGLES, 0, 36);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.5f, 0.0f));
	source.simpleShader->setMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);


	// VEGETATION

	glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(source.vegVAO);
	glBindTexture(GL_TEXTURE_2D, source.vegTexture);


	std::map<float, glm::vec3> sorted;
	for (unsigned int i = 0; i < source.vegetation->size(); ++i)
	{
		float dist = glm::length(camera.Position - (*source.vegetation)[i]);
		sorted[-dist] = (*source.vegetation)[i];
	}

	for (auto a : sorted)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0, 0.5, 0.0) + a.second);
		source.simpleShader->setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	// PLANE RENDER
	glBindVertexArray(source.planeVAO);
	glBindTexture(GL_TEXTURE_2D, source.floorTexture);
	source.simpleShader->setMat4("model", glm::mat4(1.0f));
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//OUTLINE

	source.outlineShader->use();
	source.outlineShader->setMat4("view", view);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(5.0f, 0.6f, 0.0f));
	model = glm::scale(model, glm::vec3(0.23f));
	source.outlineShader->setMat4("model", model);

	if (backpackoutline)
	{
		source.backpack->DrawOutlined(*(source.outlineShader));
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
	glfwSwapInterval(0);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");


	glm::vec3 pointLightPositions[] =
	{
		glm::vec3(0.0f,2.0f,0.0f),
		glm::vec3(5.0f,3.0f,7.0f),
		glm::vec3(10.0f,1.0f,2.0f),
		glm::vec3(2.0f,6.0f,2.0f)
	};

	float cubeVertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		// Front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		// Left face
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		// Right face
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left     
		 // Bottom face
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		  0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		 -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		 // Top face
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		  0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		 -0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left        
	};
	float planeVertices[] =
	{
		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f,  2.0f, 2.0f

	};
	float transparentVertices[] = {
		// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
		1.0f,  0.5f,  0.0f,  1.0f,  0.0f
	};

	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	float uiQuad[] =
	{
		-0.3f, 1.0f, 0.0f, 1.0f,
		-0.3f, 0.4f, 0.0f, 0.0f,
		0.3f, 0.4f, 1.0f, 0.0f,

		-0.3f, 1.0f, 0.0f, 1.0f,
		0.3f, 0.4f, 1.0f, 0.0f,
		0.3f, 1.0f, 1.0f, 1.0f
	};

	std::vector<glm::vec3> vegetation;
	vegetation.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
	vegetation.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
	vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
	vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
	vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));


	unsigned int cubeVAO, cubeVBO;
	glGenBuffers(1, &cubeVBO);
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));

	unsigned int planeVAO, planeVBO;
	glGenBuffers(1, &planeVBO);
	glGenVertexArrays(1, &planeVAO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)( 3 * sizeof(float)));

	unsigned int vegVAO, vegVBO;
	glGenBuffers(1, &vegVBO);
	glGenVertexArrays(1, &vegVAO);
	glBindVertexArray(vegVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vegVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), &transparentVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));

	unsigned int uiVAO, uiVBO;
	glGenVertexArrays(1, &uiVAO);
	glBindVertexArray(uiVAO);
	glGenBuffers(1, &uiVBO);
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uiQuad), &uiQuad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	

	// POST PROCESSING FRAMEBUFFER INITIALIZATION

	unsigned int frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// WINDOW FRAMEBUFFER INITIALIZATLION

	unsigned int frameBuffer2;
	glGenFramebuffers(1, &frameBuffer2);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer2);

	glGenTextures(1, &textureColorBuffer2);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 240, 180, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer2, 0);

	glGenRenderbuffers(1, &rbo2);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo2);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 240, 180);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo2);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Shader sponzaShader("assets/shaders/sponzaShader.vert", "assets/shaders/sponzaShader.frag");
	Shader simpleShader("assets/shaders/simpleshader.vert", "assets/shaders/simpleshader.frag");
	Shader outlineShader("assets/shaders/outline.vert", "assets/shaders/outline.frag");
	Shader screenShader("assets/shaders/screenshader.vert", "assets/shaders/screenshader.frag");
	Shader mirrorShader("assets/shaders/mirror.vert", "assets/shaders/mirror.frag");

	Model sponzaModel("assets/models/sponza/sponza.obj");

	stbi_set_flip_vertically_on_load(true);

	Model backPack("assets/models/backpack/backpack.obj");
	
	unsigned int floorTexture = loadTexture("assets/textures/metal.png", true);
	unsigned int cubeTexture = loadTexture("assets/textures/container.jpg", true);
	unsigned int vegTexture = loadTexture("assets/textures/window.png", true);

	simpleShader.use();
	simpleShader.setInt("texture1", 0);

	screenShader.use();
	screenShader.setInt("screenTexture", 0);
	screenShader.setBool("blur", blur);

	mirrorShader.use();
	mirrorShader.setInt("mirrorTexture", 0);

	sponzaShader.use();
	sponzaShader.setFloat("shininess", 64.0f);
	sponzaShader.setBool("blinn", blinn);
	sponzaShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	sponzaShader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
	sponzaShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	sponzaShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

	sponzaShader.setVec3("pointLights[0].position", pointLightPositions[0]);
	sponzaShader.setVec3("pointLights[0].ambient", 0.01f, 0.01f, 0.01f);
	sponzaShader.setVec3("pointLights[0].diffuse", 0.25f, 0.25f, 0.25f);
	sponzaShader.setVec3("pointLights[0].specular", 0.4f, 0.4f, 0.4f);
	sponzaShader.setFloat("pointLights[0].constant", 1.0f);
	sponzaShader.setFloat("pointLights[0].linear", 0.09f);
	sponzaShader.setFloat("pointLights[0].quadratic", 0.032f);

	sponzaShader.setVec3("pointLights[1].position", pointLightPositions[1]);
	sponzaShader.setVec3("pointLights[1].ambient", 0.01f, 0.01f, 0.01f);
	sponzaShader.setVec3("pointLights[1].diffuse", 0.25f, 0.25f, 0.25f);
	sponzaShader.setVec3("pointLights[1].specular", 0.4f, 0.4f, 0.4f);
	sponzaShader.setFloat("pointLights[1].constant", 1.0f);
	sponzaShader.setFloat("pointLights[1].linear", 0.09f);
	sponzaShader.setFloat("pointLights[1].quadratic", 0.032f);

	sponzaShader.setVec3("pointLights[2].position", pointLightPositions[2]);
	sponzaShader.setVec3("pointLights[2].ambient", 0.01f, 0.01f, 0.01f);
	sponzaShader.setVec3("pointLights[2].diffuse", 0.25f, 0.25f, 0.25f);
	sponzaShader.setVec3("pointLights[2].specular", 0.4f, 0.4f, 0.4f);
	sponzaShader.setFloat("pointLights[2].constant", 1.0f);
	sponzaShader.setFloat("pointLights[2].linear", 0.09f);
	sponzaShader.setFloat("pointLights[2].quadratic", 0.032f);

	sponzaShader.setVec3("pointLights[3].position", pointLightPositions[3]);
	sponzaShader.setVec3("pointLights[3].ambient", 0.01f, 0.01f, 0.01f);
	sponzaShader.setVec3("pointLights[3].diffuse", 0.25f, 0.25f, 0.25f);
	sponzaShader.setVec3("pointLights[3].specular", 0.4f, 0.4f, 0.4f);
	sponzaShader.setFloat("pointLights[3].constant", 1.0f);
	sponzaShader.setFloat("pointLights[3].linear", 0.09f);
	sponzaShader.setFloat("pointLights[3].quadratic", 0.032f);

	while (!glfwWindowShouldClose(window))
	{

		glfwPollEvents();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);


		// IMGUI INITIALIZATION

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Renderer");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Checkbox("Backpack outline", &backpackoutline);
		ImGui::Checkbox("Blinn-Phong", &blinn);
		ImGui::Checkbox("Blur", &blur);

		ImGui::Separator();
		ImGui::Text("Directional Light");
		ImGui::SliderFloat("Power", &powerOfDirectional, 0.0f, 2.0f);
		ImGui::SliderFloat("Shininess", &shine, 0.0f, 128.0f);

		ImGui::End();

		// SCENE SETUP

		SceneResources myResources;
		myResources.backpack = &backPack;
		myResources.sponzaModel = &sponzaModel;
		myResources.sponzaShader = &sponzaShader;
		myResources.simpleShader = &simpleShader;
		myResources.outlineShader = &outlineShader;
		myResources.cubeTexture = cubeTexture;
		myResources.floorTexture = floorTexture;
		myResources.vegTexture = vegTexture;
		myResources.vegetation = &vegetation;
		myResources.cubeVAO = cubeVAO;
		myResources.planeVAO = planeVAO;
		myResources.vegVAO = vegVAO;


		// POST PROCESSING FRAMEBUFFER

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, 800, 600);

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(frameBufferWidth) / static_cast<float>(frameBufferHeight), 0.1f, 500.0f);

		RenderScene(myResources, view, projection, camera.Position);


		// WINDOW FRAMEBUFFER

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer2);
		glViewport(0, 0, 240, 180);

		glm::vec3 rearFront(-camera.Front.x, camera.Front.y, -camera.Front.z);
		glm::vec3 rearUp(-camera.Up.x, camera.Up.y, -camera.Up.z);
		view = glm::lookAt(camera.Position, camera.Position + rearFront, rearUp);
		projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(MIRROR_WIDTH) / static_cast<float>(MIRROR_HEIGHT), 0.1f, 500.0f);
		
		RenderScene(myResources, view, projection, camera.Position);
	
		// POST PROCESSING

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0,frameBufferWidth,frameBufferHeight);
		glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		screenShader.use();
		screenShader.setBool("blur", blur);
		glBindVertexArray(quadVAO);
		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// WINDOW RENDER

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		mirrorShader.use();
		glBindVertexArray(uiVAO);
		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer2);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// IMGUI RENDER

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		glfwSwapBuffers(window);
	}

	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


	glfwTerminate();
	return 0;
}