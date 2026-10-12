$input v_normal, v_uv

#include <bgfx_shader.sh>

// O sampler que você criou no Material.hpp
SAMPLER2D(s_texColor, 0);

void main()
{
    // Extrai a cor do pixel baseada na coordenada UV
    vec4 texColor = texture2D(s_texColor, v_uv);

    // Iluminação direcional simples para não ficar chapado
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));
    float diffuse = max(dot(normalize(v_normal), lightDir), 0.2); 

    gl_FragColor = vec4(texColor.rgb * diffuse, texColor.a);
}