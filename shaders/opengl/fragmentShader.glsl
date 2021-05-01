#version 330 core
in vec3 bvCoord;
in vec2 bstCoord;
in vec3 bvNormal;
in mat3 TBN;
out vec4 FragColor;

uniform sampler2D texture0;
uniform sampler2D depthMap;
uniform sampler2D normalMap;
uniform sampler2D ssaoMap;
uniform sampler2D gColor;
uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform mat4 lightTrans;
uniform vec3 lightPos;
uniform vec3 eyePos;
uniform vec3 inten;
uniform vec3 inten_a;
uniform vec2 screenSize;

bool isEqual(float a,float b)
{
	return abs(a-b) < 0.0000001;
}

// return 0 if in shadow, 1 if not
float GetShadowValue(vec3 vCoord)
{
	vec4 lCoord = lightTrans * vec4(vCoord, 1.0);
	lCoord.xyz = lCoord.xyz / lCoord.w;
	lCoord = lCoord * 0.5 + 0.5;
	float sumShadow = 0.0f;
	vec2 minLength = 1.0 / textureSize(depthMap, 0);
	for(int i = -1; i <= 1; i++){
		for(int j = -1; j <= 1; j++){
			float sampleDepth = texture(depthMap, vec2(lCoord.x, lCoord.y) + vec2(i, j)*minLength).r;
			sumShadow += (sampleDepth < lCoord.z - 0.001) ? 1.0 : 0.0;
		}
	}
	return sumShadow / 9.0; 
}

float SampleShadow(vec3 vCoord)
{
	vec4 lCoord = lightTrans * vec4(vCoord, 1.0);
	lCoord.xyz = lCoord.xyz / lCoord.w;
	lCoord = lCoord * 0.5 + 0.5;
	return texture(depthMap, vec2(lCoord.x, lCoord.y)).r;
}

vec4 blingFong()
{
	vec4 k_s = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 k_d = texture(texture0, bstCoord);
	vec4 k_a = k_d;
	vec4 i = vec4(inten, 1.0);
	vec4 i_a = vec4(inten_a, 1.0);
	float p = 12.0;
	
	// normal mapping
	vec3 normal = texture(normalMap, bstCoord).rgb;
	normal = normal * 2.0 - 1.0;
	normal = normalize(TBN * normal);

	vec3 n = normalize(bvNormal);
	vec3 v = normalize(eyePos - bvCoord);
	vec3 l = normalize(lightPos - bvCoord);
	vec3 h = normalize(l + v);
	float r = length(lightPos - bvCoord);

	float cosine0 = dot(l, n);
	if(cosine0 < 0.0) cosine0 = 0.0;
	vec4 specular = k_s * i/r/r * pow(dot(h, n), p);
	vec4 diffuse = k_d * i/r/r *  cosine0;
	vec4 ambient = k_a * vec4(inten_a, 1.0);

	vec2 uv = gl_FragCoord.xy / screenSize;
	float ao = 0;
	ao = pow(ao, 2);
	ambient = ambient * (1.0 - ao);

	return (1.0 - GetShadowValue(bvCoord)) * (specular+ diffuse) + ambient;
}

void main()
{
	FragColor = blingFong();
}