$input a_position, a_normal, a_tangent, a_texcoord0
$output v_normal, v_tangent, v_uv

#include <bgfx_shader.sh>

void main()
{
    // Transforma a posição do vértice para o espaço da tela
    gl_Position = mul(u_viewProj, mul(u_model[0], vec4(a_position, 1.0)));
    
    // Transforma a Normal e a Tangente para o Espaço do Mundo (World Space)
    // Usamos xyz para descartar o w (translação), pois vetores direcionais não translacionam
    v_normal  = mul(u_model[0], vec4(a_normal, 0.0)).xyz;
    v_tangent = mul(u_model[0], vec4(a_tangent, 0.0)).xyz;
    
    v_uv = a_texcoord0;
}