
// std
#include <iostream>
#include <string>
#include <chrono>
#include <random>
#include <vector>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "application.hpp"
#include "bounding_box.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;


Application::Application(GLFWwindow *window) : m_window(window) {
	
	// build the shader for the model
	shader_builder color_sb;
	color_sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//default_vert.glsl"));
	color_sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//default_frag.glsl"));
	GLuint color_shader = color_sb.build();

	//build new shader for multiple instances
	shader_builder completion_sb;
	completion_sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders/completion_vert.glsl"));
	completion_sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders/completion_frag.glsl"));

	//build new shader for texture mapping
	shader_builder texture_sb;
	texture_sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders/completion_vert.glsl"));
	texture_sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders/completion_texture_frag.glsl"));

	core_shader = color_shader;
	completion_shader = completion_sb.build();
	texture_shader = texture_sb.build();

	// build the mesh for the model
	mesh_builder teapot_mb = load_wavefront_data(CGRA_SRCDIR + std::string("//res//assets//teapot.obj"));
	gl_mesh teapot_mesh = teapot_mb.build();

	// put together an object
	m_model.shader = color_shader;
	m_model.mesh = teapot_mesh;
	m_model.color = glm::vec3(1, 0, 0);
	m_model.modelTransform = glm::mat4(1);

	//load shader variables into cpp variables
	glUseProgram(color_shader);
	ambient_light = glGetUniformLocation(color_shader, "ambient_light_color");

	//create transformations array to pass to shader
	glUseProgram(completion_shader);

	std::vector<glm::mat4> transformations = create_random_transformations();
	glUniformMatrix4fv(glGetUniformLocation(completion_shader, "transformations"), transformations.size(), false, glm::value_ptr(transformations[0]));

	std::vector<glm::vec3> random_colors = create_random_colors();
	glUniform3fv(glGetUniformLocation(completion_shader, "colors"), random_colors.size(), value_ptr(random_colors[0]));

	//create first bounding box to draw
	box_model = create_AABB(transformations);

	//create texture
	cgra::rgba_image texture(CGRA_SRCDIR + std::string("//res//textures//uv_texture.jpg"));
	texture_object = texture.uploadTexture();

	glUseProgram(texture_shader);
	glUniform1i(glGetUniformLocation(texture_shader, "texture_sampler"), 0);
	glUniformMatrix4fv(glGetUniformLocation(texture_shader, "transformations"), transformations.size(), false, glm::value_ptr(transformations[0]));
}


void Application::render() {
	// retrieve the window hieght
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height); 

	m_windowsize = vec2(width, height); // update window size
	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// enable flags for normal/forward rendering
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);

	//update ambient light with user values
	glUniform3f(ambient_light, ambient_color.x, ambient_color.y, ambient_color.z);

	// calculate the projection and view matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	//calculate rotation whilst using sliders or mouse, using m_distance as radius of coordinate sphere
	float camera_x = sin(radians(camera_rotation.x)) * m_distance * cos(radians(camera_rotation.y));
	float camera_z = cos(radians(camera_rotation.x)) * m_distance * cos(radians(camera_rotation.y));
	float camera_y = sin(radians(camera_rotation.y)) * m_distance;

	mat4 camera = glm::lookAt(vec3(camera_x, camera_y, camera_z),
							  vec3(0.0f, 0.0f, 0.0f),
							  vec3(0.0f, 1.0f, 0.0f));

	//final translation to move pot to roughly middle of screen
	mat4 view = translate(camera, vec3(0.0, -5.0, 0.0));

	// draw options
	if (m_show_grid) cgra::drawGrid(view, proj);
	if (m_show_axis) cgra::drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);

	// draw the model
	if (show_bounding) {
		box_model.draw(view, proj);
	}
	m_model.draw(view, proj);

}


void Application::renderGUI() {

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
	ImGui::Begin("Camera", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.1f");
	ImGui::SliderFloat3("Model Color", value_ptr(m_model.color), 0, 1, "%.2f");
	ImGui::SliderFloat("Rotate Camera X", &camera_rotation.x, 0.0f, 360.0f);
	ImGui::SliderFloat("Rotate Camera Y", &camera_rotation.y, -89.0f, 89.0f);

	//drop down menu for choosing core, completion, and challenge
	const char* modes[] = { "Core", "Completion"};
	ImGui::Combo("Choose Part", &selected_mode, modes, sizeof(modes) / sizeof(modes[0]));

	//custom sliders for core
	if(selected_mode == 0){
		m_model.shader = core_shader;
		m_model.instancing = false;
		box_model.instancing = false;
		textured = false;
		ImGui::SliderFloat3("Ambient Light color", value_ptr(ambient_color), 0, 1, "%.2f");
		ImGui::SliderFloat3("Diffuse Light color", value_ptr(m_model.diffuse_color), 0, 1, "%.2f");
		ImGui::SliderFloat3("Specular Light color", value_ptr(m_model.specular_color), 0, 1, "%.2f");
		ImGui::SliderFloat("Specular Strength", &m_model.specular_strength, 0, 1, "%.2f");
		ImGui::SliderFloat("Shininess", &m_model.shininess, 0, 80, "%.2f");
	}

	if (selected_mode == 1) {
		ImGui::Checkbox("Textured", &textured);
		ImGui::Checkbox("Bounding Box (Challenge)", &show_bounding);
		m_model.instancing = true;
		box_model.instancing = true;
		if (!textured) {
			m_model.shader = completion_shader;
			ambient_color = glm::vec3(1, 1, 1);
			m_model.diffuse_color = glm::vec3(1, 1, 1);
			m_model.specular_color = glm::vec3(1, 1, 1);
		}
		if (textured) {
			m_model.shader = texture_shader;
			ImGui::SliderFloat3("Ambient Light color", value_ptr(ambient_color), 0, 1, "%.2f");
			ImGui::SliderFloat3("Diffuse Light color", value_ptr(m_model.diffuse_color), 0, 1, "%.2f");
			ImGui::SliderFloat3("Specular Light color", value_ptr(m_model.specular_color), 0, 1, "%.2f");
			ImGui::SliderFloat("Specular Strength", &m_model.specular_strength, 0, 1, "%.2f");
			ImGui::SliderFloat("Shininess", &m_model.shininess, 0, 80, "%.2f");

		}
	}

	// extra drawing parameters
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);

	// finish creating window
	ImGui::End();
}

// create and return a vector of random transformations using mersenne twister
std::vector<glm::mat4> Application::create_random_transformations() {
	std::vector<glm::mat4> transformations;

	std::mt19937 random(std::random_device{}());
	std::uniform_real_distribution<float> rand_pos(-50.0f, 50.0f);
	std::uniform_real_distribution<float> rand_rot(0.0f, 360.0f);
	std::uniform_real_distribution<float> rand_scal(0.2f, 2.0f);

	for (int i = 0; i < 100; i++) {
		glm::mat4 matrix = glm::translate(mat4(1.0f), glm::vec3(rand_pos(random), rand_pos(random), rand_pos(random)));
		matrix = glm::rotate(matrix, glm::radians(rand_rot(random)), glm::vec3(0, 1, 0));
		matrix = glm::rotate(matrix, glm::radians(rand_rot(random)), glm::vec3(1, 0, 0));
		matrix = glm::scale(matrix, glm::vec3(rand_scal(random), rand_scal(random), rand_scal(random)));
		transformations.push_back(matrix);
	}
	return transformations;
}

// similar to create_random_transformations except for colors
std::vector<glm::vec3> Application::create_random_colors() {
	std::vector<glm::vec3> colors;

	std::mt19937 random(std::random_device{}());
	std::uniform_real_distribution<float> rand_col(0.0f, 1.0f);

	for (int i = 0; i < 100; i++) {
		glm:vec3 temp_col = vec3(rand_col(random), rand_col(random), rand_col(random));
		colors.push_back(temp_col);
	}
	return colors;
}

// creating and returning a basic_model for the bounding box
basic_model Application::create_AABB(std::vector<glm::mat4> transformations) {
	std::vector<cgra::mesh_vertex> base_vertices = m_model.mesh.vertices;
	basic_model bounding_box;

	glm::vec3 min_pos = base_vertices[0].pos;
	glm::vec3 max_pos = base_vertices[0].pos;
	for (cgra::mesh_vertex vert : base_vertices) {  // find minimum and maximum point to create corners of box
		min_pos = min(vert.pos, min_pos);
		max_pos = max(vert.pos, max_pos);
	}

	shader_builder box_sb;
	box_sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//bounding_box_vert.glsl"));
	box_sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//bounding_box_frag.glsl"));
	
	GLuint box_shader = box_sb.build();
	cgra::gl_mesh box_mesh = createBoundingBoxMesh(min_pos, max_pos);      // use pre-defined bounding box method provided

	glUseProgram(box_shader);
	glUniformMatrix4fv(glGetUniformLocation(box_shader, "transformations"), transformations.size(), false, glm::value_ptr(transformations[0]));

	bounding_box.mesh = box_mesh;
	bounding_box.shader = box_shader;
	bounding_box.color = glm::vec3(1, 0, 0);
	bounding_box.modelTransform = glm::mat4(1);

	return bounding_box;
}

void Application::cursorPosCallback(double xpos, double ypos) {
	(void)xpos, ypos; // currently un-used
	if (!mouse_held) { return; }
	double xpos_to_rotation = (xpos / (double)m_windowsize.x) * 360.0f;
	double ypos_to_rotation = (ypos / (double)m_windowsize.y) * 360.0f;
	camera_rotation.x = xpos_to_rotation;
	camera_rotation.y = ypos_to_rotation;
	
}


void Application::mouseButtonCallback(int button, int action, int mods) {
	(void)button, action, mods; // currently un-used
	if (button == 0 && action == 1) {
		mouse_held = true;
	}
	else {
		mouse_held = false;
	}

}


void Application::scrollCallback(double xoffset, double yoffset) {
	(void)xoffset, yoffset; // currently un-used
	m_distance -= yoffset;
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}