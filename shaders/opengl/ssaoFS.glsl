#version 330 core
in vec3 bvCoord;
in vec2 bstCoord;
in vec3 bvNormal;
out vec4 FragColor;

uniform sampler2D texture0; // set up per object
uniform sampler2D depthMap; // not used
uniform sampler2D normalMap; // not used
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
// set up per scene
uniform vec3 lightPos;
uniform vec3 eyePos;
uniform vec3 inten;
uniform vec3 inten_a;
uniform vec3 RandomSample[64];
uniform vec2 screenSize;
uniform mat4 trans;

void main()
{
    vec2 uv = gl_FragCoord.xy / screenSize;
	vec3 n = normalize(texture(gNormal, uv).rgb);
    vec3 coord = texture(gPosition, uv).xyz;
    float sum = 0;
    float bias = 0.001;
    float x = coord.x;
    float y = coord.y;
    float z = coord.z;
    vec3 randomV = vec3(1.0, 1.0, 1.0);
    float bad = 0;
    int time = 32;
    for(int i = 0; i < time; i++){
        vec3 nt = normalize(randomV - n * dot(n, randomV));
        vec3 nbt = cross(n, nt);
        mat3 TBN = mat3(nt, nbt, n);
        vec3 sampleVec = normalize(TBN * RandomSample[i]);
        vec3 samplePos = coord + sampleVec * 0.1;
        vec4 ndc = trans * vec4(samplePos, 1.0);
        ndc.xyz = ndc.xyz / ndc.w;
        ndc = ndc * 0.5 + 0.5;
        float sampleDepth = texture(gPosition, ndc.xy).z;
        if(samplePos.z < sampleDepth+bias){
            float rate = time;
            sum = sum + 1.0/ rate;
        }
    }
    // FragColor = vec4(vec3(texture(gNormal, uv)), 1.0);
    // FragColor = texture(gColor, uv.xy);
    FragColor = vec4(vec3(sum), 1.0);
}