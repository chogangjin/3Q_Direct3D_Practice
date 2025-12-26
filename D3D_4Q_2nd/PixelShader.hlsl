#include <Shared.fxh>

static const float PI = 3.14159265f;
static const float EPSILON = 1e-6f;

// Fresnel Shlick reflection
float3 Fresnel_Shlick(in float3 f0, in float x) // F
{
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
    //Direct Lighting
    //Diffuse
    float4 DiffuseMap = txDiffuse.Sample(samLinear, input.Tex);
    float3 albedo = DiffuseMap.rgb;
    if (OverrideMaterial)  {  albedo = DiffuseColor.rgb;  }
    
    //TBN
    float3x3 TBN = { input.tangent.rgb, input.binormal.rgb, input.norm.rgb };
    
    //NormalMap
    float3 normal = txNormal.Sample(samLinear, input.Tex).rgb;
    normal = normalize(2.0f * normal - 1.0f);
    
    //Metalic
    float metalic = txMetalic.Sample(samLinear, input.Tex).r;
    if (OverrideMaterial)  { metalic = Metalness;  }
    
    //Roughness
    float Rough = txRoughness.Sample(samLinear, input.Tex).r;
    if (OverrideMaterial)
    {
        Rough = Roughness;
    }

    // vector
    float3 lightVector = normalize(-vDirectionalLight.xyz);
    float3 normalVector = normalize(input.norm);
    if (HasNormalMap)    {   normalVector = normalize(mul(normal, TBN)); }
    
    float3 viewVector = normalize(camerapos - input.worldpos);
    float3 halfvector = normalize(lightVector + viewVector);
    
    //Dot product
    float NdotH = max(dot(normalVector, halfvector), 0);
    float NdotV = max(dot(normalVector, viewVector), 0);
    float LdotH = max(dot(lightVector, halfvector), 0);
    float NdotL = max(dot(normalVector, lightVector), 0);
    float VdotH = max(dot(viewVector, halfvector), 0);
    
    float FdiElectric = 0.04f;
    float3 F0 = lerp(FdiElectric, albedo, metalic);
    
    float D = Distribution_GGX(NdotH, max(Roughness, 0.1f));
    //float3 F = Fresnel_Shlick(F0, VdotH);
    float3 F = Fresnel_Shlick(F0, NdotV); // 빛의 방향 특정할 수 없기 때문에 NdotV
    float G = GeometricAttenuation_GGX(NdotV, Rough) * GeometricAttenuation_GGX(NdotL, Rough);
    
    float3 specularBRDF = D * F * G / ((4 * NdotL * NdotV) + 0.001);
    
    float3 KS = F;
    float3 KD = (1 - KS) * (1 - metalic);
    float3 diffuse = KD * ((float3) albedo) / PI;
    float3 directLight = (diffuse + specularBRDF) * vDirectionalColor.rgb * saturate(NdotL);
    float opacity = txOpacity.Sample(samLinear, input.Tex).a;
    
    // Indirect Lighting
    float3 ambient = 0;
    float3 emissive = txEmissive.Sample(samLinear, input.Tex).rgb;
    
    //IBL Diffuse
    KD = lerp(1.0 - F, 0.0, metalic);
    float3 irradiance = txIrradianceMap.Sample(samLinear, normalVector).rgb;
    float3 diffuseIBL = KD * DiffuseMap.rgb * irradiance / PI;
    
    //IBL Specular
    float2 BRDF = txLookUpTexture.Sample(samLinear, float2(NdotL, Rough)).rg;
    //float2 BRDF = txLookUpTexture.Sample(samLinear, float2(NdotV, Rough)).rg;
    float A = BRDF.x;
    float B = BRDF.y;
    
    uint currMipLevel = 0;
    uint width = 0;
    uint height = 0;
    
    txEnvironmentMap.GetDimensions(0, width, height, currMipLevel);
    
    float3 ReflectedLight = reflect(-viewVector, normalVector);
    float mip = Rough * currMipLevel;
    float3 prefiltered = txPrefilteredMap.SampleLevel(samLinear, ReflectedLight, mip).rgb;
    
    float3 specularIBL = prefiltered * (F0 * A + B);
    float3 indirectIBL = (diffuseIBL + specularIBL) /** ambient*/;
    
    // Shadow
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

    //final
    float4 finalColor = 0;
    
    //Adapt Gamma Correction
    finalColor = float4(pow(float3(directLight + emissive + indirectIBL ), 1.0 / 2.2), 1.0);
    
    finalColor.a = opacity;
    
    if (opacity < 0.1f)
    {
        discard;
    }
    
    return finalColor;
}