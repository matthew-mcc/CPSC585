
#include <Boilerplate/Window.h>

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
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "585 Game", NULL, NULL);
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

using namespace std;


// some camera code

class KeyCallbacks : public CallbackInterface {

public:
	//KeyCallbacks(){}

	// KEYBOARD VEHICLE INPUT
	virtual void keyCallback(int key, int scancode, int action, int mods) {
		// THROTTLE (W)
		if (key == GLFW_KEY_W) {
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
		if (key == GLFW_KEY_S) {
			if (action == GLFW_PRESS) {
				brake = 1.f;
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				brake = 0.f;
				keys_pressed--;
			}
		}
		// STEER LEFT (A)
		if (key == GLFW_KEY_A) {
			if (action == GLFW_PRESS) {
				steer = 1.f;
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				steer = 0.f;
				keys_pressed--;
			}
		}
		// STEER RIGHT (D)
		if (key == GLFW_KEY_D) {
			if (action == GLFW_PRESS) {
				steer = -1.f;
				keys_pressed++;
			}
			if (action == GLFW_RELEASE) {
				steer = 0.f;
				keys_pressed--;
			}
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
		current_pos.x = (2.f / (float)xres) * xpos - 1.f;
		current_pos.y = (2.f / (float)yres) * ypos - 1.f;
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

	virtual void scrollCallback(double xoffset, double yoffset) {}

	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		std::cout << width << ' ' << height << std::endl;
		xres = width;
		yres = height;
		CallbackInterface::windowSizeCallback(width, height);

	}
	void processXbox(XboxInput x) {}

	glm::vec2 current_pos;

private:

};
std::shared_ptr<CallbackInterface> callbacks = std::make_shared<KeyCallbacks>();
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
std::shared_ptr<CallbackInterface> processInput(GLFWwindow* window) {
	//glfwSetWindowUserPointer(window, callbacks_.get());
	glfwSetKeyCallback(window, keyMetaCallback);
	//glfwSetMouseButtonCallback(window, mouseButtonMetaCallback);		// These are just commented out to work with imgui, uncomment when merging with main
	//glfwSetCursorPosCallback(window, cursorPosMetaCallback);			//
	glfwSetScrollCallback(window, scrollMetaCallback);
	glfwSetWindowSizeCallback(window, windowSizeMetaCallback);

	return callbacks;
	//if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		//glfwSetWindowShouldClose(window, true);
}
