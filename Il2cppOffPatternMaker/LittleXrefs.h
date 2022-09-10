#pragma once
#include <string>
#include <json\json.h>
#include <fstream>

#define ASK_FILES

namespace LX{

	#define LXFILE_MODE_IN std::ios::in
	#define LXFILE_MODE_IN_BINARY std::ios::in | std::ios::ate | std::ios::binary

	enum LXERROR
	{
		LX_OK,
		LX_FILESLOAD,
		LX_DISASM,
		LX_ARCH_NO_SUPPORTED
	};

	enum LXARCH {
		UNKNOWN = -1,
		ARM32,
		ARM64,
		PE32,
		PE64
	};

	enum LXFILEFORMAT {
		ELF,
		PE
	};

	class LittleXrefs {
	private:
		unsigned char*	m_AssemblyBuffEntry;
		uintptr_t		m_AssemblyBuffSize;
		Json::Value		m_ScriptJsonObj;
		LXARCH			m_Arch;
		LXFILEFORMAT	m_ExecFileFormat;

	public:
		LittleXrefs();
		~LittleXrefs();
		bool LoadFiles();
		Json::Value&	getDumpJsonObj();
		unsigned char*	getAssemblyEntry();
		uintptr_t getAssemblySize();
	};

	LXERROR MakeLittleXrefs(LittleXrefs** pLXrefs);

	namespace Utils {
		bool get_assembly_path(std::wstring& out_path);
		bool get_script_path(std::wstring& out_path);
		bool cstr_to_json_obj(const char* json_char_buff, Json::Value& json_obj);

		class LXFile {
		private:
			std::fstream* m_FileStream;

		public:
			LXFile();
			LXFile(const std::wstring& path, uintptr_t mode);
			~LXFile();

			bool isOpen();
			size_t getFileSize();
			bool ReadFile(void* buff, uintptr_t buffSize);
		};
	}
}

