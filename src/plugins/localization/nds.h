#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace NDS {

struct NDSEntry
{
	NDSEntry(const std::string& in_filename, const std::string& ext, int in_start, int in_size, int in_fileId);

	std::string filename;
	std::string type;
	int start = 0;
	int size = 0;
	int maxSize = 0;
	int fileId = -1;
};

class NDSFileSystem
{
public:
	NDSFileSystem(uint8_t* rom, uint32_t romSize);

	void getRomFileSystem(bool bSkipOverlays = false);

	void extractDirectory(const std::string& ParentDir, uint8_t nID = 0);

	void computeMaxSizes();

	NDSEntry* findEntryByOffset(uint32_t offset);
	NDSEntry* findEntryByName(const std::string& name);

	bool patchEntrySize(NDSEntry& entry, int newSize);
	const std::vector<NDSEntry>& getAllEntries() const { return m_foundEntries; }

private:
	void addEntry(std::string& path, int start, int size, int fileId);

	uint8_t* m_pRom = nullptr;
	uint32_t m_nRomSize = 0;

	std::vector<NDSEntry> m_foundEntries;
};

} // namespace NDS
