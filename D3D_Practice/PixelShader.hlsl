#include <Shared.fxh>
float4 main(PS_INPUT input) : SV_TARGET
{
    // NormalMap
    float3 normalMap = txNormal.Sample(samLinear, input.Tex).xyz;
    normalMap = normalize((2 * normalMap )- 1);
    
    // TBN 좌표계
    float3x3 TBN = { normalize(input.tangent.xyz), normalize(input.binormal.xyz), normalize(input.norm.xyz) };
    float3 worldNormal = normalize(mul(normalMap, TBN));
    
    // LightVector
    float3 lightVector = normalize(-vDirectionalLight.xyz);
    
    // ViewVector
    float3 viewVector = normalize(camerapos - input.worldpos);
    
    // Diffuse
    float NdotL = max(dot(worldNormal, lightVector), 0);
    float4 Diffuse = txDiffuse.Sample(samLinear, input.Tex) * vDirectionalColor * DiffuseMaterial * NdotL;
    
    // Ambient
    float4 Ambient = AmbientColor * AmbientMaterial;
    
    // Specular Map
    float3 specularMap = txSpecular.Sample(samLinear, input.Tex).rgb;
    
    // Specular
    float3 halfvector = normalize(lightVector + viewVector); // blinn phong specular 
    float NdotH = max(dot(worldNormal, halfvector), 0);
    float specularPower = pow(NdotH, shininess) * step(0.000001, NdotL);
    float4 Specular = SpecularColor * SpecularMaterial * specularPower* float4(specularMap,0);
    
    // 최종 출력
    float4 finalColor = 0;
    finalColor = saturate(Diffuse + Ambient + Specular);
    
    return finalColor;
}