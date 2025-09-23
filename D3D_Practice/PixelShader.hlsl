#include <Shared.fxh>
float4 main(PS_INPUT input) : SV_TARGET
{
    float4 phong = 0;
    //float3 reflected = reflect((float3) vDirectionalLight, input.norm); // phong specular
    //float RdotV = dot(reflected, input.view);
    //phong += RdotV;
    
    // Diffuse
    float3 viewVector = normalize(camerapos - input.worldpos);
    float NdotL = dot(input.norm, (float3)viewVector);
    float3 Diffuse = txDiffuse.Sample(samLinear, input.Tex) * max(NdotL, 0);
    
    //Ambient
    //float3 Ambient = 
    
    // Specular
    float3 Specularcolor = float3(1, 1, 1);
    float3 halfvector = normalize(viewVector + viewVector); // blinn phong specular
    float NdotH = dot(input.norm, halfvector);
    NdotH = max(NdotH, 0.0f);
    float3 Specular = /*Specularcolor */ NdotH;
    Specular *= pow(NdotH, shininess);
    
    phong.xyz += saturate(Diffuse+Specular);
    phong.a = 1;
    phong *= vDirectionalColor;
    
    return phong;
}