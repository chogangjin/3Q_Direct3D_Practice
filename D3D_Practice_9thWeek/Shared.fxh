Texture2D txDiffuse : register(t0);
TextureCube SkyBox : register(t1);
Texture2D txNormal : register(t2);
Texture2D txSpecular : register(t3);
Texture2D txAmbient : register(t4);
Texture2D txEmissive : register(t5);
Texture2D txOpacity : register(t6);
Texture2D txShadow : register(t7);
SamplerState samLinear : register(s0);


cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    
    float3 vDirectionalLight;
    float4 vDirectionalColor;
    
    float4 DiffuseColor;
    float4 AmbientColor;
    float4 SpecularColor;
    
    float4 DiffuseMaterial;
    float4 AmbientMaterial;
    float4 SpecularMaterial;
    
    float3 camerapos;
    float shininess;
    
    uint IsRigid;
    uint RefBoneIndex;
    float2 padding;
}

cbuffer ShadowConstantBuffer : register(b2)
{
    //matrix View;
    //matrix Projection;
    matrix ShadowView;
    matrix ShadowProjection;
}

//#define VERTEX_SKINNING

cbuffer ModelMatrix : register(b3)
{
    matrix BonePose[128];
}

cbuffer ModelMatrix : register(b4)
{
    matrix BoneOffset[128];
}

struct VS_INPUT
{
    float4 pos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
//#ifdef VERTEX_SKINNING
    int4 BlendIndices : BLENDINDICES;
    float4 BlendWeights : BLENDWEIGHT;
//#endif
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 worldpos : TEXCOORD0;
    float2 Tex : TEXCOORD1;
    float3 norm : TEXCOORD2;
    float3 tangent : TEXCOORD3;
    float3 binormal : TEXCOORD4;
    float4 shadowpos : TEXCOORD5;
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

struct VS_SHADOW_INPUT
{
    float4 pos : POSITION;
};

struct PS_SHADOW_INPUT
{
    float4 pos : SV_Position;
};