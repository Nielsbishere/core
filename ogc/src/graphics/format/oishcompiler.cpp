#include "file/filemanager.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/StandAlone/ResourceLimits.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "graphics/helper/spvhelper.h"
#include "graphics/format/oish.h"
#include "graphics/objects/shader/shader.h"
#undef ERROR
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

SHFile oiSH::convert(ShaderSource &source, bool stripDebug) {
	std::vector<String> deps;
	return convert(source, deps, stripDebug);
}

SHFile oiSH::convert(ShaderSource &source, std::vector<String> &dependencies, bool stripDebug) {

	ShaderInfo info = compile(source, dependencies, stripDebug);

	if (info.path == "") {
		Log::error("Couldn't compile to oiSH file");
		return {};
	}

	return oiSH::convert(info);
}

ShaderInfo oiSH::compile(ShaderSource &source, bool stripDebug) {
	std::vector<String> deps;
	return compile(source, deps, stripDebug);
}

//Allow including files through our FileManager
struct FileIncluder : glslang::TShader::Includer {

	String base;
	std::string currentFile;
	std::vector<String> &dependencies;

	FileIncluder(String base, std::vector<String> &dependencies) : base(base), dependencies(dependencies) {}
	~FileIncluder() {}

	//#include <x>
	//Include from res/shaders
	IncludeResult *includeSystem(const char *headerName, const char*, size_t inclusionDepth) override { 

		if (inclusionDepth >= 256)
			return (IncludeResult*)Log::error(String("Couldn't read included file \"") + headerName + "\" there were more than 256 nested includes");

		currentFile = std::string("res/shaders/") + headerName;
		
		if (std::find(dependencies.begin(), dependencies.end(), currentFile) == dependencies.end())
			dependencies.push_back(currentFile);

		Buffer buf;

		if (!FileManager::get()->read(currentFile, buf))
			return (IncludeResult*)Log::error(String("Couldn't read included file \"") + headerName + "\" aka " + currentFile);

		return new IncludeResult(currentFile, (const char*)buf.addr(), buf.size(), nullptr);

	}

	//#include "x"
	//Include from relative dir
	IncludeResult *includeLocal(const char *headerName, const char *includerName, size_t inclusionDepth) override {

		if (inclusionDepth >= 256)
			return (IncludeResult*) Log::error(String("Couldn't read included file \"") + headerName + "\" there were more than 256 nested includes");

		String includePath = String(includerName).getPath();

		if (includePath == "")
			includePath = base;

		if (includePath == "")
			return (IncludeResult*) Log::error("Please supply a valid include base path");

		currentFile = includePath.toStdString() + "/" + headerName;

		if (std::find(dependencies.begin(), dependencies.end(), currentFile) == dependencies.end())
			dependencies.push_back(currentFile);

		Buffer buf;

		if (!FileManager::get()->read(currentFile, buf))
			return (IncludeResult*) Log::error(String("Couldn't read included file \"") + headerName + "\" aka " + currentFile);

		return new IncludeResult(currentFile, (const char*) buf.addr(), buf.size(), nullptr);
	}

	void releaseInclude(IncludeResult *ir) override { 
		if (ir != nullptr) { 
			delete[] ir->headerData;
			delete ir; 
		}
	}

};

//Allow compiling shaders
bool oiSH::compileSource(ShaderSource &source, bool useFile, std::vector<String> &dependencies) {

	constexpr u32 vulkanVersion = 100;
	const String shaderVersion = "450";

	glslang::SpvOptions spvOptions;
	spvOptions.disableOptimizer = false;
	spvOptions.optimizeSize = true;
	spvOptions.validate = true;

	glslang::TProgram shaderProg;
	std::vector<glslang::TShader*> shaderStages;

	EShMessages compileFlags = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	ShaderSourceType lang = source.type;

	if (lang == ShaderSourceType::HLSL)
		compileFlags = EShMessages(compileFlags | EShMsgReadHlsl | EShMsgHlslOffsets | EShMsgHlslEnable16BitTypes | EShMsgHlslLegalization);

	glslang::InitializeProcess();

	u32 len = (u32)(useFile ? source.files.size() : source.src.size());
	auto it = source.src.begin();

	for (u32 i = 0; i < len; ++i){

		String ext, basePath;

		if(useFile){

			String s = source.files[i];

			ext = s.getExtension();

			if (ext.equalsIgnoreCase("glsl")) {

				if (lang != ShaderSourceType::GLSL) {

					for (glslang::TShader *stage : shaderStages)
						delete stage;

					return Log::error("Couldn't compile; shader type wasn't set to GLSL but it contained a GLSL source file");

				}

				ext = s.untilLast(".").fromLast(".");

			} else if (ext.equalsIgnoreCase("hlsl")) {

				if (lang != ShaderSourceType::HLSL) {

					for (glslang::TShader *stage : shaderStages)
						delete stage;

					return Log::error("Couldn't compile; shader type wasn't set to HLSL but it contained a HLSL source file");

				}

				ext = s.untilLast(".").fromLast(".");
			}

			if (!FileManager::get()->read(s, source.src[ext])) {

				for (glslang::TShader *stage : shaderStages)
					delete stage;

				return Log::error(String("Couldn't add stage to shader; wrong extension \"") + s + "\"");
			}

			basePath = s.getPath();

		} else ext = it->first;

		ShaderStageType type = SpvHelper::pickType(ext);

		if (type == ShaderStageType::Undefined) {
			
			for (glslang::TShader *stage : shaderStages)
				delete stage;

			return Log::error(String("Couldn't add stage to shader; wrong extension \"") + ext + "\"");
		}

		EShLanguage shaderType = EShLangCompute;

		switch (type.getValue()) {

		case ShaderStageType::Compute_shader.value:
			break;

		case ShaderStageType::Fragment_shader.value:
			shaderType = EShLangFragment;
			break;

		case ShaderStageType::Vertex_shader.value:
			shaderType = EShLangVertex;
			break;

		case ShaderStageType::Geometry_shader.value:
			shaderType = EShLangGeometry;
			break;

		default:

			for (glslang::TShader *stage : shaderStages)
				delete stage;

			return Log::error(String("Couldn't add stage to shader; wrong extension \"") + ext + "\"");

		}

		//Compile the shader

		String preamble = lang == ShaderSourceType::HLSL ? "" :
			String("#version ") + shaderVersion + String::lineEnd() +
			"#extension GL_GOOGLE_include_directive : require" + String::lineEnd() +
			"#extension GL_ARB_separate_shader_objects : enable" + String::lineEnd();

		source.src[ext] = preamble + source.src[ext];

		const char *shaderSrc = source.src[ext].toCString();
		std::string shaderSrcRes;

		glslang::TShader *shader = new glslang::TShader(shaderType);

		shader->setEnvInput(lang == ShaderSourceType::HLSL ? glslang::EShSourceHlsl : glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, vulkanVersion);
		shader->setEntryPoint("main");

		TBuiltInResource resources = glslang::DefaultTBuiltInResource;

		shader->setStrings(&shaderSrc, 1);

		FileIncluder fileIncluder(basePath, dependencies);

		if (!shader->preprocess(&resources, vulkanVersion, EProfile::ENoProfile, false, false, compileFlags, &shaderSrcRes, fileIncluder)) {

			Log::print(shader->getInfoLog(), LogLevel::ERROR);

			for (glslang::TShader *stage : shaderStages)
				delete stage;

			return Log::error("Couldn't add stage to shader; couldn't preprocess shader");
		}

		source.src[ext] = shaderSrcRes;

		const char *postShaderSrc = source.src[ext].toCString();

		shader->setStrings(&postShaderSrc, 1);

		if (!shader->parse(&resources, vulkanVersion, false, compileFlags)) {

			Log::print(shader->getInfoLog(), LogLevel::ERROR);

			for (glslang::TShader *stage : shaderStages)
				delete stage;

			return Log::error(String("Couldn't add stage to shader; couldn't parse shader \"") + source.src[ext] + "\"");
		}

		shaderProg.addShader(shader);
		shaderStages.push_back(shader);

		std::vector<u32> spv;
		glslang::GlslangToSpv(*shader->getIntermediate(), spv, &spvOptions);

		if (spv.size() == 0) {

			for (glslang::TShader *stage : shaderStages)
				delete stage;

			return Log::error("Couldn't add stage to shader; couldn't convert to spirv");
		}

		source.spv[ext] = CopyBuffer((u8*)spv.data(), (u32)spv.size() * 4);

		if (!useFile)
			++it;

	}

	if (!shaderProg.link(compileFlags)) {

		Log::print(shaderProg.getInfoLog(), LogLevel::ERROR);

		for (glslang::TShader *stage : shaderStages)
			delete stage;

		return Log::error("Couldn't link shader");
	}

	for (glslang::TShader *stage : shaderStages)
		delete stage;

	glslang::FinalizeProcess();

	return true;
}

ShaderInfo oiSH::compile(ShaderSource &source, std::vector<String> &dependencies, bool stripDebug) {

	//Fetch source from files
	if (source.getFiles().size() != 0) {

		switch (source.getType()) {

		case ShaderSourceType::SPV:

			for (const String &s : source.getFiles()) {

				String ext = s.getExtension();

				if (ext.equalsIgnoreCase("spv"))
					ext = s.untilLast(".").fromLast(".");

				ShaderStageType type = SpvHelper::pickType(ext);

				if (type == ShaderStageType::Undefined || !FileManager::get()->read(s, source.spv[ext])) {
					Log::error("Couldn't add stage to shader; spirv was invalid or had the wrong extension");
					return {};
				}
			}

		case ShaderSourceType::GLSL:
		case ShaderSourceType::HLSL:

			//Compile shader files
			if (!compileSource(source, true, dependencies)) {
				Log::error("Couldn't convert shader source file(s) to SPV");
				return {};
			}

			break;

		default:

			Log::error("Couldn't add stage to shader; the ShaderSourceType isn't supported yet");
			return {};

		}

	}

	//Compile shader source (only) to SPIRV
	if (source.getType() != ShaderSourceType::SPV && source.files.size() == 0) {
		if (!compileSource(source, false, dependencies)) {
			Log::error("Couldn't convert shader source file(s) to SPV");
			return {};
		}
	}

	ShaderInfo info;
	info.path = source.getName();

	for (auto &elem : source.spv) {

		ShaderStageType type = SpvHelper::pickType(elem.first);

		if (!SpvHelper::addStage(elem.second, type, info, stripDebug)) {
			Log::error("Couldn't add stage to shader");
			return {};
		}

	}

	return info;

}