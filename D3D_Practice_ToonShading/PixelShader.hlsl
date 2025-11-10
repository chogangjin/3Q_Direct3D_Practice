#include <Shared.fxh>
float4 main(PS_INPUT input) : SV_TARGET
{
    // Normal
    float3 worldNormal = normalize(input.norm.xyz);
    
    // LightVector
    float3 lightVector = normalize(-vDirectionalLight.xyz);
    
    // ViewVector
    float3 viewVector = normalize(camerapos.xyz - input.worldpos.xyz);
    
    // Diffuse
    float  NdotL   = saturate(dot(worldNormal, lightVector));
    float2 RampUV  = float2(NdotL, 0.5);
    float4 RampTexture = txDiffuse.Sample(samLinear, RampUV);
    float3 Diffuse = RampTexture.rgb * (vDirectionalColor.rgb * DiffuseMaterial.rgb);
    
    // Ambient
    float3 Ambient = AmbientColor.rgb * AmbientMaterial.rgb;
    
    // Specular
    float3 Reflect = normalize(reflect(vDirectionalLight.xyz, worldNormal));
    //float3 Reflect = normalize((2.0 * NdotL * worldNormal) - lightVector);
    float RdotV = saturate(dot(Reflect, viewVector));
    float specularPower = pow(RdotV, shininess) * step(0.000001, NdotL);
    float3 Specular = SpecularColor.rgb * SpecularMaterial.rgb * specularPower;
    
    // Emissive
    //float3 Emissive = txEmissive.Sample(samLinear, input.Tex).rgb;
    
    // 최종 출력
    float3 finalColor = 0;
    finalColor = float4((Diffuse + Ambient + Specular/*+Emissive*/),1);
    //finalColor.a = 1;
    return float4(finalColor, 1);
}