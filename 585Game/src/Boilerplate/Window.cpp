
#include <Boilerplate/Window.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <vector>

using namespace std;


// Framebuffer Size Callback
	// Adjusts screen dimensions in response to window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}


// Initialize Window
	// Creates a GLFW window and returns a pointer to it
GLFWwindow* initWindow() {
	// Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Create window, validate creation was successful
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "Super Space Salvagers", NULL, NULL);
	if (window == NULL) {
		cout << "Failed To Create Window\n";
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Failed to initialize GLAD\n";
		return NULL;
	}

	// Return window pointer
	return window;
}


class KeyCallbacks : public CallbackInterface {
public:
	bool A = false;
	bool D = false;
	
	bool shift = false;
	bool control = false;
	// KEYBOARD VEHICLE INPUT
	virtual void keyCallback(int key, int scancode, int action, int mods) {
		// Check if Input is Enabled
		if (timer->getCountdown() > timer->getStartTime()) {
			keys_pressed = 0;
			return;
		}

		// THROTTLE (W)
		if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
			if (action == GLFW_PRESS) {
				throttle = 1.f;
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				throttle = 0.f;
				keys_pressed--;
			}
		}

		// BRAKE (S)
		if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
			if (action == GLFW_PRESS) {
				reverse = 1.f;
				brake = 1.f;
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				reverse = 0.f;
				brake = 0.f;
				keys_pressed--;
			}
		}

		// STEER LEFT (A)
		if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) {
			if (action == GLFW_PRESS ) {
				A = true;
				navigateL = true;
				steer_target = 1.f;
				AirRoll = 1.f;
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				A = false;
				navigateL = false;
				if (D == true) {
					steer_target = -1.f;
					AirRoll = -1.f;
				}
				else {
					steer_target = 0.f;
					AirRoll = 0.f;
				}
				keys_pressed--;
			}
		}

		// STEER RIGHT (D)
		if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) {
			if (action == GLFW_PRESS ) {
				D = true;
				navigateR = true;
				steer_target = -1.f;
				AirRoll = -1.f;
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				D = false;
				navigateR = false;
				if (A == true) {
					steer_target = 1.f;
					AirRoll = 1.f;
				}
				else {
					steer_target = 0.f;
					AirRoll = 0.f;
				}
				keys_pressed--;
			}
		}

		// AIR CONTROL PICH DOWN
		if (key == GLFW_KEY_LEFT_SHIFT) {
			if (action == GLFW_PRESS) {
				shift = true;
				AirPitch = 1.f;
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				shift = false;
				if (control == true)
					AirPitch = -1.f;
				else
					AirPitch = 0.f;
				keys_pressed--;
			}
		
		}

		// AIR CONTROL PITCH UP
		if (key == GLFW_KEY_LEFT_CONTROL) {
			if (action == GLFW_PRESS) {
				control = true;
				AirPitch = -1.f;
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				control = false;
				if (shift == true)
					AirPitch = 1.f;
				else
					AirPitch = 0.f;
				keys_pressed--;
			}
		}

		// BOOST
		if (key == GLFW_KEY_SPACE) {
			if (action == GLFW_PRESS) {
				boosterrrrr = true;
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				boosterrrrr = false;
				keys_pressed--;
			}
		}

		// RESET
		if (key == GLFW_KEY_R) {
			if (action == GLFW_PRESS) {
				Timer* timer = &Timer::Instance();
				reset = timer->getDeltaTime();
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				reset = 0.f;
				keys_pressed--;
			}
		}

		// DEBUG - ADD TRAILER
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
			addTrailer = true;
			audioTest = true;
		}
		if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
			horn1 = true;
		}
		if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
			horn2 = true;
		}
		if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
			horn3 = true;
		}
		if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
			horn4 = true;
		}
		if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
			horn5 = true;
		}
		if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
			horn6 = true;
		}

		// MAIN MENU CONFIRM
		if (key == GLFW_KEY_ENTER) {
			if (action == GLFW_PRESS) {
				menuConfirm = true;
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				menuConfirm = false;
				keys_pressed--;
			}
		}
		
		// BACK TO MENU
		if (key == GLFW_KEY_ESCAPE) {
			if (action == GLFW_PRESS) {
				if (gameEnded) {
					backToMenu = true;
				}
				keys_pressed++;
			}
			if (action == GLFW_RELEASE){
				backToMenu = false;
				keys_pressed--;
			}
		}

		// Ensure keys_pressed is lower-bounded to 0
		if (keys_pressed < 0) keys_pressed = 0;
	}
	
	// MOUSE BUTTON CALLBACK
	virtual void mouseButtonCallback(int button, int action, int mods) {
		auto& io = ImGui::GetIO();

		if (io.WantCaptureMouse) return;
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			//cout << "Mouse Left clicked" << endl;
			clickL = true;
			clickPos = cursor_pos;
			keys_pressed++;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			//cout << "Mouse left released" << endl;
			clickL = false;
			clickPos = glm::vec2(0.f, 0.f);
			keys_pressed--;
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			//cout << "Mouse Right clicked" << endl;
			clickR = true;
			keys_pressed++;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			//cout << "Mouse Right clicked" << endl;
			clickR = false;
			xAngle = 0.f;
			keys_pressed--;
		}
	}

	// CURSOR POSITION CALLBACK
	virtual void cursorPosCallback(double xpos, double ypos) {
		auto& io = ImGui::GetIO();

		if (io.WantCaptureMouse) return;

		cursor_pos.x = (float)((2.f / xres) * xpos - 1.f);
		cursor_pos.y = (float)((2.f / yres) * ypos - 1.f);
		cursor_pos.y *= -1.f;
		lastX = cursor_pos.x;
		lastY = cursor_pos.y;
	}

	// SCROLL CALLBACK
	virtual void scrollCallback(double xoffset, double yoffset) {}

	// WINDOW SIZE CALLBACK
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		//std::cout << width << ' ' << height << std::endl;
		xres = width;
		yres = height;
		CallbackInterface::windowSizeCallback(width, height);

	}

	void processXbox(XboxInput x) {}
	glm::vec2 cursor_pos;
};


std::vector<std::shared_ptr<CallbackInterface>> callbacks;
void keyMetaCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	for (int i = 0; i < callbacks.size(); i++) callbacks[i]->keyCallback(key, scancode, action, mods);
}


void mouseButtonMetaCallback(GLFWwindow* window, int button, int action, int mods) {
	//CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	for (int i = 0; i < callbacks.size(); i++) callbacks[i]->mouseButtonCallback(button, action, mods);
}


void cursorPosMetaCallback(GLFWwindow* window, double xpos, double ypos) {
	//CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	for (int i = 0; i < callbacks.size(); i++) callbacks[i]->cursorPosCallback(xpos, ypos);
}


void scrollMetaCallback(GLFWwindow* window, double xoffset, double yoffset) {
	//CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	for (int i = 0; i < callbacks.size(); i++) callbacks[i]->scrollCallback(xoffset, yoffset);
}


void windowSizeMetaCallback(GLFWwindow* window, int width, int height) {
	//CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	for (int i = 0; i < callbacks.size(); i++) callbacks[i]->windowSizeCallback(width, height);
}

// Process Input
	// Handles GLFW window inputs
std::shared_ptr<CallbackInterface> processInput(GLFWwindow* window, Timer* timer) {
	callbacks.push_back(std::make_shared<KeyCallbacks>());
	//glfwSetWindowUserPointer(window, callbacks_.get());
	glfwSetKeyCallback(window, keyMetaCallback);
	glfwSetMouseButtonCallback(window, mouseButtonMetaCallback);	
	glfwSetCursorPosCallback(window, cursorPosMetaCallback);
	
	glfwSetScrollCallback(window, scrollMetaCallback);
	glfwSetWindowSizeCallback(window, windowSizeMetaCallback);
	callbacks.at(callbacks.size() - 1).get()->timer = timer;
	return callbacks[callbacks.size() - 1];
	//if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		//glfwSetWindowShouldClose(window, true);
}
