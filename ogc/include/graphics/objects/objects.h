#pragma once
#include "graphics/objects/graphicsobjectref.h"
#include "graphics/objects/texture/texture.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/texture/texturelist.h"
#include "graphics/objects/texture/sampler.h"
#include "graphics/objects/view/view.h"
#include "graphics/objects/view/viewbuffer.h"
#include "graphics/objects/view/camera.h"
#include "graphics/objects/view/camerafrustum.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/shader/shaderbuffer.h"
#include "graphics/objects/shader/pipeline.h"
#include "graphics/objects/shader/pipelinestate.h"
#include "graphics/objects/shader/computelist.h"
#include "graphics/objects/model/material.h"
#include "graphics/objects/model/materiallist.h"
#include "graphics/objects/model/meshmanager.h"
#include "graphics/objects/model/meshbuffer.h"
#include "graphics/objects/render/commandlist.h"
#include "graphics/objects/render/drawlist.h"
#include "graphics/objects/render/rendertarget.h"

namespace oi {

	namespace gc {

		typedef TGraphicsObjectRef<Texture> TextureRef;
		typedef TGraphicsObjectRef<TextureList> TextureListRef;
		typedef TGraphicsObjectRef<VersionedTexture> VersionedTextureRef;
		typedef TGraphicsObjectRef<ViewBuffer> ViewBufferRef;
		typedef TGraphicsObjectRef<Camera> CameraRef;
		typedef TGraphicsObjectRef<CameraFrustum> CameraFrustumRef;
		typedef TGraphicsObjectRef<View> ViewRef;
		typedef TGraphicsObjectRef<Shader> ShaderRef;
		typedef TGraphicsObjectRef<Sampler> SamplerRef;
		typedef TGraphicsObjectRef<RenderTarget> RenderTargetRef;
		typedef TGraphicsObjectRef<PipelineState> PipelineStateRef;
		typedef TGraphicsObjectRef<Pipeline> PipelineRef;
		typedef TGraphicsObjectRef<MeshManager> MeshManagerRef;
		typedef TGraphicsObjectRef<MeshBuffer> MeshBufferRef;
		typedef TGraphicsObjectRef<MaterialList> MaterialListRef;
		typedef TGraphicsObjectRef<Material> MaterialRef;
		typedef TGraphicsObjectRef<ShaderBuffer> ShaderBufferRef;
		typedef TGraphicsObjectRef<GPUBuffer> GPUBufferRef;
		typedef TGraphicsObjectRef<DrawList> DrawListRef;
		typedef TGraphicsObjectRef<ComputeList> ComputeListRef;
		typedef TGraphicsObjectRef<CommandList> CommandListRef;

	}

}