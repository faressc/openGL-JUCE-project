#version 330 core


out vec4 fragColor;

void main()
{
    vec2 p = (gl_FragCoord.xy - iResolution);

    float thickness = 0.02;
    float radius = 0.5;
    float intensity = thickness/abs(radius - length(p));
    float intensity = 1.0; // debuging
    fragcolor = vec4(intensity, 0., intensity, .5);
    fragcolor = vec4(.1f, 0.f, .3f, .5f);
}

#version 330 core
out vec4 fragColor;
uniform vec2 resolution;

void main()
{
    fragColor = vec4 (0.6f, 0.1f, 1.0f, 1.f*resolution.x/1000.f);
} 
