$input v_normal, v_tangent, v_uv

#include <bgfx_shader.sh>

// Nossos 3 Slots de Textura (Samplers)[cite: 1]
SAMPLER2D(s_albedo,   0);
SAMPLER2D(s_normal,   1);
SAMPLER2D(s_specular, 2);

// Nossos dados matemáticos enviados do C++[cite: 1]
uniform vec4 u_materialProps; // xyz = Base Color, w = Roughness
uniform vec4 u_dirLight;      // xyz = Light Direction, w = Intensity

void main()
{
    // 1. Amostragem de Texturas
    vec4 texColor = texture2D(s_albedo, v_uv);
    vec4 albedo = texColor * vec4(u_materialProps.xyz, 1.0);
    
    // Decodifica o Normal Map (de [0, 1] para [-1, 1])
    vec3 normalMap = texture2D(s_normal, v_uv).xyz * 2.0 - 1.0;
    
    // Amostra o Specular Map (Usamos apenas o canal R para intensidade do brilho)
    float specularIntensity = texture2D(s_specular, v_uv).r;

    // 2. Construção da Matriz TBN (Tangente, Bitangente, Normal)
    vec3 N = normalize(v_normal);
    vec3 T = normalize(v_tangent);
    
    // Ortogonaliza a Tangente (Gram-Schmidt) para evitar deformações na textura
    T = normalize(T - dot(T, N) * N);
    
    // A Bitangente é o produto vetorial entre a Normal e a Tangente
    vec3 B = cross(N, T);
    
    // Matriz de conversão do Espaço da Textura (Tangent Space) para o Espaço do Mundo
    mat3 TBN = mat3(T, B, N);

    // Perturba a Normal geométrica com os detalhes do Normal Map
    vec3 finalNormal = normalize(mul(TBN, normalMap));

    // 3. Iluminação Direcional (Lambert)
    // A luz direcional aponta para um lado. Para calcular o impacto, invertemos ela contra a normal.
    vec3 lightDir = normalize(-u_dirLight.xyz); 
    
    // Calcula o ângulo da luz (0.0 a 1.0)
    float nDotL = max(dot(finalNormal, lightDir), 0.0);
    vec3 diffuse = albedo.rgb * nDotL * u_dirLight.w;

    // 4. Especular Simples (Blinn-Phong) baseado no Roughness (Rugosidade)
    // Simula de onde a câmera está olhando (simplificado)
    vec3 viewDir = normalize(vec3(0.0, 0.0, -1.0)); 
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(finalNormal, halfDir), 0.0);
    
    // Transforma a Rugosidade (0.0 a 1.0) em uma potência (Shininess)
    float shininess = mix(100.0, 1.0, u_materialProps.w); 
    float specularFactor = pow(specAngle, shininess) * specularIntensity;
    
    vec3 specular = vec3(1.0, 1.0, 1.0) * specularFactor * u_dirLight.w; // Brilho branco da luz
    
    // Luz ambiente básica (0.1) para as áreas de sombra não ficarem 100% pretas
    vec3 ambient = albedo.rgb * 0.1;

    // 5. Cor Final
    gl_FragColor = vec4(ambient + diffuse + specular, albedo.a);
}