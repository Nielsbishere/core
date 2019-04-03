# Oxsomi Material List (.oiML)

The oiML format is created to allow applying multiple materials to the same model; allowing you to batch lots of objects into the same batch, but making them look different. Each node has a subtype, this subtype is used to identify the current material list. The oiML file contains 1 list of materials and 1 of indices that are used by a subtype. This format is created to support materials on models.

This format allows you to create modular assets that can be used with different materials; allowing you to change their looks, while keeping efficiency. In gameplay purposes this could be to change the texture (like with in-game paintings, shiny/altered colorations, tools/armor, etc.).

## File spec

### Header

```cpp
struct MLHeader {
  
	char header[4];			//oiML
    
    u8 version;				//MLHeaderVersion
    u8 containsStrings;
    u16 textures;
    
    u16 materials;
    u16 materialLists;
    
    u16 materialsPerList;
    u16 padding;
    
};
```

'version' is the oiML version; v0_1 is the first version, but other versions might be added later.
'containsStrings' If this is set to true, it doesn't use an external string list, if false, it does. This is generally set to false if it's attached to an oiRM file.
'textures' is the number of textures that are included in the file; the textures packed have to be of the supported type (PNG). 
'materials' is the number of materials that are included in the file; this has to be non zero.
'materialLists' is the count of material lists, if this is zero, it will create a default material list that includes all materials.

### MLTexture

```cpp
//4 bytes texture header; Texture with max size 256 MiB
struct MLTexture {
	u32 mipFilter : 1;
	u32 loadFormat : 3;
	u32 size : 28;
};
```

'size + 1' is the size of the texture's data (null textures aren't allowed).
'loadFormat' + 1 = TextureLoadFormat.
'mipFilter' + 1 = TextureMipFilter.

Every texture's data is stacked onto the previous texture's data. The first texture is located after the MLTexture array; the second texture is located at `textures[0].size + textureOffset`.

### MLMaterial

```cpp
//16 bytes texture header, 44 bytes material header; 60 bytes per material
struct MLMaterial {
	
    Vec3 reflective;
    
    Vec3 diffuse;
    
    Vec3 emissive;
    
    u8 padding;
    u8 textureFlags;			//MaterialTextureType
    u16 opacity;
    
    u16 metallic;
    u16 roughness;
    
    u16 t_diffuse;
    u16 t_opacity;
    
    u16 t_emissive;
    u16 t_roughness;
    
    u16 t_ao;
    u16 t_height;
    
    u16 t_metallic;
    u16 t_normal;
    
};
```

`reflective` is the reflective color (if it's non metallic), `diffuse` is the albedo color (for lighting), `emissive` is the emissive color (for bloom).

`opacity`, `metallic` and `roughness` are a normalized float; so their real value would be x / 65535.f. 

`t_x` are the textures of the material; these index through the textures created and the index is only expected to be valid if the texture is enabled in textureFlags.

### MLMaterialList

A material list is an `Array<u16>`, an index to the materials. It is indexed using the triangle's material id. This array has the size of header.materialsPerList. 

### MLFile

```cpp
struct MLFile {
  
    MLHeader header;
    
    Array<MLTexture> textures;		//header.textures
    CopyBuffer textureData;			//sum(textures[i].size)
    
    Array<MLMaterial> materials;
    
    Array<u16> materialLists;
    
    SLFile names;					//Optional
    
};
```