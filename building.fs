#version 130

out vec4 fragcolor;

uniform vec3 light_position;
uniform vec3 light_intensity;

in vec3 _normal; //normal vector
in vec4 _position; //position vector

void main()
{
    //simplified lighting model 
    vec3 light = vec3(0.6, 0.0, 0.0);
    float intensity;
    intensity = _normal.x*0.5 + (1 - _normal.z)*0.5 + (light_intensity.y/100);

    if (intensity < 0.0) intensity = 0.0;
    if(diffuseshader){
        intensity = intensity * ((_normal.x * light.x) + (_normal.y * light.y) + (_normal.z * light.z));
    }
    else{
        intensity = intensity + 0.01; // ambient lighting
    }
    if(toonshader){
        light = round((clamp(light * intensity, 0.0, 1.0)) * 5) / 5;
    }
    else{
        light = clamp(light * intensity, 0.0, 1.0);
    }
    fragcolor = vec4(light, 1.0);
}