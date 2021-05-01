#version 330 core
in vec3 bvCoord;
in vec2 bstCoord;
in vec3 bvNormal;
out vec4 FragColor;

uniform sampler2D texture0;
uniform vec3 lightPos;
uniform vec3 eyePos;
uniform vec3 inten;
uniform vec3 inten_a;

void main()
{
	// bling fong shading model
	// color = specular + diffuse + ambient
	// let n = normal, l = light ray, v = view ray
	// let i = intensity, r = distance from light to spot
	// specular = K_s * i/r^2 * pow(dot(half(l, v), n), p)
	// diffuse = K_d * i/r^2 * dot(l, n)
	// ambient = K_a * i_a
	vec4 k_s = vec4(0.9, 0.9, 0.9, 1.0);
	vec4 k_d = texture(texture0, bstCoord);
	vec4 k_a = k_d;
	vec4 i = vec4(inten, 1.0);
	vec4 i_a = vec4(inten_a, 1.0);
	float p = 32.0;
	
	vec3 n = normalize(bvNormal);
	vec3 v = normalize(eyePos - bvCoord);
	vec3 l = normalize(lightPos - bvCoord);
	vec3 h = normalize(l + v);
	float r = length(lightPos - bvCoord);

	float cosine0 = dot(l, n);
	if(cosine0 < 0.0) cosine0 = 0.0;
	vec4 specular = k_s * i/r/r * pow(dot(h, n), p);
	vec4 diffuse = k_d * i/r/r * cosine0;
	vec4 ambient = k_a * i_a;
	FragColor = ambient + diffuse + specular;
	// FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}