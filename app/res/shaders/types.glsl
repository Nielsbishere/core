#define _GLSL
#define _VULKAN

#define u32 uint
#define f32 float
#define f64 double

#define Vec2 vec2
#define Vec3 vec3
#define Vec4 vec4
#define Quat Vec4
#define Vec2d dvec2
#define Vec3d dvec3
#define Vec4d dvec4
#define Vec2i ivec2
#define Vec3i ivec3
#define Vec4i ivec4
#define Vec2u uvec2
#define Vec3u uvec3
#define Vec4u uvec4

#define Matrix mat4
#define Matrix4 mat4
#define Matrix3 mat3
#define Matrix2 mat2
#define Matrixd dmat4
#define Matrix4d dmat4
#define Matrix3d dmat3
#define Matrix2d dmat2
#define Matrixu umat4
#define Matrix4u umat4
#define Matrix3u umat3
#define Matrix2u umat2
#define Matrixi imat4
#define Matrix4i imat4
#define Matrix3i imat3
#define Matrix2i imat2

#define lerp mix

#define Sampler(register, name) layout(binding = register) uniform sampler name
#define Texture2D(register, name) layout(binding = register) uniform texture2D name
#define Texture2Du(register, name) layout(binding = register) uniform utexture2D name
#define Texture2Di(register, name) layout(binding = register) uniform itexture2D name
#define Image2D(register, name, format) layout(binding = register, format) uniform image2D name
#define Image2Du(register, name, format) layout(binding = register, format) uniform uimage2D name
#define Image2Di(register, name, format) layout(binding = register, format) uniform iimage2D name
#define TextureList(register, name, maxCount) layout(binding = register)  uniform texture2D name[maxCount]
#define Uniform(register, name, typeName, definition) layout(binding = register) uniform typeName definition name
#define Array(register, name, typeName, type) layout(binding = register, std430) buffer typeName { type name[]; }
#define ConstArray(register, name, typeName, type) layout(binding = register, std430) readonly buffer typeName { type name[]; }
#define Buffer(register, name, typeName, definition) layout(binding = register, std430) buffer typeName definition name
#define ConstBuffer(register, name, typeName, definition) layout(binding = register, std430) readonly buffer typeName definition name
#define Out(register, name, type) layout(location = register) out type name
#define In(register, name, type) layout(location = register) in type name
#define ConstIn(register, name, type) layout(location = register) in flat type name

#define Vertex() out gl_PerVertex { Vec4 gl_Position; }; void main()
#define Fragment() layout(early_fragment_tests) in; void main()
#define Geometry() void main()
#define Compute(x, y, z) layout(local_size_x = x, local_size_y = y, local_size_z = z) in; void main()
#define Tesselation() void main()
#define TesselationEvaluation() void main()
#define AnyHit() void main()
#define ClosestHit() void main()
#define Miss() void main()
#define Call() void main()
#define Raygen() void main()

Vec4 sample2D(sampler s, texture2D t, Vec2 uv){
	return texture(sampler2D(t, s), uv);
}

Vec4u sample2Du(sampler s, utexture2D t, Vec2 uv){
	return texture(usampler2D(t, s), uv);
}

Vec4i sample2Di(sampler s, itexture2D t, Vec2 uv){
	return texture(isampler2D(t, s), uv);
}

#define write2D(img, coords, value) imageStore(img, Vec2i(coords), value)

#define instanceId gl_InstanceIndex
#define invocationId gl_GlobalInvocationID
#define vPosition gl_Position
#define mul(x, y) x * y
#define ref inout
#define inline 