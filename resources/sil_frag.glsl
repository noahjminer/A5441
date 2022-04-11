#version 120

varying vec3 vpos;
varying vec3 normal; 

void main()
{
	vec3 n = normalize(normal);
    vec3 e = vec3(-vpos.x, -vpos.y, -vpos.z);
    if (dot(n, e) < .3) {
        gl_FragColor = vec4(0, 0, 0, 1);
    } else {
        gl_FragColor = vec4(1, 1, 1, 1);
    }
}
