#include "format/oisl.h"

namespace oi {

	namespace gc {

		DEnum(BMFlag, u16, None = 0, StripDebug = 1);

		struct BMHeader {

			char header[4];		//oiBM

			u16 files;
			u16 flags;			//BMFlag

			u32 version;

			//If any change is made to the baker; this increases
			static const u32 getGlobalVersion() { return 1; }

		};
		
		struct BMBakedFile {
			u8 inputs;
			u8 outputs;
			u16 dependencies;
		};

		struct BakedFile {
			String extension, file;
			std::vector<String> outputs, inputs, dependencies;
		};

		struct BakerFile {

			BMHeader header;

			std::vector<BakedFile> files;

			SLFile strings;

		};

		typedef std::vector<std::pair<String, std::vector<String>>> BakeTypes;
		typedef std::function<bool(BakedFile&, bool stripDebug)> BakeFunction;

		struct BakeOption {

			String type;
			String path;

			BakeTypes inputExtensions;
			BakeFunction bake;

			BakeOption(String type, String path, BakeTypes inputExtensions, BakeFunction bake) : type(type), path(path), inputExtensions(inputExtensions), bake(bake) {}

		};

		class BakeManager {

		public:

			BakeManager(bool stripDebug, String file = "mod/baker.oiBM");

			//Returns how many options have failed (0 if success)
			int run();

		protected:

			void load();
			void write();
			void cache(BakedFile &file);
			bool shouldUpdate(BakedFile &file);

			u16 makeFlags();

			static bool bakeModel(BakedFile &file, bool stripDebug);
			static bool bakeShader(BakedFile &file, bool stripDebug);

		private:

			std::vector<BakeOption> bakeOptions;
			BakerFile file;
			String location;
			bool changed = false, stripDebug;

		};

	}

}