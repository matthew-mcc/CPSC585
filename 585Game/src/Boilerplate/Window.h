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
	bool LB; // digit ,1 or 0. Press down is 1, otherwise 0
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
	
	// Mouse
	float lastX = 0.f;
	float lastY = 0.f;
	float lastX_Controller = 0.f;

	// Window Dimensions
	int xres = 1920;
	int yres = 1080;

	// Vehicle Control Inputs
	int keys_pressed = 0;
	float throttle = 0.f;
	float brake = 0.f;
	float steer = 0.f;
	float reverse = 0.f;
	float AirPitch = 0.f;
	float AirRoll = 0.f;

	float boosterrrrr = 0.f;

	// Camera Control
	bool moveCamera = false;
	float xAngle = 0.f;
	glm::vec2 clickPos = glm::vec2(0.f, 0.f);

	// Debug - Add Trailer
	bool addTrailer = false;

	// GAMEPAD VEHICLE INPUT

	void XboxUpdate(XboxInput x) {
		if (keys_pressed <= 0) {
			throttle = x.data.RT / 255.f;
			brake = x.data.LT / 255.f;
			steer = -x.data.LThumb_X_direction;
			reverse = x.data.LT / 255.f;
			AirPitch = x.data.LThumb_Y_direction;
			AirRoll = -x.data.LThumb_X_direction;
			boosterrrrr = x.data.RB;
			//std::cout << x.data.LB <<std::endl;
		}
		if (abs(x.data.RThumb_magnitude) > 0.01f) {
			moveCamera = true;
			xAngle = atan(x.data.RThumb_X_direction / x.data.RThumb_Y_direction);
			if (x.data.RThumb_Y_direction < 0.f) xAngle = (atan(1)*4.f) + xAngle;
		}
		else if (abs(lastX_Controller) > 0.01f) {
			moveCamera = false;
		}
		lastX_Controller = x.data.RThumb_magnitude;
	}
};

std::shared_ptr<CallbackInterface> processInput(GLFWwindow* window);