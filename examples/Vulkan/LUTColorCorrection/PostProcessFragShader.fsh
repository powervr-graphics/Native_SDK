#version 450

layout(set = 0, binding = 0) uniform sampler2D sSceneTexture;
layout(set = 0, binding = 1) uniform sampler3D sLUT3D;
layout(set = 0, binding = 2) uniform sampler2D sLUT2D;
layout(set = 1, binding = 0) uniform UBO
{
    mat4  mvp;
    vec3  lightDir;
    uint  lutSize;
    uint  use3DLUT;
}ubo;

layout(location = 0) in vec2 vTexCoords;
layout(location = 0) out vec4 oColor;

vec3 sample3DLUT(vec3 color)
{
    return texture(sLUT3D, color).rgb;
}

vec3 sample2DLUT(vec3 color)
{
    float size = ubo.lutSize;

    // convert blue channel into LUT slice index
    float blue = color.b * (size - 1.0);

    // current slice
    float slice0 = floor(blue);
    // next slice used for interpolation
    float slice1 = min(slice0 + 1.0, size - 1.0);
    // interpolation factor between slices
    float t = fract(blue);

    // horizontal textel position
    float u0 = slice0 * size + color.r * (size - 1.0);
    float u1 = slice1 * size + color.r * (size - 1.0);
    // vertical coordinate from green channel
    float v  = color.g * (size - 1.0);

    // move to texel centers to avoid filtering artifacts
    u0 = (u0 + 0.5) / (size * size);
    u1 = (u1 + 0.5) / (size * size);
    v  = (v  + 0.5) / size;

    // Sample 2 neighboring slices
    vec3 c0 = texture(sLUT2D, vec2(u0, v)).rgb;
    vec3 c1 = texture(sLUT2D, vec2(u1, v)).rgb;

    // interpolate results
    return mix(c0, c1, t);
}

void main()
{
    vec3 color = texture(sSceneTexture, vTexCoords).rgb;
    
    // apply LUT effect only on right half of screen
    // left side remains original for comparison
    if(vTexCoords.x > 0.5)
    {
        if(ubo.use3DLUT == 1)
        {
            color = sample3DLUT(color);
        }
        else
        {
            color = sample2DLUT(color);
        }
    }

    oColor = vec4(color, 1.0);
}