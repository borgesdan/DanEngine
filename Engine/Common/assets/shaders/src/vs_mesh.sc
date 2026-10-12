$input a_position, a_normal, a_texcoord0
$output v_normal, v_uv

#include <bgfx_shader.sh>

void main()
{
    gl_Position = mul(u_viewProj, mul(u_model[0], vec4(a_position, 1.0)));
    
    v_normal = a_normal;
    
    // Repassa a coordenada de textura exata para o Fragment Shader
    v_uv = a_texcoord0; 
}