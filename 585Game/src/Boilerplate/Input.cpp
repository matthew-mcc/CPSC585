// Core Includes
#include <iostream>

// 3rd Party Includes
#include <PxPhysicsAPI.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include "Input.h"
using namespace std;


// some camera code 

class Assignment3 : public CallbackInterface {

public:
	Assignment3()
	{
	}
	virtual void keyCallback(int key, int scancode, int action, int mods) {
		
		//MOVE CAMERA
		/*
		if (key == GLFW_KEY_C && action == GLFW_PRESS) {
			if (view_3D == false) {
				view_3D = true;
			}
			else {
				view_3D = false;
			}
		}*/

		if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS) && view_3D == true) {
			camera_pos += cameraSpeed * camera_front;
		}
		if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS) && view_3D == true) {
			camera_pos -= cameraSpeed * (glm::normalize(glm::cross(camera_front, camera_up)));
		}
		if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS) && view_3D == true) {
			camera_pos -= cameraSpeed * camera_front;
		}
		if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS) && view_3D == true) {
			camera_pos += cameraSpeed * (glm::normalize(glm::cross(camera_front, camera_up)));
		}

	}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			cout << "Mouse Left clicked" << endl;
		}
		
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {  
			cout << "Mouse Right clicked" << endl;
		}

	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		current_pos.x = (2.f / (float)window_w) * xpos - 1.f;
		current_pos.y = (2.f / (float)window_h) * ypos - 1.f;
		current_pos.y *= -1.f;
		//std::cout << current_pos.x << ',' << current_pos.y << std::endl;

		float xoffset = (current_pos.x - lastX) * 1000.f;
		float yoffset = (lastY - current_pos.y) * 1000.f; // reversed since y-coordinates go from bottom to top
		lastX = current_pos.x;
		lastY = current_pos.y;
		float sensitivity = 0.05f; // change this value to your liking
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;     //
		pitch -= yoffset;  // REVERSE UP/DOWN DIRECTION 
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		camera_front = glm::normalize(front);
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		fov -= (float)yoffset;
		if (fov < 1.0f)
			fov = 1.0f;
		if (fov > 45.0f)
			fov = 45.0f;
	}
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		//std::cout << width << ' ' << height << std::endl;
		window_w = width;
		window_h = height;
		CallbackInterface::windowSizeCallback(width, height);

	}

	glm::vec2 current_pos;
	bool view_3D = true;
	float cameraSpeed = 0.01f;
	int window_w = 1000;
	int window_h = 1000;
private:

};
std::shared_ptr<CallbackInterface> callbacks = std::make_shared<Assignment3>();
void keyMetaCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->keyCallback(key, scancode, action, mods);
}


void mouseButtonMetaCallback(GLFWwindow* window, int button, int action, int mods) {
	//CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->mouseButtonCallback(button, action, mods);
}


void cursorPosMetaCallback(GLFWwindow* window, double xpos, double ypos) {
	//CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->cursorPosCallback(xpos, ypos);
}


void scrollMetaCallback(GLFWwindow* window, double xoffset, double yoffset) {
	//CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->scrollCallback(xoffset, yoffset);
}


void windowSizeMetaCallback(GLFWwindow* window, int width, int height) {
	//CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->windowSizeCallback(width, height);
}

// Process Input
	// Handles GLFW window inputs
std::shared_ptr<CallbackInterface> processInput(GLFWwindow* window ) {
	//glfwSetWindowUserPointer(window, callbacks_.get());
	glfwSetKeyCallback(window, keyMetaCallback);
	glfwSetMouseButtonCallback(window, mouseButtonMetaCallback);
	glfwSetCursorPosCallback(window, cursorPosMetaCallback);
	glfwSetScrollCallback(window, scrollMetaCallback);
	glfwSetWindowSizeCallback(window, windowSizeMetaCallback);

	return callbacks;
	//if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		//glfwSetWindowShouldClose(window, true);
}

