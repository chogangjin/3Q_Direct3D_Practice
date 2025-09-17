Texture2D txDiffuse : register(t0);
TextureCube SkyBox : register(t1);

SamplerState samLinear : register(s0);


cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vDirectionalLight[2];
    float4 vDirectionalColor[2];
    float4 vOutputColor;
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
    float3 norm : TEXCOORD0;
    float2 Tex : TEXCOORD1;
};

struct VS_SKYBOX_INPUT
{
    float4 pos : POSITION;
};

struct PS_SKYBOX_INPUT
{
    float4 pos : SV_Position;
    float3 Tex : position;
};