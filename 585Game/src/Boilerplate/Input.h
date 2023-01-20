#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>


class CallbackInterface {
public:
	virtual void keyCallback(int key, int scancode, int action, int mods) {}
	virtual void mouseButtonCallback(int button, int action, int mods) {}
	virtual void cursorPosCallback(double xpos, double ypos) {}
	virtual void scrollCallback(double xoffset, double yoffset) {}
	virtual void windowSizeCallback(int width, int height) { glViewport(0, 0, width, height); }
	glm::vec3 camera_pos = glm::vec3(0.f, 0.f, 1.f);
	glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
	float yaw = -90.f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
	float pitch = 0.f;
	float lastX = 0.f;
	float lastY = 0.f;
	float fov = 45.f;
};

std::shared_ptr<CallbackInterface> processInput(GLFWwindow* window);
