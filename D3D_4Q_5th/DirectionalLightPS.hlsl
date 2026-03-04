#include "Shared.fxh"

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

float ShadowFactor(float3 worldPos)
{
    float4 sp = mul(float4(worldPos, 1.0f), ShadowView);
    sp = mul(sp, ShadowProjection);
    
    float currentDepth = sp.z / sp.w;
    float2 uv = sp.xy / sp.w;
    uv.y = -uv.y;
    uv = uv * 0.5f + 0.5f;
    
    if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1)
    {
        return 1.0f;
    }
    
    float sampleDepth = txShadow.Sample(samLinear, uv).r;
    
    return (currentDepth > sampleDepth + 0.001f) ? 0.0f : 1.0f;
}


float4 main(PS_QUAD_INPUT input) : SV_TARGET
{
    //GBuffer
    float4 base = txGBufferBaseColor.Sample(samLinear, input.UV);
    float4 Normal = txGBufferNormal.Sample(samLinear, input.UV);
    float4 pos = txGBufferPosition.Sample(samLinear, input.UV);
    float depth = txDepth.Sample(samLinear, input.UV).r;
    
    // TODO : a값이 아닌, Depth 값 받아서 discard 하기
    
    //if(base.a == 0)
    //{
    //    discard;
    //}
    if (depth >= 1)
    {
        discard;
    }
    
    float3 albedo = base.rgb;
    float opacity = base.a;
    
    float3 N = normalize(Normal.rgb * 2.0f - 1.0f);
    float Rough = Normal.a;
    
    float3 worldpos = pos.rgb;
    float metalic = saturate(pos.a);
    
    float3 lightVector = normalize(-vDirectionalLight.xyz);
    float3 viewVector = normalize(camerapos - worldpos);
    float3 halfvector = normalize(lightVector + viewVector);
    
    //Dot product
    float NdotH = max(dot(N, halfvector), 0);
    float NdotV = max(dot(N, viewVector), 0);
    float LdotH = max(dot(lightVector, halfvector), 0);
    float NdotL = max(dot(N, lightVector), 0);
    float VdotH = max(dot(viewVector, halfvector), 0);
    
    float FdiElectric = 0.04f;
    float3 F0 = lerp(FdiElectric, albedo, metalic);
    
    float D = Distribution_GGX(NdotH, max(Rough, 0.1f));
    float3 F = Fresnel_Shlick(F0, VdotH); // 빛의 방향 특정할 수 없기 때문에 NdotV
    float G = GeometricAttenuation_GGX(NdotV, Rough) * GeometricAttenuation_GGX(NdotL, Rough);
    
    float3 specularBRDF = D * F * G / max((4 * NdotL * NdotV),  EPSILON);
    
    float3 KS = F;
    float3 KD = (1 - KS) * (1 - metalic);
    float3 diffuse = KD * ((float3) albedo) / PI;
    float shadow = ShadowFactor(worldpos);
    float3 directLight = (diffuse + specularBRDF) * vDirectionalColor.rgb * saturate(NdotL) * shadow;
    directLight *= lightPower;
    //float opacity = txOpacity.Sample(samLinear, input.UV).a;
    
    // Indirect Lighting
    float3 ambient = 0;
    float3 emissive = txEmissive.Sample(samLinear, input.UV).rgb;
    
    //IBL Diffuse
    KD = lerp(1.0 - F, 0.0, metalic);
    float3 irradiance = txIrradianceMap.Sample(samLinear, N).rgb;
    float3 diffuseIBL = KD * albedo * irradiance / PI;
    
    //IBL Specular
    float2 BRDF = txLookUpTexture.Sample(samClamp, float2(NdotV, Rough)).rg;
    float A = BRDF.x;
    float B = BRDF.y;
    
    uint numMip = 0;
    uint width = 0;
    uint height = 0;
    
    txPrefilteredMap.GetDimensions(0, width, height, numMip);
    
    float3 ReflectedLight = normalize(reflect(-viewVector, N));
    float perceptualRough = saturate(Rough);
    float mip = perceptualRough * perceptualRough * (numMip - 1);
    mip = clamp(mip, 0.0f, (float) (numMip - 1));
    float3 prefiltered = txPrefilteredMap.SampleLevel(samLinear, ReflectedLight, mip).rgb;
    
    //float3 F_ibl = F0 + (max(1.0 - Rough, F0) - F0) * pow(1.0 - NdotV, 5.0);
    float3 specularIBL = prefiltered * (F0 * A + B);
    //specularIBL = prefiltered * (F_ibl * A + B);
    float3 indirectIBL = (diffuseIBL + specularIBL);
    
    //final
    float4 finalColor = 0;
    float3 lighting = float3(directLight + emissive + indirectIBL);
    finalColor = float4(lighting, opacity);
    finalColor.a = opacity;
    
    return finalColor;
}