#include "LineShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
VertexOutput main(VertexInput input)
{
    VertexOutput output;

	// Transform to homogeneous clip space.
    output.position = mul(float4(input.position, 1.0f), cWorldPosition);
    output.position = mul(output.position, cCameraView);
    output.position = mul(output.position, cCameraProjection);

	// Set the pixel colours
    output.colour = input.colour;

    return output;
}