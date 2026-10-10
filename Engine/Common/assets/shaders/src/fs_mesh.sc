$input v_normal

#include <bgfx_shader.sh>

void main()
{
    // Normaliza e mapeia as normais de [-1, 1] para [0, 1] (RGB)
    vec3 color = normalize(v_normal) * 0.5 + 0.5;
    gl_FragColor = vec4(color, 1.0);
}