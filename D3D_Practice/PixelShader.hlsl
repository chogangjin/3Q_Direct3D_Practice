#include <Shared.fxh>
float4 main(PS_INPUT input) : SV_TARGET
{
    float3 lightVector = normalize(-vDirectionalLight.xyz);
    //lightVector = normalize(lightVector);
    float3 normalVector = normalize(input.norm);
    float3 viewVector = normalize(camerapos - input.worldpos);
    //float3 lightVector = viewVector; // 카메라의 위치에 따라 Directional Light 변경
    
    // Diffuse
    float NdotL = max(dot(normalVector, lightVector),0);
    float4 Diffuse = txDiffuse.Sample(samLinear, input.Tex) * vDirectionalColor * DiffuseMaterial * NdotL;
    
    //Ambient
    float4 Ambient = AmbientColor * AmbientMaterial;
    
    // Specular
    float3 halfvector = normalize(lightVector+viewVector); // blinn phong specular // view vector와 lightDirection 일치 -> halfvector = viewvector
    float NdotH = max(dot(halfvector, normalVector), 0) ;
    float specularPower = pow(NdotH, shininess) * step(0.000001, NdotL);
    float4 Specular = SpecularColor * SpecularMaterial * specularPower;
    float4 finalColor= 0;
    
    finalColor = saturate(Diffuse + Ambient + Specular);
    
    return finalColor;
}