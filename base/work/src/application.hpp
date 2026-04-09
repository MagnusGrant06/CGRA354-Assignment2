
#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "opengl.hpp"
#include "basic_model.hpp"

#include <vector>

// Main application class
//
class Application {
private:
	// window
	glm::vec2 m_windowsize;
	GLFWwindow *m_window;

	// oribital camera
	float m_distance = 20.0;
	glm::vec2 camera_rotation = glm::vec2(0, 0);
	glm::vec3 ambient_color = glm::vec3(0.0, 0.0, 0.0);

	// drawing flags
	bool m_show_axis = false;
	bool m_show_grid = false;
	bool m_showWireframe = false;

	// basic model
	// contains a shader, a model transform
	// a mesh, and other model information (color etc.)
	basic_model m_model;
	basic_model box_model;

	//temporary GLints for getting and altering shader information
	GLint ambient_light;

	GLint core_shader;
	GLint completion_shader;
	GLint texture_shader;

	GLuint texture_object;

	int selected_mode = 0;
	bool mouse_held = false;
	bool textured = false;
	bool show_bounding = false;

public:
	// setup
	Application(GLFWwindow *);

	// disable copy constructors (for safety)
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	// rendering callbacks (every frame)
	void render();
	void renderGUI();

	//custom methods for generating textures and random transformations
	std::vector<glm::mat4> create_random_transformations();
	std::vector<glm::vec3> create_random_colors();

	basic_model create_AABB(std::vector<glm::mat4> transformations);

	// input callbacks
	void cursorPosCallback(double xpos, double ypos);
	void mouseButtonCallback(int button, int action, int mods);
	void scrollCallback(double xoffset, double yoffset);
	void keyCallback(int key, int scancode, int action, int mods);
	void charCallback(unsigned int c);
};