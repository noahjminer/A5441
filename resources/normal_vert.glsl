#version 120

uniform mat4 P;
uniform mat4 MV;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 normal;

void main()
{
	gl_Position = P * (MV * aPos);
	vec4 camNorm = MV * vec4(aNor.x, aNor.y, aNor.z, 0);
	normal = vec3(camNorm.x, camNorm.y, camNorm.z);
}
