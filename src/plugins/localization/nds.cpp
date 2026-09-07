#include "nds.h"

#include <string>
#include <algorithm>
#include <memory>
#include <stdexcept>

#include "utils.h"

#define HEADERCOUNT 8
#define OVERLAY_FMT	"/FSI.CT/overlay%d_%04d.bin"

namespace NDS {

using BYTE = int8_t;
using UINT = uint32_t;
using WORD = int16_t;

#pragma pack(push, 1)
struct NDSHEADER
{
	char GameTitle[0x0C];
	char GameCode[0x04];
	char MakerCode[0x02];
	BYTE UnitCode;
	BYTE DeviceCode;						// type of device in the game card
	BYTE DeviceCap;							// device capacity (128kb<<n Mbit)
	BYTE Reserved_0x015[0x09];				// (zero filled)
	BYTE RomVersion;
	BYTE Autostart;							// (Bit2: Skip "Press Button" after Health and Safety) (Also skips bootmenu, even in Manual mode & even Start pressed)
	UINT Arm9_Rom_Offset;					// copy src
	UINT Arm9_Entry_Address;				// entry point
	UINT Arm9_Ram_Address;					// copy dst
	UINT Arm9_Size;							// size
	UINT Arm7_Rom_Offset;
	UINT Arm7_Entry_Address;
	UINT Arm7_Ram_Address;
	UINT Arm7_Size;
	UINT Fnt_Offset;
	UINT Fnt_Size;
	UINT Fat_Offset;
	UINT Fat_Size;
	UINT Arm9_Overlay_Offset;
	UINT Arm9_Overlay_Size;
	UINT Arm7_Overlay_Offset;
	UINT Arm7_Overlay_Size;
	UINT Port_40001A4h_Normal_Commands;		// Port 40001A4h setting for normal commands (usually 00586000h)
	UINT Port_40001A4h_KEY1_Commands;		// Port 40001A4h setting for KEY1 commands   (usually 001808F8h)
	UINT Banner_Offset;
	WORD Secure_Area_CRC;
	WORD Secure_Area_Loading_Timeout;
	UINT ARM9_Auto_Load_List_RAM_Address;	// ?
	UINT ARM7_Auto_Load_List_RAM_Address;	// ?
	UINT Secure_Area_Disable1;				// unique ID for homebrew
	UINT Secure_Area_Disable2;				// unique ID for homebrew
	UINT Application_End_Offset;			// Total Used ROM size
	UINT Rom_Header_Size;
	BYTE Reserved_0x088[0x38];				// Reserved (zero filled)
	BYTE Nintendo_Logo[0x9C];
	WORD Nintendo_Logo_CRC;
	WORD Header_CRC;
	UINT Debug_Rom_Offset;					// (0=none) (8000h and up)       ;only if debug
	UINT Debug_Size;						// (0=none) (max 3BFE00h)        ;version with
	UINT Debug_Ram_Address;					// (0=none) (2400000h..27BFE00h) ;SIO and 8MB
	UINT Reserved_0x16C;
	BYTE Zero[0x90];
};

struct OVERLAYENTRY
{
	UINT id;
	UINT ram_address;
	UINT ram_size;
	UINT bss_size;
	UINT sinit_init;
	UINT sinit_init_end;
	UINT file_id;
	UINT reserved;
};

struct NDSSPECREC {
	UINT nTop;
	UINT nBottom;
	std::string FileName;
};

struct NDSFILEREC
{
	UINT top;
	UINT bottom;	// size = bottom-top
};

struct NDSDIRREC
{
	UINT entry_start;
	WORD top_file_id;
	WORD parent_id_or_count;
};
#pragma pack(pop)

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1);
}

void string_tolower(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
}

std::string getFileExtension(const std::string& path)
{
	const std::size_t sep = path.find_last_of("/\\");
	const std::size_t start = (sep == std::string::npos) ? 0 : sep + 1;
	const std::size_t dot = path.find_last_of('.');

	if (dot == std::string::npos || dot < start || dot == start || dot + 1 == path.size())
		return {};

	return path.substr(dot + 1);
}

NDSEntry::NDSEntry(const std::string& in_filename, const std::string& ext, int in_start, int in_size, int in_fileId)
	: filename(in_filename)
	, type(ext)
	, start(in_start)
	, size(in_size)
	, maxSize(in_size)
	, fileId(in_fileId)
{
	string_tolower(type);
}

NDSFileSystem::NDSFileSystem(uint8_t* rom, uint32_t romSize)
	: m_pRom (rom)
	, m_nRomSize(romSize)
{
}

void NDSFileSystem::getRomFileSystem(bool bSkipOverlays)
{
	auto* m_pHeader = (NDSHEADER*)m_pRom;

	std::string OverlayFiles = "/FSI.CT/";

	UINT nArm9_Size = m_pHeader->Arm9_Size;
	if (*(UINT*)(m_pRom + m_pHeader->Arm9_Rom_Offset + nArm9_Size) == 0xDEC00621)
		nArm9_Size += 4 * 4;	// arm9.bin with_footer

	NDSSPECREC SpecRec[HEADERCOUNT] = {
		//Top	Bottom			Name
		{0, sizeof(NDSHEADER), OverlayFiles + "/FSI.CT/ndsheader.bin" },
		{m_pHeader->Arm9_Rom_Offset, m_pHeader->Arm9_Rom_Offset + nArm9_Size, OverlayFiles + "arm9.bin" },
		{m_pHeader->Arm7_Rom_Offset, m_pHeader->Arm7_Rom_Offset + m_pHeader->Arm7_Size, OverlayFiles + "arm7.bin" },
		{m_pHeader->Arm9_Overlay_Offset, m_pHeader->Arm9_Overlay_Offset + m_pHeader->Arm9_Overlay_Size, OverlayFiles + "arm9ovltable.bin" },
		{m_pHeader->Arm7_Overlay_Offset, m_pHeader->Arm7_Overlay_Offset + m_pHeader->Arm7_Overlay_Size, OverlayFiles + "arm7ovltable.bin" },
		{m_pHeader->Fnt_Offset, m_pHeader->Fnt_Offset + m_pHeader->Fnt_Size, OverlayFiles + "fnt.bin" },
		{m_pHeader->Fat_Offset, m_pHeader->Fat_Offset + m_pHeader->Fat_Size, OverlayFiles + "fat.bin" },
		{m_pHeader->Banner_Offset, m_pHeader->Banner_Offset + 0x840, OverlayFiles + "banner.bin" }
	};

	if (!bSkipOverlays)
	{
		UINT m_nOverlayFiles9 = m_pHeader->Arm9_Overlay_Size / sizeof(OVERLAYENTRY);
		UINT m_nOverlayFiles7 = m_pHeader->Arm7_Overlay_Size / sizeof(OVERLAYENTRY);

		UINT m_nOverlayFileSize = 0;
		OVERLAYENTRY OverlayEntry;
		NDSFILEREC* pFileRec;
		for (UINT i = 0; i < m_nOverlayFiles9; i++)
		{
			OverlayFiles = string_format(OVERLAY_FMT, 9, i);
			memcpy(&OverlayEntry, m_pRom + m_pHeader->Arm9_Overlay_Offset + sizeof(OverlayEntry) * i, sizeof(OverlayEntry));

			pFileRec = (NDSFILEREC*)(m_pRom + m_pHeader->Fat_Offset + (OverlayEntry.file_id << 3));
			m_nOverlayFileSize += pFileRec->bottom - pFileRec->top;

			addEntry(OverlayFiles, pFileRec->top, pFileRec->bottom - pFileRec->top, static_cast<int>(OverlayEntry.file_id));
		}

		for (UINT i = 0; i < m_nOverlayFiles7; i++)
		{
			OverlayFiles = string_format(OVERLAY_FMT, 7, i);
			memcpy(&OverlayEntry, m_pRom + m_pHeader->Arm7_Overlay_Offset + sizeof(OverlayEntry) * i, sizeof(OverlayEntry));

			pFileRec = (NDSFILEREC*)(m_pRom + m_pHeader->Fat_Offset + (OverlayEntry.file_id << 3));
			m_nOverlayFileSize += pFileRec->bottom - pFileRec->top;

			addEntry(OverlayFiles, pFileRec->top, pFileRec->bottom - pFileRec->top, static_cast<int>(OverlayEntry.file_id));
		}
	}

	OverlayFiles = "/";
	extractDirectory(OverlayFiles);

	computeMaxSizes();
}

void NDSFileSystem::computeMaxSizes()
{
	if (m_foundEntries.empty())
		return;

	std::vector<NDSEntry*> sorted;
	sorted.reserve(m_foundEntries.size());
	for (auto& entry : m_foundEntries)
		sorted.push_back(&entry);

	std::sort(sorted.begin(), sorted.end(), [](const NDSEntry* a, const NDSEntry* b) { return a->start < b->start; });

	size_t i = 0;
	while (i < sorted.size())
	{
		size_t next = i;
		while (next < sorted.size() && sorted[next]->start == sorted[i]->start)
			next++;

		const int nextStart = (next < sorted.size()) ? sorted[next]->start : static_cast<int>(m_nRomSize);

		for (size_t k = i; k < next; k++)
			sorted[k]->maxSize = std::max(nextStart - sorted[k]->start, sorted[k]->size);

		i = next;
	}
}

void NDSFileSystem::extractDirectory(const std::string& ParentDir, uint8_t nID)
{
	std::string strFilePathName;
	BYTE nRecLen;
	bool bIsDir;
	std::string Dir = ParentDir;

	auto* m_pHeader = (NDSHEADER*)m_pRom;

	UINT nPos = m_pHeader->Fnt_Offset+(nID<<3);
	if(m_nRomSize < nPos)
		return;

	NDSDIRREC* pDirRec = (NDSDIRREC*)(m_pRom +nPos);
	nPos = m_pHeader->Fnt_Offset + pDirRec->entry_start;
	if(m_nRomSize < nPos)
		return;

	char m_FileName[0x80];

	const uint8_t* pRec = m_pRom + nPos;
	UINT FileID = pDirRec->top_file_id;
	while(1)
	{
		nRecLen = *(pRec++);	if(nRecLen==0) break;
		bIsDir = nRecLen&0x80; nRecLen&=0x7F;

		memcpy(m_FileName, pRec, nRecLen);
		pRec+=nRecLen;
		m_FileName[nRecLen]=0;
		if(bIsDir)
		{
			WORD DirID = *(WORD*)(pRec);DirID&=0xFFF;
			pRec+=2;
			Dir = string_format("%s%s/", ParentDir.c_str(), m_FileName);
			extractDirectory(Dir, DirID);
			continue;
		}
		NDSFILEREC *pFileRec;
		pFileRec = (NDSFILEREC*)(m_pRom+m_pHeader->Fat_Offset+(FileID<<3));
		nPos = pFileRec->bottom - pFileRec->top;

		strFilePathName = string_format("%s%s", ParentDir.c_str(), m_FileName);

		addEntry(strFilePathName, pFileRec->top, nPos, static_cast<int>(FileID));
		FileID++;
	}
}

void NDSFileSystem::addEntry(std::string& path, int start, int size, int fileId)
{
	auto ext = getFileExtension(path);
	if (ext.empty())
	{
		auto type = utils::getFileFormat(m_pRom + start, size);
		ext = utils::getExtName(type);
	}

	m_foundEntries.emplace_back(path, ext, start, size, fileId);
}

NDSEntry* NDSFileSystem::findEntryByOffset(uint32_t offset)
{
	for (auto& entry : m_foundEntries)
	{
		if (offset >= entry.start && offset <= entry.start + entry.size)
			return &entry;
	}

	return nullptr;
}

NDSEntry* NDSFileSystem::findEntryByName(const std::string& name)
{
	for (auto& entry : m_foundEntries)
	{
		if (entry.filename.find(name) != std::string::npos)
			return &entry;
	}

	return nullptr;
}

bool NDSFileSystem::patchEntrySize(NDSEntry& entry, int newSize)
{
	if (m_pRom == nullptr)
		return false;

	if (entry.fileId < 0)
		return false;

	if (newSize < 0 || newSize > entry.maxSize)
		return false;

	auto* pHeader = (const NDSHEADER*)m_pRom;

	const UINT nPos = pHeader->Fat_Offset + (static_cast<UINT>(entry.fileId) << 3);
	if (nPos + sizeof(NDSFILEREC) > m_nRomSize)
		return false;

	NDSFILEREC* pFileRec = (NDSFILEREC*)(m_pRom + nPos);
	if (pFileRec->top != static_cast<UINT>(entry.start))
		return false;

	pFileRec->bottom = pFileRec->top + static_cast<UINT>(newSize);
	entry.size = newSize;

	return true;
}

} // namespace NDS
