#include "graphics/objects/shader/shader.h"
#include "graphics/graphics.h"
#include "graphics/objects/texture/sampler.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/texture/texturelist.h"
#include "graphics/format/oish.h"
using namespace oi::gc;
using namespace oi;

Shader::~Shader() {

	for (ShaderStage *stage : info.stage)
		g->destroy(stage);

	destroyData();
}

Shader::Shader(ShaderInfo info) : info(info) { }
const ShaderInfo &Shader::getInfo() const { return info; }
Vec3u Shader::getComputeThreads() const { return info.computeThreads; }

bool Shader::isCompatible(ShaderStageType t0, ShaderStageType t1) {

	if (t0 == ShaderStageType::Any_hit_shader || t0 == ShaderStageType::Closest_hit_shader || t0 == ShaderStageType::Intersection_shader)
		return t1 == ShaderStageType::Any_hit_shader || t1 == ShaderStageType::Closest_hit_shader || t1 == ShaderStageType::Intersection_shader;

	if (t0 == ShaderStageType::Vertex_shader || t0 == ShaderStageType::Fragment_shader || t0 == ShaderStageType::Geometry_shader ||
		t0 == ShaderStageType::Tesselation_shader || t0 == ShaderStageType::Tesselation_evaluation_shader)

		return	t1 == ShaderStageType::Vertex_shader || t1 == ShaderStageType::Fragment_shader || t1 == ShaderStageType::Geometry_shader ||
		t1 == ShaderStageType::Tesselation_shader || t1 == ShaderStageType::Tesselation_evaluation_shader;

	return t0 == t1;
}

bool Shader::init() {

	if (info.stages.size() == 0) {

		SHFile file;

		if (!oiSH::read(info.path, file))
			return (Shader*)Log::throwError<Shader, 0x0>("Couldn't read shader");

		String path = info.path;

		info = oiSH::convert(g, file);

	} else {

		for (ShaderStageInfo &inf : info.stages) {
			if (inf.type == ShaderStageType::Vertex_shader)
				info.inputs = inf.input;
			else if (inf.type == ShaderStageType::Fragment_shader)
				info.outputs = inf.output;
		}

		if (info.stage.size() == 0) {

			info.stage.resize(info.stages.size());

			for (u32 i = 0, j = (u32)info.stages.size(); i < j; ++i)
				info.stage[i] = g->create(info.path + " " + info.stages[i].type.getName(), info.stages[i]);

		}

	}
	
	for (ShaderStageInfo &sinfo0 : info.stages)
		for (ShaderStageInfo &sinfo1 : info.stages)
			if (!Shader::isCompatible(sinfo0.type, sinfo1.type))
				return Log::error("Shader stage types are incompatible; meaning the shader is invalid");

	for (ShaderStage *ss : info.stage)
		g->use(ss);

	return initData();
}