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

		RMFile file;
		if (!oiRM::read(minfo.path, file))
			return (Mesh*) Log::error(String("Couldn't read mesh from file \"") + minfo.path + "\"");

		auto rmdat = oiRM::convert(file);

		if (minfo.meshBuffer == nullptr){

			minfo.meshBuffer = findBuffer(rmdat.first, minfo);

			if(minfo.meshBuffer == nullptr)
				return (Mesh*)Log::error(String("Couldn't read mesh into meshBuffer \"") + minfo.path + "\" (" + minfo.meshBuffer->getName() + "); couldn't find or allocate MeshBuffer");

		} else if (!validateBuffer(minfo, rmdat.first))
			return (Mesh*)Log::error(String("Couldn't read mesh into meshBuffer \"") + minfo.path + "\" (" + minfo.meshBuffer->getName() + ")");

		MeshInfo mi = rmdat.second;
		mi.buffer = minfo.meshBuffer;

		mi.buffer->open();
		m = minfo.mesh = g->create(minfo.path, mi);
		mi.buffer->close();

		info.meshAllocations[minfo.path] = minfo;

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

		auto it = info.meshAllocations.find(mai.path);

		if (it == info.meshAllocations.end()) {

			RMFile file;
			if (!oiRM::read(mai.path, file)) {
				Log::error(String("Couldn't load model \"") + mai.path + "\"");
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
		
		if (meshes[i] != nullptr) continue;

		MeshAllocationInfo &mai = minfo[i];

		if (mai.meshBuffer == nullptr) {
			mai.meshBuffer = findBuffer(oiRMs[i].first, mai);
			batches[mai.meshBuffer].push_back({ i, &oiRMs[i].second });
		} else if(validateBuffer(mai, oiRMs[i].first))
			batches[mai.meshBuffer].push_back({ i, &oiRMs[i].second });
		else
			Log::error(String("Couldn't read mesh into meshBuffer \"") + mai.path + "\" (" + mai.meshBuffer->getName() + ")");

	}

	t.lap("Load MeshBuffers");

	//Create Meshes

	for (auto &elem : batches) {

		MeshBuffer *meshBuffer = elem.first;
		meshBuffer->open();

		for (auto &elem0 : elem.second) {

			u32 id = elem0.first;

			MeshInfo *mi = elem0.second;
			mi->buffer = meshBuffer;

			MeshAllocationInfo &mai = minfo[id];
			mai.mesh = meshes[id] = g->create(mai.path, *mi);
			info.meshAllocations[mai.path] = mai;

		}

		meshBuffer->close();

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