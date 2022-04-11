#version 120

varying vec3 normal; 

void main()
{
	vec3 n = normalize(normal);
	vec3 color = 0.5 * (n + 1.0);
	gl_FragColor = vec4(color.r, color.g, color.b, 1.0);
}
