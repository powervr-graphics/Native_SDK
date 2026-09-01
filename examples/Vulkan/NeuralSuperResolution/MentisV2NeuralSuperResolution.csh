#version 450 core

#pragma use_vulkan_memory_model
#extension GL_KHR_shader_subgroup_basic : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_KHR_memory_scope_semantics : enable
#extension GL_NV_cooperative_matrix : require
#extension GL_NV_integer_cooperative_matrix : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_control_flow_attributes : enable

#define iM 16
#define iK 16
#define iN 16
#define K_ 64
#define N_ 64
#define K_L1 96
#define LAYERS 3
#define C_ 86
#define M_STEPS (M_/iM) // 32400
#define K_STEPS (K_/iK) // 4
#define N_STEPS (N_/iN) // 4
#define K_STEPS_L1 (K_L1/iK)
#define WARP_SIZE (2*iM)
#define M_ 518400
#define IM_W 960
#define IM_H 540

layout(local_size_x = 1, local_size_y = 32, local_size_z = 1) in;

layout(set = 0, binding = 1) uniform highp sampler2D imageInMotionVector;

layout(set = 0, binding = 2) uniform highp sampler2D imageInDepth;

layout(set = 0, binding = 3) uniform highp sampler2D imageInPreviousResult;

uniform layout(rgba8, set = 0, binding = 4) highp image2D imageOut;

layout(set = 0, binding = 0) uniform highp sampler2D x_image;

//  TODO: Put in different binding as this has per-frame update frequency
layout(set = 0, binding = 5) uniform nnUniformBuffer { highp vec2 jitter; highp float frameCounter; };

layout(set = 0, binding = 6) buffer buf_matrix_b
{
    float16_t matrix_b[LAYERS][K_STEPS][N_STEPS][iK * iN];
};

layout(set = 0, binding = 7) buffer buf_matrix_b_l1
{
    float16_t matrix_b_l1[K_STEPS_L1][N_STEPS][iK * iN];
};

layout(set = 0, binding = 8) buffer buf_debugBuffer
{
    float debugBuffer[];
};

fcoopmatNV<32, gl_ScopeSubgroup, iM, iN> result[N_STEPS];
fcoopmatNV<16, gl_ScopeSubgroup, iM, iK> matA[K_STEPS_L1];
fcoopmatNV<16, gl_ScopeSubgroup, iK, iN> matB1;
fcoopmatNV<16, gl_ScopeSubgroup, iK, iN> matB2;

shared float matCshared_f32[iM * iN];
shared float16_t matCshared_f16[iM * iN];

shared float16_t matA_local[iM][K_L1];
shared float16_t temp[iM * iN];

uint gidx = gl_GlobalInvocationID.x;
uint gidy = gl_GlobalInvocationID.y;
uint lidx = gl_LocalInvocationID.x;
uint lidy = gl_LocalInvocationID.y;
uint widx = gl_WorkGroupID.x;
uint widy = gl_WorkGroupID.y;
uint wgs = gl_WorkGroupSize.x;

bool record = false;

// Bilinear sampling function with offset
// This function is here because Kompute doesnt support bilinear sampling, only nearest
// In the final code HW sampling should be used
vec4 bilinear_sample(sampler2D image, float sample_x, float sample_y)
{
    // Get the four surrounding pixel coordinates
    int x0 = int(floor(sample_x));
    int y0 = int(floor(sample_y));
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // Calculate interpolation weights
    float fx = sample_x - float(x0);
    float fy = sample_y - float(y0);

    // Sample the four surrounding pixels
    vec4 p00 = texture(image, vec2(float(x0) + 0.5, float(y0) + 0.5));
    vec4 p10 = texture(image, vec2(float(x1) + 0.5, float(y0) + 0.5));
    vec4 p01 = texture(image, vec2(float(x0) + 0.5, float(y1) + 0.5));
    vec4 p11 = texture(image, vec2(float(x1) + 0.5, float(y1) + 0.5));

    // Bilinear interpolation
    vec4 top = mix(p00, p10, fx);
    vec4 bottom = mix(p01, p11, fx);
    vec4 bil_sample = mix(top, bottom, fy);

    return bil_sample;
}

void im2col(int x, int y, uint row)
{
    uint local_row = row % iM;

    const int k1 = 3;
    const int k2 = 4;
    int miny = y - (k1 - 1) / 2;
    int maxy = miny + k1;
    int minx = x - (k1 - 1) / 2;
    int maxx = minx + k1;
    int idx = 0;
    // First process x image (3 channels from x_image)
    // OPT: take a single texture sample (+ edges), write to local, distribte to threads
    // OPT: maybe use buffer?
    for (int c = 0; c < 4; c++)
    { // 3 channels from x_image
        for (int i = miny; i < maxy; i++)
        {
            for (int j = minx; j < maxx; j++)
            {
                bool cond = (i >= 0 && i < IM_H && j >= 0 && j < IM_W);
                if (cond)
                {
                    vec4 x_val = vec4(0.0);
                    float pixel_val = 0.0;

                    // Color and depth are not merged in the same input image in this case
                    if(c < 3)
                    {
                        x_val = texture(x_image, vec2((float(j) + 0.5), (float(i) + 0.5))); // OPT: only read once
                        pixel_val = x_val[c];
                    }
                    else
                    {
                        x_val = texture(imageInDepth, vec2((float(j) + 0.5), (float(i) + 0.5))); // OPT: only read once
                        pixel_val = x_val.x;

                    }
                                      
                    matA_local[local_row][idx] = float16_t(pixel_val);
                }
                else
                {
                    
                    matA_local[local_row][idx] = float16_t(0); // zero padding
                }
                idx++;
            }
        }
    }

    miny = 2 * y - (k2 - 1) / 2; // 2y-1
    maxy = miny + k2;
    minx = 2 * x - (k2 - 1) / 2; // 2x-1
    maxx = minx + k2;
    for (int c = 0; c < 3; c++)
    { // 3 channels from hr_image
        for (int i = miny; i < maxy; i++)
        {
            for (int j = minx; j < maxx; j++)
            {
                bool cond = (i >= 0 && i < IM_H * 2 && j >= 0 && j < IM_W * 2);
                if (cond)
                {
                    // vec4 hr_val = imageLoad(hr_image, ivec2(j, i)); // This needs to be a single sample copied to local memory so we won't optimise it futher for now

                    float x_offset = float(j % 2) - 0.5;
                    float y_offset = float(i % 2) - 0.5;

                    float sample_x = texture(imageInMotionVector, vec2(j/2, i/2)).g + x_offset;
                    float sample_y = texture(imageInMotionVector, vec2(j/2, i/2)).r + y_offset;
                    vec4 hr_val = bilinear_sample(imageInPreviousResult, sample_x, sample_y);
                    float pixel_val = (c == 0) ? hr_val.r : (c == 1) ? hr_val.g : hr_val.b;
                    
                    matA_local[local_row][idx] = float16_t(pixel_val);
                }
                else
                {
                    
                    matA_local[local_row][idx] = float16_t(0); // zero padding
                }
                idx++;
            }
        }
    }

    // The original Mentis implementation stores jitter values jitter.x and jitter.y in the format (jitter.y, jitter.x)
    // The SDK sample stores jitter values jitter.x and jitter.y in the format (jitter.x, jitter.y)
    matA_local[local_row][idx++] = float16_t(jitter.x);
    matA_local[local_row][idx++] = float16_t(jitter.y);
    
    for(int i = idx; i < 96; ++i)
    {
        // fill remaining elements with 0. Might skip this if weights are 0 padded
        matA_local[local_row][i] = float16_t(0);
    }
}

// Standard C to A conversion function
// OPT: use register-to-register
fcoopmatNV<16, gl_ScopeSubgroup, iM, iK> CtoA(fcoopmatNV<32, gl_ScopeSubgroup, iM, iN> matC)
{
    fcoopmatNV<16, gl_ScopeSubgroup, iM, iK> matA;
    barrier();
    coopMatStoreNV(matC, matCshared_f32, 0, iN, false); // store in shared memory
    barrier(); // ensure all threads have written to shared memory
    const int elems = (iM * iN) / (WARP_SIZE);
    float array[elems];
    for (int i = 0; i < elems; i++)
    { // load from shared memory
        array[i] = (matCshared_f32[lidy * elems + i]); // TODO: fix bank conflicts
    }
    barrier();
    for (int i = 0; i < elems; i++) { matCshared_f16[lidy * elems + i] = float16_t(array[i]); }
    barrier();
    coopMatLoadNV(matA, matCshared_f16, 0, iN, false);
    barrier();
    return matA;
}

fcoopmatNV<16, gl_ScopeSubgroup, iM, iK> relu(fcoopmatNV<16, gl_ScopeSubgroup, iM, iN> A)
{
    const int elems = (iM * iN) / (WARP_SIZE);
    for (int i = 0; i < elems; i++) { A[i] = float16_t(max(A[i], 0)); }
    return A;
}

fcoopmatNV<16, gl_ScopeSubgroup, iM, iK> residual(fcoopmatNV<16, gl_ScopeSubgroup, iM, iN> A_prev, fcoopmatNV<16, gl_ScopeSubgroup, iM, iK> A_new)
{
    const int elems = (iM * iN) / (WARP_SIZE);
    for (int i = 0; i < elems; i++) { A_prev[i] += float16_t(max(A_new[i], 0)); }
    return A_prev;
}

float sigmoid(float x) { return 1.0 / (1.0 + exp(-x)); }

// This function takes the neural network's output features and synthesizes the final high-resolution image by:
// 1. Applying a learned blending factor to mix multiple sources
//  - the output
// 3. Handling spatial upscaling from low to high resolution
void output_blend()
{
    // Only process valid pixels within image bounds
    uint pixel_pos = gidy / 2; // "/2" since 1 matrix row is 2 threads
    if (pixel_pos >= IM_H * IM_W) return; // Out of bounds check
    if (gidy % 2 == 1) return; // every 2nd thread can be skipped, since 1 matrix row is 2 threads
    uint x_base = (pixel_pos % IM_W) * 2; // Base x position in output (2x upscaled)
    uint y_base = (pixel_pos / IM_W) * 2; // Base y position in output (2x upscaled)
    uint row = pixel_pos % iM;
    if (row < 16 && pixel_pos < IM_H * IM_W)
    {
        for (uint y = 0; y < 2; ++y)
        {
            for (uint x = 0; x < 2; ++x)
            {
                uint out_x = x_base + x;
                uint out_y = y_base + y;
                // debugPrintfEXT("<><><>");
                // Bounds check for output coordinates
                if (out_x < 2 * IM_W && out_y < 2 * IM_H)
                {
                    // Get blend factor from first 4 elements (UL, UR, LL, LR)
                    float blend_in = matCshared_f16[lidy * iN + y * 2 + x];
                    float blend_factor = sigmoid(float(matCshared_f16[row * iN + y * 2 + x]));

                    float x_offset = float(out_x % 2) - 0.5;
                    float y_offset = float(out_y % 2) - 0.5;

                    float sample_x = texture(imageInMotionVector, vec2((out_x) / 2, (out_y) / 2)).g + x_offset; // x component of motion vector
                    float sample_y = texture(imageInMotionVector, vec2((out_x) / 2, (out_y) / 2)).r + y_offset; // y component of motion vector
                    vec4 hr_val = bilinear_sample(imageInPreviousResult, sample_x, sample_y);

                    // The SDK sample stores jitter values jitter.x and jitter.y in the format (jitter.x, jitter.y)
                    sample_x = (float(out_x - 0.5) / 2.0) - jitter.x;
                    sample_y = (float(out_y - 0.5) / 2.0) - jitter.y;
                    vec4 bil_pixel = bilinear_sample(x_image, sample_x, sample_y); // Bilinear sample with offset

                    vec4 outColor = vec4(0.0);
                    for (uint c = 0; c < 3; ++c)
                    {
                        float bil_val = bil_pixel[c];

                        float pixel = float(matCshared_f16[row * iN + 4 + c * 4 + y * 2 + x]);
                        float hr_pixel = hr_val[c]; // Get the pixel value from hr image
                        // OPT: bypass the L2 cache
                        //new_buf[c][out_y][out_x] = blend_factor * (pixel + hr_pixel) + (1.0 - blend_factor) * bil_val;
                        outColor[c] = blend_factor * (pixel + hr_pixel) + (1.0 - blend_factor) * bil_val;
                        
                    }

                    imageStore(imageOut, ivec2(out_x, out_y), outColor);
                }
            }
        }
    }
}

void main()
{
    uint m = widy;
    uint row = gidy / 2; // "/2" since 1 matrix row is 2 threads
    uint x = row % IM_W;
    uint y = row / IM_W;
    if (row < IM_H * IM_W) im2col(int(x), int(y), row); // Load data into shared memory. OPT: load directly to matA, skipping LM
    barrier(); // Ensure all threads have written to shared memory

    // Full computation mode
    fcoopmatNV<16, gl_ScopeSubgroup, iM, iK> matA_im2col = fcoopmatNV<16, gl_ScopeSubgroup, iM, iK>(0.0);

    for (int n = 0; n < N_STEPS; n++) { result[n] = fcoopmatNV<32, gl_ScopeSubgroup, iM, iN>(0.0); }

    // TODO: preload matA to local memory at 8 bit

    for (int n = 0; n < N_STEPS; n++)
    {
        for (int k = 0; k < K_STEPS_L1; k++)
        {
            coopMatLoadNV(matA[k], matA_local[0], k * iK, K_L1, false);
            coopMatLoadNV(matB1, matrix_b_l1[k][n], 0, iN, false);
            result[n] = coopMatMulAddNV(matA[k], matB1, result[n]);
        }
    }

    for (int n = 0; n < N_STEPS; n++)
    {
        matA[n] = relu(CtoA(result[n])); // relu activation
    }

    for (int l = 0; l < LAYERS; l++)
    {
        for (int n = 0; n < N_STEPS; n++)
        {
            result[n] = fcoopmatNV<32, gl_ScopeSubgroup, iM, iN>(0.0);
            for (int k = 0; k < K_STEPS; k++)
            {
                coopMatLoadNV(matB1, matrix_b[l][k][n], 0, iN, false);
                result[n] = coopMatMulAddNV(matA[k], matB1, result[n]);
            }
            if (l == LAYERS - 1) // Break after first 16 oyutputs on last layer
                break; // last layer has only iN outputs. TODO: Make sure also true on MMA
        }
        for (int n = 0; n < N_STEPS; n++)
        {
            if (l < LAYERS - 1)
            {
                // if (gidy==0) debugPrintfEXT(">>>> %f %d\n", matA[n][0], l);
                matA[n] = residual(matA[n], CtoA(result[n])); // relu activation
            }
            else
            {
                matA[n] = CtoA(result[n]);
                if (n == 0) output_blend(); // Blend output with bil and hr
            }
        }
    }
}
