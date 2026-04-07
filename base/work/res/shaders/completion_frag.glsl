#version 330 core
#extension GL_ARB_draw_instanced : enable

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

uniform vec3 ambient_light_color = vec3(1,0,0);
uniform vec3 diffuse_light_color = vec3(1,0,0);
uniform vec3 specular_light_color = vec3(1,0,0);
uniform float specular_strength = 0.5;
uniform vec3 colors[100];

flat in int instance_ID;
// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;
// framebuffer output
out vec4 fb_color;

void main() {
	// calculate lighting using phong model
	
	float ambient_strength = 0.8;
	vec3 ambient = ambient_strength*ambient_light_color;
	
	//diffuse part
	vec3 norm = normalize(f_in.normal);
	vec3 light_direction = normalize(-f_in.position);
	
	float diff = max(dot(norm,light_direction),0.0);

	vec3 diffuse = diff * diffuse_light_color;
	
	//specular part
	vec3 reflection_direction = reflect(-light_direction, norm);
	vec3 view_direction = normalize(-f_in.position);
	
	float spec = pow(max(dot(view_direction, reflection_direction), 0.0), 32);
	vec3 specular = specular_strength * spec * specular_light_color;

	//final output color
	vec3 result = colors[instance_ID] * (ambient + diffuse + specular);

	// output to the frambuffer
	fb_color = vec4(result, 1.0);
}