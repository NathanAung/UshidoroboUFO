uniform sampler2D RT;
varying vec2 oUv0;

void main()
{
    vec2 p = oUv0 - vec2(0.5, 0.55);
    p.x *= 1.45;

    float vignette = smoothstep(0.34, 0.82, length(p));
    float exposure = 1.0 - 0.68 * vignette;
    vec4 scene = texture2D(RT, oUv0);
    vec3 coolShadow = vec3(0.008, 0.018, 0.085) * vignette;
    vec3 nightGrade = scene.rgb * vec3(0.86, 0.93, 1.12);
    vec3 blueAtmosphere = vec3(0.012, 0.02, 0.04);

    gl_FragColor = vec4(
        (nightGrade + blueAtmosphere) * exposure + coolShadow,
        scene.a);
}
