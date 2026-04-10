#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

uniform vec3 ambient_light_color;
uniform vec3 diffuse_light_color;
uniform vec3 specular_light_color;
uniform float specular_strength;
uniform float shininess;

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
	
	float spec = pow(max(dot(view_direction, reflection_direction), 0.0), shininess);
	vec3 specular = specular_strength * spec * specular_light_color;

	//final output color
	vec3 result = uColor * (ambient + diffuse + specular);

	// output to the frambuffer
	fb_color = vec4(result, 1.0);
}