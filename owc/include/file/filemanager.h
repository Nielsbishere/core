#pragma once

namespace oi {

	class String;
	class Buffer;

	namespace wc {

		//File reading:
		//resources (read only): res/
		//files (read write): out/
		class FileManager {

		public:

			static const FileManager *get();

			FileManager(void *param);
			~FileManager();

			bool read(String path, String &s) const;
			bool read(String path, Buffer &b) const;

			bool write(String path, String &s) const;
			bool write(String path, Buffer b) const;

			bool mkdir(String path) const;
			bool exists(String path) const;
			bool fileExists(String path) const;
			bool dirExists(String path) const;

			String getAbsolutePath(String path) const;

		private:

			static FileManager *instance;
			void *param;
		};

	}

}