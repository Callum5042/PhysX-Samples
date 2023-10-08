// Vertex input
struct VertexInput
{
    float3 position : POSITION;
    float4 colour : COLOUR;
};

// Vertex output / pixel input structure
struct VertexOutput
{
    float4 position : SV_POSITION;
    float4 colour : COLOUR;
};

// World constant buffer
cbuffer ModelBuffer : register(b0)
{
    matrix cWorldPosition;
    matrix cInverseWorldPosition;
    float4 cColour;
}

// Camera buffer
cbuffer CameraBuffer : register(b1)
{
    matrix cCameraView;
    matrix cCameraProjection;
    float4 cCameraPosition;
}