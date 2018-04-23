#pragma once

namespace oi {

	class String;
	class Buffer;

	namespace wc {

		class FileManager {

		public:

			static const FileManager *get();

			FileManager(void *param);
			~FileManager();

			bool read(String path, String &s) const;
			bool read(String path, Buffer &b) const;

			bool write(String path, String &s) const;
			bool write(String path, Buffer &b) const;

		private:

			static FileManager *instance;
			void *param;
		};

	}

}