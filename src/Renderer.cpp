#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <filesystem>

#include "Renderer.h"

Renderer::Renderer(const char* modelDirectory) :
	modelIndex(0), rotate(0), scale(1),
	firstMouse(true), lastX(width / 2.0f), lastY(height / 2.0f),
	shiftPressed(false), deltaTime(0.0f), lastFrame(0.0f),
	lightPosition(0.0f, 0.0f, 2.0f), lightColor(1.0f, 1.0f, 1.0f),
	modelColor(0.7f, 0.0f, 0.0f), blue(0.0f, 0.0f, 1.0f), yellow(1.0f, 1.0f, 0.0f),
	coolIntensity(0.5f), warmIntensity(0.5f)
{
	initWindow();
	edgeShader = std::make_unique<Shader>("shaders/edge_vertex.glsl", "shaders/edge_fragment.glsl");
	edgeShader->link();
	goochShader = std::make_unique<Shader>("shaders/gooch_vertex.glsl", "shaders/gooch_fragment.glsl");
	goochShader->link();

	loadModels(modelDirectory);	

	perspective = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	edgeShader->use();
	edgeShader->setUniformMatrix4fv("perspective", perspective);
	edgeShader->setUniformMatrix4fv("view", camera.getViewMatrix());

	goochShader->use();
	goochShader->setUniformMatrix4fv("perspective", perspective);
	goochShader->setUniformMatrix4fv("view", camera.getViewMatrix());
	goochShader->setUniform3fv("lightPosition", lightPosition);
	goochShader->setUniform3fv("lightColor", lightColor);
	glUseProgram(0);	// unbind shader
}

Renderer::~Renderer() {}

void Renderer::initWindow()
{
	// Setup glfw context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Edge Buffer", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

	glViewport(0, 0, width, height);

	// Let GLFW store pointer to this instance of Renderer.
	glfwSetWindowUserPointer(window, static_cast<void*>(this));

	glfwSetFramebufferSizeCallback(window,
			[](GLFWwindow* window, int newWidth, int newHeight) {

		Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));

		float aspectRatio = renderer->aspectRatio;
		float viewPortHeight = (1/aspectRatio) * newWidth;
		float viewPortWidth = newWidth;
		float xPos = 0;
		float yPos = 0;

		if(viewPortHeight > newHeight)
		{
			viewPortHeight = newHeight;
			viewPortWidth = aspectRatio * newHeight;
			xPos = (newWidth - viewPortWidth) / 2.0f;	
		}
		else
		{
			yPos = (newHeight - viewPortHeight) / 2.0f;
		}

		glViewport(xPos, yPos, viewPortWidth, viewPortHeight);
	});
 
	glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glEnable(GL_DEPTH_TEST);

}

void Renderer::loadModels(const char* modelDirectory)
{
	namespace fs = std::filesystem;
	const std::string extension = ".obj";

	unsigned int count = 1;
	for (const auto& entry : fs::directory_iterator(modelDirectory))
	{
		if (entry.is_regular_file() && entry.path().extension() == extension)
		{
			std::cout << "Loading " << entry.path() << "..." << std::flush;
			models.push_back(std::make_unique<Model>(entry.path()));
			std::cout << "Done! Index: " << count << "\n";
			count++;
		}
	}
}

void Renderer::run()
{
	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT);

		processWindowInput();

		goochShader->use();
		goochShader->setUniformMatrix4fv("view", camera.getViewMatrix());
		goochShader->setUniform3fv("toCamera", camera.getPosition());
		goochShader->setUniform3fv("color", modelColor);
		goochShader->setUniform3fv("blue", blue);
		goochShader->setUniform3fv("yellow", yellow);
		goochShader->setUniform1f("coolIntensity", coolIntensity);
		goochShader->setUniform1f("warmIntensity", warmIntensity);

		models[modelIndex]->rotate(rotate);
		models[modelIndex]->scale(scale);
		models[modelIndex]->update();
		models[modelIndex]->draw(*goochShader);

		edgeShader->use();
		edgeShader->setUniformMatrix4fv("view", camera.getViewMatrix());
		edgeShader->setUniform3fv("color", glm::vec3(0.0f, 0.0f, 0.0f));

		models[modelIndex]->updateEdgeBuffer(camera.getDirection());
		models[modelIndex]->drawEdgeBuffer(*edgeShader);
		models[modelIndex]->resetEdgeBuffer();

		glUseProgram(0);

		rotate = glm::vec3(0.0f);
		scale = 1;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

/*
 * This method typically runs faster than handling a key callback.
 * So controls like movements should be placed in here.
 */
void Renderer::processWindowInput()
{
	float rotationSpeed = glm::radians(135.0f) * deltaTime;
	float scaleSpeed = 1.0f + 1.0f * deltaTime;
	shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

	// Rotations
	if (!shiftPressed)
	{
		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			rotate.x -= rotationSpeed;
		}

		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			rotate.x += rotationSpeed;
		}

		if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			rotate.y += rotationSpeed;
		}

		if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			rotate.y -= rotationSpeed;
		}

		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			rotate.z -= rotationSpeed;
		}

		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			rotate.z += rotationSpeed;
		}
	}

	// Camera Movement
	if (shiftPressed)
	{
		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			camera.processKeyboard(Camera::Movement::FORWARD, deltaTime);
		}

		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			camera.processKeyboard(Camera::Movement::BACKWARD, deltaTime);
		}

		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			camera.processKeyboard(Camera::Movement::RIGHT, deltaTime);
		}

		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			camera.processKeyboard(Camera::Movement::LEFT, deltaTime);
		}

		if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			camera.processKeyboard(Camera::Movement::UP, deltaTime);
		}

		if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			camera.processKeyboard(Camera::Movement::DOWN, deltaTime);
		}
	}

	// Scaling
	if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		scale *= scaleSpeed;
	}
	if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{
		scale /= scaleSpeed;
	}

}

/*
 * Handle keyboard inputs that don't require frequent repeated actions,
 * ex closing window, selecting model etc.
 */ 
void Renderer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
	
	if (action == GLFW_PRESS)
	{
		switch(key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, true);
				break;

			case GLFW_KEY_R:
				renderer->reset();
				break;

			// Select model
			case GLFW_KEY_0:
				renderer->modelIndex = 9;
				break;
			case GLFW_KEY_1:
			case GLFW_KEY_2:
			case GLFW_KEY_3:
			case GLFW_KEY_4:
			case GLFW_KEY_5:
			case GLFW_KEY_6:
			case GLFW_KEY_7:
			case GLFW_KEY_8:
			case GLFW_KEY_9:
				renderer->modelIndex = key - GLFW_KEY_1;
				break;
		}
	}
}

void Renderer::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));

    if (renderer->firstMouse)
    {
        renderer->lastX = xpos;
        renderer->lastY = ypos;
        renderer->firstMouse = false;
    }

    float xoffset = xpos - renderer->lastX;
    float yoffset = renderer->lastY - ypos; // reversed since y-coordinates go from bottom to top

    renderer->lastX = xpos;
    renderer->lastY = ypos;

    renderer->camera.processMouseMovement(xoffset, yoffset);
}

/**
 * Reset camera and models to default positions.
 */
void Renderer::reset()
{
	camera = Camera();

	for (auto& model : models)
	{
		model->reset();
	}
}
