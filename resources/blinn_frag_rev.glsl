#version 120

uniform vec3 lights_pos[10];
uniform vec3 lights_col[10];
uniform vec3 ke;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec3 vpos; 
varying vec3 normal;

void main()
{
	const int count = 10;
	vec3 n = normalize(normal);
	vec3 lpos;
	vec3 cd;
	vec3 cs;
	vec3 e = vec3(vpos.x, vpos.y, vpos.z);
	vec3 fragColor = ke;
	for (int i = 0; i < count; i++) {
		lpos = vec3(lights_pos[i].x - vpos.x, lights_pos[i].y - vpos.y, lights_pos[i].z - vpos.z);
		lpos = normalize(lpos);
		cd = max(0, dot(lpos, n)) * kd;
		cs = (e + lpos) / (length(e + lpos));
		cs = ks * pow(max(0, dot(cs, n)), s);
		vec3 color = lights_col[i] * (cs + cd);
		float lpos_length = length(lpos);
		float attenuation = 1.0 / (1.0 + 0.0429 * lpos_length + 0.9857 * pow(lpos_length,2));
		fragColor += color * attenuation;
	}
	gl_FragColor= vec4(fragColor.r, fragColor.g, fragColor.b, 1.0);
	//gl_FragColor = vec4(n, 1.0f);
}

