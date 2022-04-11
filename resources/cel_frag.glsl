#version 120

uniform vec3 light1Pos;
uniform vec3 light1Col;
uniform vec3 light2Pos;
uniform vec3 light2Col;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec3 vpos;
varying vec3 normal; 

void main()
{
	vec3 n = normalize(normal);
    vec3 e = vec3(-vpos.x, -vpos.y, -vpos.z);
    if (dot(n, e) < .3) {
        gl_FragColor = vec4(0, 0, 0, 1);
    } else {
        vec3 l1 = vec3(light1Pos.x - vpos.x, light1Pos.y - vpos.y, light1Pos.z - vpos.z);
        vec3 l2 = vec3(light2Pos.x - vpos.x, light2Pos.y - vpos.y, light2Pos.z - vpos.z);

        l1 = normalize(l1);
        l2 = normalize(l2);
        vec3 cd1 = max(0, dot(l1, n)) * kd;
        vec3 cd2 = max(0, dot(l2, n)) * kd;

        vec3 h1 = (e + l1) / (length(e + l1));
        vec3 h2 = (e + l2) / (length(e + l2));

        vec3 cs1 = ks * pow(max(0, dot(h1, n)), s);
        vec3 cs2 = ks * pow(max(0, dot(h2, n)), s);

        float r = light1Col.r * (ka.r + cs1.r + cd1.r) + light2Col.r * (ka.r + cs2.r + cd2.r);
        float g = light1Col.g * (ka.g + cs1.g + cd1.g) + light2Col.g * (ka.g + cs2.g + cd2.g);
        float b = light1Col.b * (ka.b + cs1.b + cd1.b) + light2Col.b * (ka.b + cs2.b + cd2.b);
        
        r = floor(4 * r) * .25;
        g = floor(4 * g) * .25;
        b = floor(4 * b) * .25;

        gl_FragColor = vec4(r, g, b, 1.0);
    }
}
