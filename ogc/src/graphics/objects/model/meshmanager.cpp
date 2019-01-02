#include "utils/timer.h"
#include "graphics/format/oirm.h"
#include "graphics/objects/model/meshmanager.h"
#include "graphics/objects/model/meshbuffer.h"
#include "graphics/objects/model/mesh.h"
using namespace oi::gc;
using namespace oi;

MeshManager::MeshManager(MeshManagerInfo info) : info(info) { }
const MeshManagerInfo &MeshManager::getInfo() { return info; }
MeshManager::~MeshManager() {

	for(auto &ma : info.meshAllocations)
		g->destroy(ma.second.mesh);

	for (MeshBuffer *mb : info.meshBuffers)
		g->destroy(mb);

}

void MeshManager::unloadAll(MeshBuffer *meshBuffer) {
	unload(meshBuffer, true);
}

bool MeshManager::validateBuffer(MeshAllocationInfo &mai, MeshBufferInfo &mbi) {
	return mai.meshBuffer->canAllocate(mbi);	//TODO: Actually allocate space!
}

bool MeshManager::contains(String path) {
	return info.meshAllocations.find(path) != info.meshAllocations.end();
}

bool MeshManager::init() {

	for (MeshBuffer *mb : info.meshBuffers)
		g->use(mb);

	return true; 
}

Mesh *MeshManager::get(String path) {

	auto it = info.meshAllocations.find(path);

	if (it == info.meshAllocations.end())
		return (Mesh*) Log::error(String("Couldn't get Mesh by path \"") + path + "\"");

	return it->second.mesh;

}

MeshBuffer *MeshManager::findBuffer(MeshBufferInfo &mbi, MeshAllocationInfo &mai) { //This should allocate memory too!

	if(mai.hintMaxVertices != MeshAllocationHint::SIZE_TO_FIT && mai.hintMaxIndices != MeshAllocationHint::SIZE_TO_FIT)
		for (MeshBuffer *mb : info.meshBuffers)
			if (mb->canAllocate(mbi))
				return mb;

	if (mai.hintMaxVertices == MeshAllocationHint::FORCE_EXISTING)
		return (MeshBuffer*)Log::error("Mesh isn't allowed to create a new MeshBuffer");

	MeshBufferInfo allocation = mbi;
	allocation.fillMode = info.defFillMode;
	allocation.topologyMode = info.defTopologyMode;

	if (mai.hintMaxIndices == MeshAllocationHint::ALLOCATE_DEFAULT && mai.hintMaxVertices != MeshAllocationHint::SIZE_TO_FIT)
		allocation.maxIndices = info.indices;
	else if (mai.hintMaxVertices != MeshAllocationHint::SIZE_TO_FIT)
		allocation.maxIndices = (u32) mai.hintMaxIndices;

	if (mai.hintMaxVertices == MeshAllocationHint::ALLOCATE_DEFAULT)
		allocation.maxVertices = info.vertices;
	else if (mai.hintMaxVertices != MeshAllocationHint::SIZE_TO_FIT)
		allocation.maxVertices = (u32)mai.hintMaxVertices;

	MeshBuffer *mb = g->create(getName() + " MeshBuffer " + mbId, allocation);
	info.meshBuffers.push_back(mb);
	++mbId;

	return mb;

}

Mesh *MeshManager::load(MeshAllocationInfo minfo) {

	auto it = info.meshAllocations.find(minfo.path);

	Mesh *m;

	if (it == info.meshAllocations.end()) {

		if (minfo.path != "") {

			RMFile file;
			if (!oiRM::read(minfo.path, file))
				return (Mesh*)Log::error(String("Couldn't read mesh from file \"") + minfo.name + "\"");

			auto rmdat = oiRM::convert(file);

			if (minfo.meshBuffer == nullptr) {

				minfo.meshBuffer = findBuffer(rmdat.first, minfo);

				if (minfo.meshBuffer == nullptr)
					return (Mesh*)Log::error(String("Couldn't write mesh into meshBuffer \"") + minfo.name + "\" couldn't find or allocate MeshBuffer");

			} else if (!validateBuffer(minfo, rmdat.first))
				return (Mesh*)Log::error(String("Couldn't write mesh into meshBuffer \"") + minfo.name + "\" (" + minfo.meshBuffer->getName() + ")");

			MeshInfo mi = rmdat.second;
			mi.buffer = minfo.meshBuffer;

			m = minfo.mesh = g->create(minfo.name, mi);
			mi.buffer->flush(mi.allocation);

			info.meshAllocations[minfo.name] = minfo;

		} else {

			if (minfo.meshBuffer == nullptr)
				return (Mesh*)Log::error("Couldn't write mesh into meshBuffer; when submitting raw data, be sure to use a valid buffer");

			MeshBufferInfo mbinfo = minfo.meshBuffer->getInfo();

			if(minfo.vbos.size() != mbinfo.vbos.size() || (minfo.ibo.size() == 0) != (mbinfo.ibo == nullptr))
				return (Mesh*)Log::error("Couldn't write mesh into meshBuffer; when submitting raw data, the number of buffers (vbos, ibo) has to be the same");
			
			u32 i = 0;
			for (Buffer b : minfo.vbos) {

				if (b.size() % mbinfo.vboStrides[i] != 0)
					return (Mesh*)Log::error("Couldn't write mesh into meshBuffer; when submitting raw data, the vbo layouts have to be the same");

				++i;
			}

			mbinfo.maxVertices = minfo.vbos[0].size() / mbinfo.vboStrides[0];
			mbinfo.maxIndices = minfo.ibo.size() / 4;

			if(!validateBuffer(minfo, mbinfo))
				return (Mesh*)Log::error(String("Couldn't write mesh into meshBuffer \"") + minfo.name + "\" (" + minfo.meshBuffer->getName() + ")");

			MeshInfo mi;
			mi.buffer = minfo.meshBuffer;
			mi.indices = mbinfo.maxIndices;
			mi.vertices = mbinfo.maxVertices;
			mi.vbo = minfo.vbos;
			mi.ibo = minfo.ibo;

			m = minfo.mesh = g->create(minfo.name, mi);
			mi.buffer->flush(mi.allocation);

			info.meshAllocations[minfo.name] = minfo;

		}


	} else m = it->second.mesh;

	g->use(m);
	return m;

}

std::vector<Mesh*> MeshManager::loadAll(std::vector<MeshAllocationInfo> &minfo) {

	Timer t;

	//Get all loaded meshes or load them from disk

	std::vector<Mesh*> meshes(minfo.size());
	memset(meshes.data(), 0, sizeof(Mesh*) * meshes.size());

	std::vector<std::pair<MeshBufferInfo, MeshInfo>> oiRMs(minfo.size());
	
	u32 i = 0;
	for (const MeshAllocationInfo &mai : minfo) {

		auto it = info.meshAllocations.find(mai.name);

		if (it == info.meshAllocations.end()) {

			if (mai.path == "") {

				if (mai.meshBuffer == nullptr) {
					Log::error("Couldn't write mesh into meshBuffer; when submitting raw data, be sure to use a valid buffer");
					continue;
				}

				MeshBufferInfo mbinfo = mai.meshBuffer->getInfo();

				if (mai.vbos.size() != mbinfo.vbos.size() || (mai.ibo.size() == 0) != (mbinfo.ibo == nullptr)) {
					Log::error("Couldn't write mesh into meshBuffer; when submitting raw data, the number of buffers (vbos, ibo) has to be the same");
					continue;
				}

				u32 j = 0;
				bool invalidVbo = false;

				for (Buffer b : mai.vbos) {

					if (b.size() % mbinfo.vboStrides[j] != 0) {
						Log::error("Couldn't write mesh into meshBuffer; when submitting raw data, the vbo layouts have to be the same");
						invalidVbo = true;
						break;
					}

					++j;
				}

				if (invalidVbo) continue;

				mbinfo.maxVertices = mai.vbos[0].size() / mbinfo.vboStrides[0];
				mbinfo.maxIndices = mai.ibo.size() / 4;
				
				MeshInfo mi;
				mi.buffer = mai.meshBuffer;
				mi.indices = mbinfo.maxIndices;
				mi.vertices = mbinfo.maxVertices;
				mi.vbo = mai.vbos;
				mi.ibo = mai.ibo;

				oiRMs[i] = { mbinfo, mi };
				continue;
			}

			RMFile file;
			if (!oiRM::read(mai.path, file)) {
				Log::error(String("Couldn't load model \"") + mai.name + "\"");
				continue;
			}

			oiRMs[i] = oiRM::convert(file);

		} else 
			meshes[i] = it->second.mesh;

		++i;
	}

	t.lap("Load models from disk");

	//Create MeshBuffers

	std::unordered_map<MeshBuffer*, std::vector<std::pair<u32, MeshInfo*>>> batches;

	for (i = 0; i < (u32)minfo.size(); ++i) {
		
		if (meshes[i] != nullptr || oiRMs[i].second.vbo.size() == 0) continue;

		MeshAllocationInfo &mai = minfo[i];	//What if oiRM not loaded?

		if (mai.meshBuffer == nullptr) {

			mai.meshBuffer = findBuffer(oiRMs[i].first, mai);
			batches[mai.meshBuffer].push_back({ i, &oiRMs[i].second });

		} else if (validateBuffer(mai, oiRMs[i].first))
			batches[mai.meshBuffer].push_back({ i, &oiRMs[i].second });
		else
			Log::error(String("Couldn't write mesh into meshBuffer \"") + mai.name + "\" (" + mai.meshBuffer->getName() + ")");

	}

	t.lap("Load MeshBuffers");

	//Create Meshes

	for (auto &elem : batches) {

		MeshBuffer *meshBuffer = elem.first;

		for (auto &elem0 : elem.second) {

			i = elem0.first;

			MeshInfo *mi = elem0.second;
			mi->buffer = meshBuffer;

			MeshAllocationInfo &mai = minfo[i];
			mai.mesh = meshes[i] = g->create(mai.name, *mi);
			info.meshAllocations[mai.name] = mai;

		}

	}

	t.lap("Load Meshes");
	t.print();

	return meshes;


}

void MeshManager::unload(Mesh *mesh) {

	if (mesh == nullptr) return;

	String path = "";
	MeshBuffer *mb = mesh->getBuffer();

	for (auto &ma : info.meshAllocations)
		if (ma.second.mesh == mesh) {

			g->destroy(mesh);

			if (mesh == nullptr) {	//It has been removed
				path = ma.first;
				break;
			}

		}

	if (path != "") {
		info.meshAllocations.erase(path);
		unload(mb, false);
	}

}

void MeshManager::unload(MeshBuffer *meshBuffer, bool children) {

	if (meshBuffer == nullptr) return;

	u32 i = 0;
	for (MeshBuffer *mb : info.meshBuffers)
		if (mb == meshBuffer) break;
		else ++i;

	if (i == (u32)info.meshBuffers.size())
		return;

	if (children) {

		std::vector<String> toErase;

		for (auto &ma : info.meshAllocations)
			if (ma.second.meshBuffer == meshBuffer) {
				g->destroy(ma.second.mesh);
				toErase.push_back(ma.first);
			}

		for (String st : toErase)
			info.meshAllocations.erase(st);

	} else {

		bool contains = false;
		for (auto &ma : info.meshAllocations)
			if (ma.second.meshBuffer == meshBuffer) {
				contains = true;
				break;
			}

		if (contains)
			return;

	}

	g->destroy(meshBuffer);
	info.meshBuffers.erase(info.meshBuffers.begin() + i);

}