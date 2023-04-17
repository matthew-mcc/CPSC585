#pragma once
#include <Windows.h>
#include <glm.hpp>
#include <PxPhysicsAPI.h>
#include <iostream>
#include <Xinput.h>
#include <thread>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>				// include is here twice?
#include <Boilerplate/Timer.h>
#define _USE_MATH_DEFINES
#include <math.h>

GLFWwindow* initWindow();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
DWORD WINAPI update_controller_thread(LPVOID lpParameter);
void checkLeftThumbBar(XINPUT_STATE state, int cNum);
void checkRightThumbBar(XINPUT_STATE state, int cNum);
void checkButtons(XINPUT_STATE state, int cNum);

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
	ConStruct data[4] = {ConStruct(), ConStruct(), ConStruct(), ConStruct()};
	XboxInput();
	void run();
	void run(int numPlayers);
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
	
	float smallestSignedAngleBetween(float x, float y) {
		float a = (x - y);
		if (a > float(2 * M_PI)) a = a - float(2 * M_PI);
		else if (a < 0.f) a = a + float(2 * M_PI);
		float b = (y - x);
		if (b > float(2 * M_PI)) b = b - float(2 * M_PI);
		else if (b < 0.f) b = b + float(2 * M_PI);
		if (a < b) return -a;
		else return b;
	}

	// Mouse
	float lastX = 0.f;
	float lastY = 0.f;
	float lastX_Controller = 0.f;

	// Window Dimensions
	int xres = 1920;
	int yres = 1080;

	// Main Menu
	bool menuConfirm = false;
	bool menuConfirmHold = false;
	bool navigateU = false;
	bool navigateUHold = false;
	bool navigateD = false;
	bool navigateDHold = false;
	bool navigateR = false;
	bool navigateRHold = false;
	bool navigateL = false;
	bool navigateLHold = false;

	bool backToMenu = false;
	bool gameEnded = false;

	bool ingameMenu = false;
	bool ingameMenuHold = false;
	bool ingameMenuChange = false;

	// Vehicle Control Inputs
	int keys_pressed = 0;
	float throttle = 0.f;
	float brake = 0.f;
	float steer = 0.f;
	float steer_target = 0.f;
	float reverse = 0.f;
	float AirPitch = 0.f;
	float AirRoll = 0.f;
	float reset = 0.f;

	// Vehicle Control Parameters
	float steer_release_speed = 2.5f;			// Higher = Quicker snap back to neutral steering
	float steer_activate_speed = 2.0f;			// Higher = Quicker snap to input steer level
	float steer_speed_sensitivity = 75.f;		// Higher = More responsive at high speeds
	float min_speed_sensitivity = 0.68f;		// Higher = Less effective speed sensitivity (range [0..1])
	bool boosterrrrr = false;

	// Camera Control
	bool moveCamera = false;
	float mouseX = M_PI;
	float xAngle = M_PI;
	glm::vec2 clickPos = glm::vec2(0.f, 0.f);

	// Debug - Add Trailer
	bool addTrailer = false;
	bool audioTest = false;

	bool clickR = false;
	bool clickL = false;
	bool horn1 = false;
	bool horn2 = false;
	bool horn3 = false;
	bool horn4 = false;
	bool horn5 = false;
	bool horn6 = false;

	// Timer
	Timer* timer;

	// GAMEPAD VEHICLE INPUT
	void XboxUpdate(XboxInput x, float vehicleSpeed, bool gameEnded, int cNum) {
		// Check if Input is Enabled
		if (timer->getCountdown() > timer->getStartTime()) return;
		
		// Retrieve gameEnded Bool
		this->gameEnded = gameEnded;

		// Retrive Delta Time
		float deltaTime = (float)timer->getDeltaTime();

		// Update input variables
		if (keys_pressed <= 0) {
			throttle = x.data[cNum].RT / 255.f;
			brake = x.data[cNum].LT / 255.f;
			steer_target = -x.data[cNum].LThumb_X_direction * x.data[cNum].LThumb_magnitude;
			reverse = x.data[cNum].LT / 255.f;
			AirPitch = x.data[cNum].LThumb_Y_direction;
			AirRoll = -x.data[cNum].LThumb_X_direction;
			boosterrrrr = x.data[cNum].B;

			// Reset
			if (x.data[cNum].Y) reset = deltaTime;
			else reset = 0.f;
		}

		// Camera Look
		if (abs(x.data[cNum].RThumb_magnitude != 0.f) || clickL) {
			moveCamera = true;
			float stickAngle;
			if (clickL) {
				stickAngle = ((lastX + 1.0f) / 2.0f) * float(2 * M_PI);
			}
			else {
				stickAngle = atan2(x.data[cNum].RThumb_X_direction, x.data[cNum].RThumb_Y_direction) + M_PI;
			}

			float smallest = smallestSignedAngleBetween(stickAngle, xAngle);
			if (abs(stickAngle - xAngle) > 0.1f) {
				if (smallest > 0.01f) {
					xAngle = xAngle - abs(smallest) * 6.0f * deltaTime;
				}
				else if (smallest < -0.01f) {
					xAngle = xAngle + abs(smallest) * 6.0f * deltaTime;
				}
			}

		}
		// Camera Un-Look
		else {
			float smallest = smallestSignedAngleBetween(M_PI, xAngle);
			if (abs(M_PI - xAngle) > 0.01f) {
				if (smallest > 0.01f) {
					xAngle = xAngle - abs(smallest) * 6.0f * deltaTime;
				}
				else if (smallest < -0.01f) {
					xAngle = xAngle + abs(smallest) * 6.0f * deltaTime;
				}
			}
			else {
				xAngle = M_PI;
				moveCamera = false;
			}
		}

		// If Steer Speed is near-zero and the steering angle isn't 0, unwind the steering input
		if (abs(steer_target) <= 0.01f) {
			if (steer < -0.01f) steer = steer + steer_release_speed * deltaTime;
			else if (steer > 0.01f) steer = steer - steer_release_speed * deltaTime;
			else steer = 0.f;
		}
		// Otherwise add the steering input to steer
		else {
			float maxSteer = glm::clamp(1.0f - vehicleSpeed / steer_speed_sensitivity, min_speed_sensitivity, 1.0f);
			steer = glm::clamp(steer + steer_target * steer_activate_speed * deltaTime, -maxSteer, maxSteer);
		}

		// Main Menu
		if (keys_pressed <= 0) {
			// Menu Confirm
			if (x.data[cNum].A) {
				if (menuConfirmHold) {
					menuConfirm = false;
				}
				else {
					menuConfirm = true;
					menuConfirmHold = true;
				}
			}
			else {
				menuConfirm = false;
				menuConfirmHold = false;
			}

			// Back to Menu
			if (x.data[cNum].BACK) {
				if (gameEnded) {
					backToMenu = true;
				}
			}
			else {
				backToMenu = false;
			}

			// Menu Navigate Left
			if (x.data[cNum].LEFT) {
				if (navigateLHold) {
					navigateL = false;
				}
				else {
					navigateL = true;
					navigateLHold = true;
				}
			}
			else {
				navigateL = false;
				navigateLHold = false;
			}

			// Menu Navigate Right
			if (x.data[cNum].RIGHT) {
				if (navigateRHold) {
					navigateR = false;
				}
				else {
					navigateR = true;
					navigateRHold = true;
				}
			}
			else {
				navigateR = false;
				navigateRHold = false;
			}

			// Menu Navigate Up
			if (x.data[cNum].UP) {
				if (navigateUHold) {
					navigateU = false;
				}
				else {
					navigateU = true;
					navigateUHold = true;
				}
			}
			else {
				navigateU = false;
				navigateUHold = false;
			}

			// Menu Navigate Down
			if (x.data[cNum].DOWN) {
				if (navigateDHold) {
					navigateD = false;
				}
				else {
					navigateD = true;
					navigateDHold = true;
				}
			}
			else {
				navigateD = false;
				navigateDHold = false;
			}

			// Open Game Menu
			if (x.data[cNum].START) {
				if (!ingameMenuHold) {
					ingameMenu = !ingameMenu;
					ingameMenuHold = true;
				}
			}
			else {
				ingameMenuHold = false;
			}
		}
	}
};

std::shared_ptr<CallbackInterface> processInput(GLFWwindow* window, Timer* timer);