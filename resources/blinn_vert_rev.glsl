#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 MVit;
uniform float t;

attribute vec4 aPos; // in object space
attribute vec3 aTex;

varying vec3 normal; 
varying vec3 vpos;

void main()
{
	float x = aPos.x;
	float theta = aPos.y;
	float fx = sin(.5f * x + 1 + t) + 5;
	fx = cos(x+t) + 2;
	float dfx = .5f * cos(.5f * x + 1 + t);
	dfx = -sin(x+t);
	vec3 pos = vec3(x, fx * cos(theta), fx * sin(theta));
	gl_Position = P * MV * vec4(pos, 1.0f);

	vec3 dx = vec3(1, dfx * cos(theta), dfx * sin(theta));
	vec3 dtheta = vec3(0, -fx* sin(theta), fx * cos(theta));
	normal = vec3(MVit * vec4(cross(dtheta, dx), 0.0f));
	vpos = vec3(MV * vec4(pos, 1.0f));
}
