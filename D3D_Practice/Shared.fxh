Texture2D txDiffuse : register(t0);
TextureCube SkyBox : register(t1);

SamplerState samLinear : register(s0);


cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    
    float4 vDirectionalLight;
    float4 vDirectionalColor;
    
    float3 camerapos;
    float shininess;
}



struct VS_INPUT
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 worldpos : TEXCOORD0;
    float3 norm : TEXCOORD1;
    float2 Tex : TEXCOORD2;
};

struct VS_SKYBOX_INPUT
{
    float4 pos : POSITION;
};

struct PS_SKYBOX_INPUT
{
    float4 pos : SV_Position;
    float3 Tex : TEXCOORD0;
};