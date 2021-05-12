#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

struct Material
{
  int textureId;
};

struct Vertex
{
  vec3 pos;
  vec3 nrm;
  vec2 texCoord;
  vec3 tangent;
};

struct hitPayload
{
  vec3 hitValue;
};

struct sceneDesc
{
  int  objId;
  mat4 transform;
  mat4 transformIT;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

//layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS; // << REMOVE

layout(binding = 1, set = 1, scalar) buffer MatColorBufferObject { Material m[]; } materials[];
layout(binding = 2, set = 1, scalar) buffer ScnDesc { sceneDesc i[]; } scnDesc;
layout(binding = 3, set = 1) uniform sampler2D textureSamplers[];
layout(binding = 4, set = 1) buffer MatIndexColorBuffer { int i[]; } matIndex[];
layout(binding = 5, set = 1, scalar) buffer Vertices { Vertex v[]; } vertices[];
layout(binding = 6, set = 1) buffer Indices { uint i[]; } indices[];

hitAttributeEXT vec2 attribs;

void main()
{
  // Object of this instance
  uint objId = scnDesc.i[gl_InstanceID].objId;

  // Indices of the triangle
  ivec3 ind = ivec3(indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 0],   //
                    indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 1],   //
                    indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 2]);  //
  // Vertex of the triangle
  Vertex v0 = vertices[nonuniformEXT(objId)].v[ind.x];
  Vertex v1 = vertices[nonuniformEXT(objId)].v[ind.y];
  Vertex v2 = vertices[nonuniformEXT(objId)].v[ind.z];

  const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  // Material of the object
  int matIdx = matIndex[nonuniformEXT(objId)].i[gl_PrimitiveID];
  Material mat = materials[nonuniformEXT(objId)].m[matIdx];

  prd.hitValue = vec3(0);
  if(mat.textureId >= 0)
  {
    vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y + v2.texCoord * barycentrics.z;
    prd.hitValue = texture(textureSamplers[nonuniformEXT(mat.textureId)], texCoord).xyz;
  }
}
