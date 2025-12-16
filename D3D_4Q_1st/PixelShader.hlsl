#include <Shared.fxh>


static const float PI = 3.14159265f;
static const float EPSILON = 1e-6f;
// Fresnel Shlick reflection
float3 Fresnel_Shlick(in float3 f0 , in float x ) // F
{
    //float3 f90 = float3(1, 1, 1) * metalness + 0.4f * (1 - metalness);
    return f0 + (1 - f0) * pow(1 - x, 5.0f);
}

float Distribution_GGX(in float ndoth, float roughness) // D
{
    float alpha = roughness * roughness;
    float a2 = alpha * alpha;
    float devideFactor = max(EPSILON, (ndoth * ndoth) * (a2 - 1) + 1);
    
    return a2 / (PI * devideFactor * devideFactor);
}

float GeometricAttenuation_GGX(float dot, float roughness)
{
    float alpha = roughness * roughness;
    float k = (alpha + 1.0f) * (alpha + 1.0f) / 8.0f;
    return dot / (dot * (1 - k) + k);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    //Diffuse
    float4 DiffuseMap = txDiffuse.Sample(samLinear, input.Tex);
    float3 albedo = DiffuseMap.rgb ;
    if (overridematerial)
    {
        albedo = DiffuseColor.rgb;
    }
    
    
    float3x3 TBN = { input.tangent.rgb, input.binormal.rgb, input.norm.rgb };
    //NormalMap
    float3 normal = txNormal.Sample(samLinear, input.Tex).rgb;
    normal = normalize(2.0f * normal - 1.0f);
    
    //Metalic
    float metalic = txMetallic.Sample(samLinear, input.Tex).r;
    if(overridematerial)
    {
        metalic = metalness;
    }
    
    //Roughness
    float Rough = txRoughness.Sample(samLinear, input.Tex).r;
    if (overridematerial)
    {
        Rough = max(roughness, 0.01f);
    }
    
    // vector
    float3 lightVector  = normalize(-vDirectionalLight.xyz);
    float3 normalVector = normalize(mul(normal, TBN));
    float3 viewVector   = normalize(camerapos - input.worldpos);
    float3 halfvector   = normalize(lightVector + viewVector);
    
    ////dot product
    float NdotH = max(dot(normalVector, halfvector), 0);
    float NdotV = max(dot(normalVector, viewVector), 0);
    float LdotH = max(dot(lightVector, halfvector), 0);
    float NdotL = max(dot(normalVector, lightVector), 0);
    float VdotH = max(dot(viewVector, halfvector), 0);
    
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), DiffuseColor.rgb, metalic);
    
    float  D = Distribution_GGX(NdotH, Rough);
    float3 F = Fresnel_Shlick(F0, VdotH);
    float  G = GeometricAttenuation_GGX(NdotV, Rough) * GeometricAttenuation_GGX(NdotL, Rough);
    
    float3 specularBRDF = D * F * G / ((4 * NdotL * NdotV) + 0.001);
    
    float3 KS = F0;
    //float3 KD = (1 - KS) * (1 - metalness);
    float3 KD = (1 - KS) * (1 - metalic);
    float3 diffuse = KD * ((float3) albedo) / PI;
    //float3 diffuse = KD * ((float3) DiffuseColor) / PI;
    float3 directLight = (diffuse + specularBRDF) * vDirectionalColor.rgb * saturate(NdotL);
    float opacity = txOpacity.Sample(samLinear, input.Tex).a;
    
    float3 ambient = 0;
    float3 emissive = txEmissive.Sample(samLinear, input.Tex).rgb;
    //ambient += emissive.rgb;

    //float4 finalColor = float4(directLight, 1.0f);
    //float4 finalColor = float4(directLight, 1.0f);
    float4 finalColor = float4(pow(float3(directLight + emissive/*+ ambient*/), 1.0 / 2.2), 1.0);
    
    finalColor.a = opacity;
    
    
    if (opacity < 0.1f)
    {
        discard;
    }
    //Adapt Gamma Correction
    return finalColor;
}