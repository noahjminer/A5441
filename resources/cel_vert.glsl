#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 MVit;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 normal;
varying vec3 vpos;

void main()
{
	gl_Position = P * (MV * aPos);
	vec4 camNorm = MVit * vec4(aNor.x, aNor.y, aNor.z, 0);
    vec4 camPos = MV * vec4(aPos.x, aPos.y, aPos.z, 1);
	vpos = vec3(camPos.x, camPos.y, camPos.z);
	normal = normalize(vec3(camNorm.x, camNorm.y, camNorm.z));
}
