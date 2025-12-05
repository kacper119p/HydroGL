struct particle
{
    vec3 Position;
    float Rotation;
    vec3 Velocity;
    float LifeTime;
};

struct drawElementsIndirectCommand
{
    uint Count;
    uint InstanceCount;
    uint FirstIndex;
    int BaseVertex;
    uint BaseInstance;
};

#define DEFINE_PARTICLE_DATA_BUFFER(binding_point, access) \
    layout(std430, binding = binding_point) access restrict buffer particleDataBuffer \
    { \
        particle Particles[];\
 } ParticleDataBuffer;

#define DEFINE_PARTICLE_FREE_LIST_BUFFER(binding_point, access) \
    layout(std430, binding = binding_point) access restrict buffer freelistBuffer \
    { \
        int Count; \
        uint Indices[];\
 } FreeListBuffer;

#define DEFINE_PARTICLE_ALIVE_LIST_BUFFER(binding_point, access) \
    layout(std430, binding = binding_point) access restrict buffer aliveListBuffer \
    { \
        uint Indices[];\
 } AliveListBuffer;

#define DEFINE_INDIRECT_DRAW_BUFFER(binding_point, access) \
    layout(std430, binding = binding_point) access restrict buffer indirectDrawBuffer \
    { \
        drawElementsIndirectCommand Command; \
 } IndirectDrawBuffer;

#define DEFINE_SPRAY_PARAMETERS_BUFFER(binding_point)

layout (std140, binding = 0) uniform sprayParameters
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    vec3 CameraPosition;
    uint TotalParticleCount;
    vec3 SceneColor;
    uint ParticlesSpawnedThisFrame;
    vec2 WindVelocity;
    uint Seed;
    float SpawnRange;
    float LifeTime;
    float DeltaTime;
    float Time;
    float SprayOpacity;
    float SpraySize;
    float SpawnThreshold;
    float WaterVelocityInfluence;
    float WindInfluence;
};
