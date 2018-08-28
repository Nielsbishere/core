# Osomi Raw Model (.oiRM)
The file format was created to support the choice between low load times and small file sizes, as well as being more game-based, allowing the specification of any texture format. Unlike fbx, obj or other formats, you can choose the texture format you need for every variable, as well as allowing to use multiple vbos to support that decision. This format would correspond to a VBO/IBO and VAO in OpenGL, which are very easily creatable from plain oiRM. This type is also a binary format, making it harder to modify and keeping your assets safe(r). Other than that, oiRM stores the topology and fill mode of the model, allowing you to specify that the model should only be rendered the way you tell it to. However; setting these to undefined will allow them to be used as anything. Last but not least, oiRM supports miscs, which is a primitive or a group of primitives with a special meaning, this could also have an offset or just be a point in model space. Miscs could be used for gameplay features, such as anchor points, spawn points and more.  
The compression flag turns oiRM from high file size and very fast / almost immediate load in into a very compact, yet still very fast file format. This version with compression is shortened as oiRMc.
# File specification
## Header
```cpp
  struct RMHeader {

  char header[4];   //oiRM

  u8 version;       //RMHeaderVersion
  u8 flags;         //RMHeaderFlag1
  u8 vertexBuffers;
  u8 vertexAttributes;

  u8 topologyMode;  //TopologyMode
  u8 fillMode;      //FillMode
  u8 miscs;
  u8 p0;

  u8 p1[4];

  u32 vertices;

  u32 indices;      //Optional (=0 if no indices)

};
```
'version' is the oiRM version; v_0_0_1 is the first version, but other versions will probably pop up, to support other features in the future.  
'flags' include the following; Contains_materials (1) if there's materials embedded with the model. Per_tri_materials (2) if there's multiple materials per triangle, this will be `u8[triangles]` that specifies which triangle has which material. Uses_compression (4) if you prefer small file size over fast load times.  
'vertexBuffers' is the number of 'RMVBO' structs that are included, they represent a physical buffer.  
'vertexAttributes' is the number of 'RMAttribute' structs that are included, they represent a variable in a buffer.  
'topologyMode' is defined in ogc.md; important ones are triangle, points and lines.  
'fillMode' is how the polygons are processed; most commonly fill (see ogc.md).  
'miscs' how many miscs are in this file.  
'vertices' how many vertices are in this file.  
'indices' how many indices are in this file (0 if there's no indices present).
## RMVBO
```cpp
struct RMVBO {
  u16 stride;
  u16 layouts;
};
```
'stride' is how many bytes each vertex takes up. A maximum is 65 535 bytes, which is quite a bit for a vertex.  
'layouts' is how many layouts are used in this vbo.
## RMAttribute
```cpp
struct RMAttribute {
  u8 padding;
  u8 format;			//TextureFormat
  u16 name;
};
```
'name' represents the name that's stored in the oiSL file.  
'format' is the attribute's format.  
'padding' is used to pad the attribute to 4 bytes, can also be used in the future.
## RMMisc
```cpp
struct RMMisc {

  u8 flags;			//RMMiscFlags
  u8 type;			//RMMiscType
  u16 size;			//Data size
			
};
```
'flags' can be 'Uses_offset' (1) if there is an offset to the primitives, 'Is_array' (2) if there are multiple primitives present, Offset_in_relative_space (4) if the offset uses relative space (1 vertex; tangent space, multiple; combined tangent space).  
'type' can be Vertex (0), Primitive (1), Center_primitive (2) the center of a primitive, Point (3) a point in model space.  
'size' the data attached to the misc.
## VBOs
After this data, all VBO data is stored in binary (but might have to be decompressed). One vertex's buffer is stored as following:
```
[ 0, Graphics::getFormatSize(format0) ]
[ Graphics::getFormatSize(format0), Graphics::getFormatSize(format1)]
etc.
```
For example;
```
Vec3 position, normal(0, 1, 0), tangent(1, 0, 0);
Vec2 uv;
Vertex v0(position, uv, normal);
Vertex v1(tangent);
```
A vertex with buffers (position, uv, normal) and (tangent) would be stored as following:
```
First buffer

0 0 0	(pos = 12 bytes; 3 floats)
0 0	(uv = 8 bytes; 2 floats)
0 1 0	(normal = 12 bytes; 3 floats)

Second buffer

1 0 0	(tangent = 12 bytes; 3 floats)

```
The second buffer would be appended to the first buffer, giving us a float array `[0,0,0, 0,0, 0,1,0, 1,0,0]`, which is represented as a buffer. This means that the (relative) offset of the second buffer can be calculated by using RMVBO::stride * vertices and the index buffer (in this case; if there is no other buffer and if indices is not 0) is located at offset buf0.stride * vertices + buf1.stride * vertices.
### (TODO) Compression
## IBO
The index buffer is located after the vertex buffers and the size can be determined by first determing the format. If there's at max 256 vertices, it means that you can use a u8 to represent an index to it, if there's at max 65536, it means you can use a u16 and otherwise you have to use a u32. This means that the index buffer can be the size of vertices, 2 * vertices or 4 * vertices, depending on the vertexCount. On the GPU, this IBO is expanded to 4 * vertices, because of cache improvements that come with uints (it can fetch it with 1 operation instead of multiple).
### (TODO) Compression
## (TODO) MiscBuffer
## (TODO) Names
# (TODO) API Usage
