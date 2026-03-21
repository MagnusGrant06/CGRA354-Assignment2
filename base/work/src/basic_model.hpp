
#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"


// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and/or modified for adding in extra information for drawing
// including colors for diffuse/specular, and textures for texture mapping etc.
struct basic_model {
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	glm::vec3 color{1, 0, 0};
	glm::mat4 modelTransform{1.0};

	glm::vec3 diffuse_color = glm::vec3(1.0, 0.0, 0.0);
	glm::vec3 specular_color = glm::vec3(1.0, 0.0, 0.0);
	float specular_strength = 0.5;

	void draw(const glm::mat4 &view, const glm::mat4 proj) {
		using namespace glm;

		// cacluate the modelview transform
		mat4 modelview = view * modelTransform;

		// load shader and variables
		glUseProgram(shader);
		glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
		glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
		glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));
		glUniform3f(glGetUniformLocation(shader, "diffuse_light_color"), diffuse_color.x,diffuse_color.y,diffuse_color.z);
		glUniform3f(glGetUniformLocation(shader, "specular_light_color"), specular_color.x, specular_color.y, specular_color.z);
		glUniform1f(glGetUniformLocation(shader, "specular_strength"), specular_strength);

		// draw the mesh
		mesh.draw(); 
	}
};