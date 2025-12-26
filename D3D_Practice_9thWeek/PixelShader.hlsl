#include <Shared.fxh>
float4 main(PS_INPUT input) : SV_TARGET
{
    // NormalMap
    float3 normal = normalize(input.norm);
    normal = normalize((2 * normal )- 1);
    
    // TBN 좌표계
    float3x3 TBN = { normalize(input.tangent.xyz), normalize(input.binormal.xyz), normalize(input.norm.xyz) };
    float3 worldNormal = normalize(input.norm);
    
    // LightVector
    float3 lightVector = normalize(-vDirectionalLight.xyz);
    
    // ViewVector
    float3 viewVector = normalize(camerapos - input.worldpos);
    
    // Diffuse
    float NdotL = max(dot(worldNormal, lightVector), 0);
    float4 Diffuse = txDiffuse.Sample(samLinear, input.Tex) * vDirectionalColor * DiffuseMaterial * NdotL;;
    Diffuse.a = 1;
    
    // Opacity
    float opacity = txOpacity.Sample(samLinear, input.Tex).a;
    
    // Ambient
    float4 Ambient = AmbientColor * AmbientMaterial;
    
    // Specular
    float3 halfvector = normalize(lightVector + viewVector); // blinn phong specular 
    float NdotH = max(dot(worldNormal, halfvector), 0);
    float specularPower = pow(NdotH, shininess) * step(0.000001, NdotL);
    float4 Specular = SpecularColor * SpecularMaterial * specularPower;
    Specular.a = 1;
    
    //alpha
    
    float4 directLight = saturate(Diffuse + Specular);
    
    //Shadow
    float currentShadowDepth = input.shadowpos.z / input.shadowpos.w;
    
    float2 uv = input.shadowpos.xy / input.shadowpos.w;
    
    uv.y = -uv.y;
    uv = uv * 0.5 + 0.5;
    
    
    if(uv.x >= 0.0f && uv.x <=1.0f && uv.y >= 0.0f && uv.y < 1.0f)
    {
        float sampleShadowDepth = txShadow.Sample(samLinear, uv).r;
        
        if(currentShadowDepth > sampleShadowDepth + 0.001f)
        {
            directLight = 0.0f;
        }
    }
    
    
    
    // 최종 출력
    float4 finalColor = 0;
    finalColor = saturate(directLight + Ambient);
    finalColor.a = 1;
    return finalColor;
}