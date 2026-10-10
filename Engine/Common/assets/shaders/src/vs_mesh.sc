$input a_position, a_normal
$output v_normal

#include <bgfx_shader.sh> // Arquivo padrão do bgfx com funções utilitárias

void main()
{
    // O bgfx já gerencia as matrizes u_model[0] e u_viewProj automaticamente
    // quando você chama bgfx::setTransform e bgfx::setViewTransform no C++
    gl_Position = mul(u_viewProj, mul(u_model[0], vec4(a_position, 1.0)));
    
    // Repassa a normal para colorir o pixel
    v_normal = a_normal;
}