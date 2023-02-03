#pragma once
#include <Windows.h>
#include <glm.hpp>
#include <PxPhysicsAPI.h>
#include <iostream>
#include <Xinput.h>
#include <thread>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLFWwindow* initWindow();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
DWORD WINAPI update_controller_thread(LPVOID lpParameter);
void checkLeftThumbBar(XINPUT_STATE state);
void checkRightThumbBar(XINPUT_STATE state);
void checkButtons(XINPUT_STATE state);

struct ConStruct
{
	float LThumb_X_direction;
	float LThumb_Y_direction;
	float LThumb_magnitude;

	float RThumb_X_direction;
	float RThumb_Y_direction;
	float RThumb_magnitude;

	int LT;
	int RT;

	bool A;
	bool B;
	bool X;
	bool Y;
	bool START;
	bool BACK;
	bool UP;
	bool DOWN;
	bool LEFT;
	bool RIGHT;
	bool LB;
	bool RB;
	bool L3; //press down the left thumb bar
	bool R3; //press down the right thumb bar

};

class XboxInput {
public:
	ConStruct data;
	XboxInput();
	void run();
	void stop();
	void update();
};


class CallbackInterface {
public:
	virtual void keyCallback(int key, int scancode, int action, int mods) {}
	virtual void mouseButtonCallback(int button, int action, int mods) {}
	virtual void cursorPosCallback(double xpos, double ypos) {}
	virtual void scrollCallback(double xoffset, double yoffset) {}
	virtual void windowSizeCallback(int width, int height) { glViewport(0, 0, width, height); }
	//glm::vec3 camera_pos = glm::vec3(0.f, 0.f, 3.f);
	glm::vec3 camera_pos = glm::vec3(-3.76605, -31.4349f, 51.97f);
	// glm::vec3 camera_front = glm::vec3(0.039f, -0.361f, -0.910f);
	glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
	float cameraSpeed = 1.0f;
	float yaw = -90.f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
	float pitch = 0.f;
	float lastX = 0.f;
	float lastY = 0.f;
	float fov = 45.f;
	int xres = 1920;
	int yres = 1080;

	void XboxUpdate(XboxInput x) {
		if (x.data.LThumb_magnitude != 0) {
			camera_pos += (x.data.LThumb_Y_direction/10) * camera_front;
			camera_pos += (x.data.LThumb_X_direction/10) * (glm::normalize(glm::cross(camera_front, camera_up)));
		}

		if (x.data.RThumb_magnitude != 0) {
			yaw += x.data.RThumb_X_direction;     //
			pitch += x.data.RThumb_Y_direction;  // REVERSE UP/DOWN DIRECTION 
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

	}
};

std::shared_ptr<CallbackInterface> processInput(GLFWwindow* window);