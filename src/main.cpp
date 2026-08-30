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

const unsigned int SCR_WIDTH = 800, SCR_HEIGHT = 600;
const unsigned int MIRROR_WIDTH = 240, MIRROR_HEIGHT = 180;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

float deltaTime = 0.0;
float lastFrame = 0.0;

float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;

int frameBufferWidth = SCR_WIDTH;
int frameBufferHeight = SCR_HEIGHT;

Camera camera(glm::vec3(0.0f,0.0f,3.0f));

bool firstMouse = true;
bool backpackoutline = false;
bool escPressed = false;
bool blinn = false;
bool blur = false;
bool mirrored = false;
bool showShadow = false;
int refractMode = 0;
float exposure = 2.5f;
float speedMultiplier = 1.0f;

float multiplier = 55.0f;

unsigned int textureColorBuffer, textureColorBuffer2, depthMap;
unsigned int rbo, rbo2;

float shine = 64.0f;
glm::vec3 dirLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 dirLightAmbient = glm::vec3(0.02f);
glm::vec3 dirLightDiffuse = glm::vec3(0.35f);
glm::vec3 dirLightSpecular = glm::vec3(0.35f);

float powerOfDirectional = 1.0f;

void resizeFramebufferAttachments(int width, int height)
{
	if (!height || !width)
	{
		return;
	}

	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, textureColorBuffer2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, MIRROR_WIDTH, MIRROR_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);

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

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int cubemapID;
	glGenTextures(1, &cubemapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

	int width, height, nrChannels;
	unsigned char* data;

	for (unsigned int i = 0; i < faces.size(); ++i)
	{
		data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to laod at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	return cubemapID;
}

struct SceneResources
{
	Shader* lightingShader;
	Shader* outlineShader;
	Shader* shadowShader;
	Shader* skyboxShader;
	Shader* reflectShader;

	Model* sponzaModel;
	Model* backpack;

	unsigned int skyboxVAO;
	unsigned int skyboxTexture;
	
};

void RenderShadowPass(const SceneResources& source, const glm::mat4& lightSpaceMatrix)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	// SPONZA SCENE
	source.shadowShader->use();
	source.shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.02f));
	source.shadowShader->setMat4("model", model);
	source.sponzaModel->Draw(*(source.shadowShader));

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(5.0f, 0.6f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2f));
	source.shadowShader->setMat4("model", model);
	source.backpack->Draw(*(source.shadowShader));
}

void RenderScene(const SceneResources& source, const glm::mat4& view, const glm::mat4& projection, const glm::vec3 viewPos, const glm::mat4& lightSpaceMatrix)
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

	source.lightingShader->use();
	source.lightingShader->setBool("blinn", blinn);
	source.lightingShader->setBool("showShadow", showShadow);
	source.lightingShader->setFloat("shininess", shine);
	source.lightingShader->setVec3("dirLight.direction", dirLightDirection);
	source.lightingShader->setVec3("dirLight.ambient", dirLightAmbient * powerOfDirectional);
	source.lightingShader->setVec3("dirLight.diffuse", dirLightDiffuse * powerOfDirectional);
	source.lightingShader->setVec3("dirLight.specular", dirLightSpecular * powerOfDirectional);

	source.lightingShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

	source.lightingShader->setVec3("viewPos", viewPos);

	source.lightingShader->setMat4("view", view);
	source.lightingShader->setMat4("projection", projection);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.02f));
	source.lightingShader->setMat4("model", model);
	source.lightingShader->setMat3("modelMatrix", glm::mat3(glm::transpose(glm::inverse(model))));

	glActiveTexture(GL_TEXTURE15);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	source.sponzaModel->Draw(*(source.lightingShader));


	//skybox rendering
	
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	source.skyboxShader->use();

	glm::mat4 newview = glm::mat4(glm::mat3(view));
	source.skyboxShader->setMat4("projection", projection);
	source.skyboxShader->setMat4("view", newview);

	glBindVertexArray(source.skyboxVAO);
	glActiveTexture(GL_TEXTURE16);
	glBindTexture(GL_TEXTURE_CUBE_MAP, source.skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);



	// BACKPACK RENDER

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 60.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2f));
	source.reflectShader->use();
	source.reflectShader->setMat4("model", model);
	source.reflectShader->setMat4("view", view);
	source.reflectShader->setMat4("projection", projection);
	source.reflectShader->setMat3("modelMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
	source.reflectShader->setVec3("viewPos", viewPos);
	source.reflectShader->setInt("refractMode", refractMode);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, source.skyboxTexture);
	source.backpack->Draw(*(source.reflectShader));
	glStencilMask(0x00);


	//OUTLINE

	source.outlineShader->use();
	source.outlineShader->setMat4("view", view);
	source.outlineShader->setMat4("projection", projection);

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
		glm::vec3(-1.7f,3.8f,-8.7f),
		glm::vec3(19.9f,3.7f,-0.5f),
		glm::vec3(9.8f,7.9f,-0.6f),
		glm::vec3(-15.0f,6.7f,4.0f)
	};
	float quadVertices[] =
	{
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

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

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

	unsigned int skyVAO, skyVBO;
	glGenVertexArrays(1, &skyVAO);
	glBindVertexArray(skyVAO);
	glGenBuffers(1, &skyVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));

	// POST PROCESSING FRAMEBUFFER INITIALIZATION

	unsigned int frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGB, GL_FLOAT, NULL);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 240, 180, 0, GL_RGB, GL_FLOAT, NULL);
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


	// SHADOW PASS FRAMEBUFFER INITIALIZATION

	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// CUBEMAP TEXTURE INITIALIZATION


	std::vector<std::string> faces
	{
		"assets/textures/skybox/right.jpg",
		"assets/textures/skybox/left.jpg",
		"assets/textures/skybox/top.jpg",
		"assets/textures/skybox/bottom.jpg",
		"assets/textures/skybox/front.jpg",
		"assets/textures/skybox/back.jpg",
	};

	unsigned int cubemapTexture = loadCubemap(faces);



    // SHADERS AND MODELS INITIALIZATION

	Shader lightingShader("assets/shaders/lightingShader.vert", "assets/shaders/lightingShader.frag");
	Shader outlineShader("assets/shaders/outline.vert", "assets/shaders/outline.frag");
	Shader screenShader("assets/shaders/screenshader.vert", "assets/shaders/screenshader.frag");
	Shader mirrorShader("assets/shaders/mirror.vert", "assets/shaders/mirror.frag");
	Shader shadowShader("assets/shaders/shadowShader.vert", "assets/shaders/shadowShader.frag");
	Shader skyboxShader("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
	Shader reflectShader("assets/shaders/reflect.vert", "assets/shaders/reflect.frag");
	
	Model sponzaModel("assets/models/sponza/sponza.obj");
	stbi_set_flip_vertically_on_load(true);

	Model backPack("assets/models/backpack/backpack.obj");
	
	screenShader.use();
	screenShader.setInt("screenTexture", 0);
	screenShader.setBool("blur", blur);

	mirrorShader.use();
	mirrorShader.setInt("mirrorTexture", 0);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);


	reflectShader.use();
	reflectShader.setInt("cubeMap", 16);


	lightingShader.use();
	lightingShader.setFloat("shininess", 64.0f);
	lightingShader.setBool("blinn", blinn);
	lightingShader.setInt("shadowMap", 15);

	for (int i = 0; i < 4; ++i)
	{
		std::string s = "pointLights[" + std::to_string(i) + "].";
		lightingShader.setVec3(s + "position", pointLightPositions[i]);
		lightingShader.setVec3(s + "ambient", 0.0f, 0.0f, 0.0f);
		lightingShader.setVec3(s + "diffuse", 0.0f, 0.0f, 0.0f);
		lightingShader.setVec3(s + "specular", 0.0f, 0.0f, 0.0f);
		lightingShader.setFloat(s + "constant", 1.0f);
		lightingShader.setFloat(s + "linear", 0.0f);
		lightingShader.setFloat(s + "quadratic", 1.0f);

	}

	// SCENE SETUP

	SceneResources myResources;
	myResources.backpack = &backPack;
	myResources.sponzaModel = &sponzaModel;
	myResources.lightingShader = &lightingShader;
	myResources.outlineShader = &outlineShader;
	myResources.shadowShader = &shadowShader;
	myResources.skyboxShader = &skyboxShader;
	myResources.skyboxTexture = cubemapTexture;
	myResources.skyboxVAO = skyVAO;
	myResources.reflectShader = &reflectShader;

	while (!glfwWindowShouldClose(window))
	{

		glfwPollEvents();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glm::vec3 lightDir = glm::normalize(dirLightDirection);
		glm::vec3 sceneCenter(0.0f);
		glm::vec3 lightPos = sceneCenter - lightDir * multiplier;


		// IMGUI INITIALIZATION

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Renderer");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Text("Frame time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("POSITION: X: %.1f  Y: %.1f  Z: %.1f", camera.Position.x, camera.Position.y, camera.Position.z);
		ImGui::Checkbox("Backpack outline", &backpackoutline);
		ImGui::Checkbox("Blinn-Phong", &blinn);
		ImGui::Checkbox("Blur", &blur);
		ImGui::Checkbox("Activate Mirror", &mirrored);
		
		ImGui::Separator();
		ImGui::Text("Camera");
		ImGui::SliderFloat("Camera Speed", &speedMultiplier, 1.0f, 10.0f);

		ImGui::Separator();
		ImGui::Text("Directional Light");
		ImGui::SliderFloat("Power", &powerOfDirectional, 0.0f, 5.0f);
		ImGui::SliderFloat("Shininess", &shine, 1.0f, 128.0f);
		ImGui::SliderFloat3("Direction", glm::value_ptr(dirLightDirection), -1.0f, 1.0f);
		ImGui::SliderFloat("multiplier", &multiplier, 30.0f, 100.f);
		ImGui::Checkbox("Show Shadow", &showShadow);

		ImGui::Separator();
		ImGui::Text("Backpack");
		ImGui::RadioButton("Reflect Mode", &refractMode, 0); ImGui::SameLine();
		ImGui::RadioButton("Refract Mode", &refractMode, 1);

		ImGui::Separator();
		ImGui::Text("HDR");
		ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f);


		ImGui::End();

		// SHADOW PASS
		float near_plane = 1.0f, far_plane = 80.0f;
		float originalSpeed = 2.5f;
		camera.MovementSpeed = originalSpeed * speedMultiplier;

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_CULL_FACE);
	
		//dirLightDirection = glm::vec3(dirLightDirection.x, sin(glfwGetTime()) * 0.5f, cos(glfwGetTime()) * 0.5f);



		glm::mat4 lightProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(lightPos,sceneCenter, glm::vec3(0.0, 1.0, 0.0));

		glm::mat4 lightSpace = lightProjection * lightView;

		RenderShadowPass(myResources, lightSpace);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// POST PROCESSING FRAMEBUFFER
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		lightingShader.use();
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, frameBufferWidth, frameBufferHeight);

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(frameBufferWidth) / static_cast<float>(frameBufferHeight), 0.1f, 500.0f);

		RenderScene(myResources, view, projection, camera.Position, lightSpace);


		// MIRROR PASS

		if (mirrored)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer2);
			glViewport(0, 0, 240, 180);

			glm::vec3 rearFront(-camera.Front.x, camera.Front.y, -camera.Front.z);
			glm::vec3 rearUp(-camera.Up.x, camera.Up.y, -camera.Up.z);
			view = glm::lookAt(camera.Position, camera.Position + rearFront, rearUp);
			projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(MIRROR_WIDTH) / static_cast<float>(MIRROR_HEIGHT), 0.1f, 500.0f);

			RenderScene(myResources, view, projection, camera.Position, lightSpace);
		}

		// POST PROCESSING

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0,frameBufferWidth,frameBufferHeight);
		glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		screenShader.use();
		screenShader.setBool("blur", blur);
		screenShader.setFloat("exposure", exposure);
		screenShader.setFloat("far_plane", far_plane);
		screenShader.setFloat("near_plane", near_plane);
		glBindVertexArray(quadVAO);
		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		//glBindTexture(GL_TEXTURE_2D, depthMap);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		// WINDOW RENDER

		if(mirrored)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);	

			mirrorShader.use();
			mirrorShader.setFloat("exposure", exposure);
			glBindVertexArray(uiVAO);
			glDisable(GL_DEPTH_TEST);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureColorBuffer2);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}


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