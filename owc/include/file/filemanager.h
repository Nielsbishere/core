#pragma once
#include "types/string.h"
#include "platforms/generic.h"
#include <functional>

namespace oi {

	class Buffer;

	namespace wc {

		enum class FileAccess {
			QUERY,
			READ,
			WRITE
		};

		//Information about a file
		struct FileInfo {

			bool isFolder;
			String name;
			u64 fileSize;
			time_t modificationTime;	//Can be 0 for res/ files on read-only machines (check FileManager canModifyAssets)

			FileInfo(bool isFolder, String name, time_t modificationTime, u64 fileSize) : isFolder(isFolder), name(name), modificationTime(modificationTime), fileSize(fileSize) {}
			FileInfo() : FileInfo(false, "", 0, 0) {}

		};

		//Returns bool continue
		typedef std::function<bool(FileInfo)> FileCallback;

		struct FileManagerExt;

		struct ParentedFileInfo {

			String name;
			u32 dirId;

			bool operator==(const ParentedFileInfo &other) const {
				return name == other.name && dirId == other.dirId;
			}
		};

		//File reading:
		//resources (read only): res/
		//files (read write): out/
		//resources (write only): mod/			(PC only)
		class FileManager {

			friend struct FileManagerExt;

		public:

			static const FileManager *get();

			FileManager(AppExt *app);
			~FileManager();

			bool read(String path, String &s) const;
			bool read(String path, Buffer &b) const;

			bool write(String path, String &s) const;
			bool write(String path, Buffer b) const;

			bool mkdir(String path) const;
			bool exists(String path) const;
			bool fileExists(String path) const;
			bool dirExists(String path) const;

			bool validate(String path, FileAccess access) const;
			bool foreachFile(String path, FileCallback callback) const;				//Loop through a directory
			bool foreachFileRecurse(String path, FileCallback callback) const;		//Loop through a directory (and its directories, recursively)

			bool canModifyAssets() const;											//Whether or not the mod/ dir can be used

			String getAbsolutePath(String path) const;

			FileInfo getFile(String path) const;									//Get the file info

		protected:

			void init();

		private:

			static FileManager *instance;
			AppExt *param;

			std::vector<String> dirs;
			std::vector<ParentedFileInfo> files;
		};

	}

}