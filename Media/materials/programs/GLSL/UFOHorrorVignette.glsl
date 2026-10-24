uniform sampler2D RT;
varying vec2 oUv0;

void main()
{
    vec2 p = oUv0 - vec2(0.5, 0.55);
    p.x *= 1.45;

    float vignette = smoothstep(0.34, 0.82, length(p));
    float exposure = 1.0 - 0.82 * vignette;
    vec4 scene = texture2D(RT, oUv0);
    vec3 coolShadow = vec3(0.008, 0.016, 0.04) * vignette;

    gl_FragColor = vec4(scene.rgb * exposure + coolShadow, scene.a);
}
