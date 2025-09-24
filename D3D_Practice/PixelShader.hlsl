#include <Shared.fxh>
float4 main(PS_INPUT input) : SV_TARGET
{
    //float3 lightVector = normalize(-vDirectionalLight);
    float3 normalVector = normalize(input.norm);
    float3 viewVector = normalize(camerapos - input.worldpos);
    float3 lightVector = viewVector; // 카메라의 위치에 따라 빛이 따라다니도록 설정
    
    // Diffuse
    float NdotL = dot(normalVector, lightVector);
    float4 Diffuse = txDiffuse.Sample(samLinear, input.Tex) * vDirectionalColor * DiffuseMaterial * NdotL;
    
    //Ambient
    float4 Ambient = AmbientColor * AmbientMaterial;
    
    // Specular
    float3 halfvector = normalize(lightVector+viewVector); // blinn phong specular // view vector와 lightDirection 일치 -> halfvector = viewvector
    float NdotH = max(dot(normalVector, halfvector), 0);
    float specularPower = pow(NdotH, shininess);
    float4 Specular = SpecularColor * SpecularMaterial * specularPower;
    float4 finalColor= 0;
    
    finalColor = saturate(Diffuse + Ambient + Specular);
    
    return finalColor;
}