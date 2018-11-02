#include "file/filemanager.h"
#include "graphics/format/obj.h"
#include "graphics/format/fbx.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

int main() {

	FileManager fm(nullptr);
	int res = 0;

	fm.foreachFileRecurse("mod/models", [&](FileInfo info) -> bool {

		if (!info.name.endsWithIgnoreCase(".obj") && !info.name.endsWithIgnoreCase(".fbx"))
			return false;

		String target = info.name.getFilePath() + ".oiRM";

		if (fm.fileExists(target)) {

			FileInfo targ = fm.getFile(target);
			FileInfo curr = fm.getFile(info.name);

			if (targ.modificationTime > curr.modificationTime) {
				Log::println(target + " is already up-to-date");
				return false;
			}
		}

		if(info.name.endsWithIgnoreCase(".obj")){

			if (!Obj::convert(info.name, target, true)) {
				res = 1;
				return !Log::error("Couldn't convert obj to oiRM");
			}

			Log::println(target + " has been updated successfully");

		} else if (info.name.endsWithIgnoreCase(".fbx")) {

			if (!Fbx::convertMeshes(info.name, target, true)) {
				res = 1;
				return !Log::error("Couldn't convert fbx to oiRM");
			}

			Log::println(target + " has been updated successfully");

		}

		return false;

	});

	return res;

}