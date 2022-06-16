#pragma once

#include <DirectXTex.h>

constexpr uint32_t HEADER_SIZE = 32 + 40;
constexpr uint32_t MEM_POOL_SIZE = 0x150;
constexpr uint32_t DATA_PTR_SIZE = 0x8;

constexpr uint32_t START_OF_STR_SECTION = HEADER_SIZE + MEM_POOL_SIZE + DATA_PTR_SIZE;

constexpr uint32_t STR_HEADER_SIZE = 0x14;
constexpr uint32_t EMPTY_STR_SIZE = 0x4;

constexpr uint32_t FILENAME_STR_OFFSET = START_OF_STR_SECTION + STR_HEADER_SIZE + EMPTY_STR_SIZE;

constexpr uint32_t BRTD_SECTION_START = 0xFF0;
constexpr uint32_t START_OF_TEXTURE_DATA = BRTD_SECTION_START + 16;

static char DIC_SECTION[] = "\x5F\x44\x49\x43\x01\x00\x00\x00\xFF\xFF\xFF\xFF\x01\x00\x00\x00\xB4\x01\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x01\x00\xB8\x01\x00\x00\x00\x00\x00\x00";

#pragma pack(push, 1)

// Can be memory-mapped.
struct BRTISection {
    char Magic[4] = { 'B', 'R', 'T', 'I' };
    uint32_t Size;
    uint64_t Size2;
    uint8_t  Flags;
    uint8_t  Dim;
    uint16_t TileMode;
    uint16_t Swizzle;
    uint16_t MipCount;
    uint32_t NumMultiSample;
    uint32_t Format;
    uint32_t unk2;
    uint32_t Width;
    uint32_t Height;
    uint32_t Depth;
    uint32_t ArrayLength;
    int32_t  SizeRange;
    uint32_t unk4[6];
    uint32_t ImageSize;
    uint32_t Align;
    uint32_t CompSel;
    uint32_t TY;

    uint64_t NameOffset;
    uint64_t ParentOffset;

    uint64_t UnkOffset;
    uint64_t unk5;
    uint64_t EndOffset;
    uint64_t UnkOffset2;
    uint64_t unk6;
    uint64_t unk7;
};

// Can be memory-mapped.
struct BRTDSection {
    char Magic[4] = { 'B', 'R', 'T', 'D' };
    uint32_t Padding;
    uint64_t ImageSize;
};

// Can be memory-mapped.
struct BNTXHeader {
    char Magic[4] = { 'B', 'N', 'T', 'X' };
    uint32_t Padding;
    uint16_t Version[2];
    uint16_t BOM;
    uint16_t Revision;
    uint32_t FileNameOffset;
    uint16_t Padding2;
    uint16_t STROffset;
    uint32_t RLTOffset;
    uint32_t FileSize;
};

// Can be memory-mapped.
struct NXHeader {
    char Magic[4] = { 'N', 'X', ' ', ' ' };
    uint32_t Count;
    uint64_t BRTIFarOffset;
    uint64_t BRTDOffset;
    uint64_t DICOffset;
    uint64_t DICSize;
};

// Can be memory-mapped.
struct RLTSection {
    uint64_t Offset;
    uint32_t Position;
    uint32_t Size;
    uint32_t Index;
    uint32_t Count;
};

// Can be memory-mapped.
struct RLTEntry {
    uint32_t Position;
    uint16_t StructCount;
    uint8_t  OffsetCount;
    uint8_t  PaddingCount;
};

// CANNOT be memory-mapped.
struct RLTTable {
    char Magic[4] = { '_', 'R', 'L', 'T' };
    uint32_t RLTSectionPosition;
    uint32_t RLTSectionCount;
    std::vector<RLTSection> Sections;
    std::vector<RLTEntry> Entries;

    void Parse(std::istream& stream) {
        stream.read((char*)this, 12);
        stream.seekg(4, std::ios_base::cur);
        Sections.resize(RLTSectionCount);
        stream.read((char*)&Sections[0], RLTSectionCount * sizeof(RLTSection));

        int entry_count = 0;
        for (const auto& section : Sections) {
            entry_count += section.Count;
        }

        Entries.resize(entry_count);
        stream.read((char*)&Entries[0], entry_count * sizeof(RLTEntry));
    }

    void Write(std::ostream& stream) {
        stream.write((char*)this, 12);
        stream.seekp(4, std::ios_base::cur);
        stream.write((char*)&Sections[0], Sections.size() * sizeof(RLTSection));
        stream.write((char*)&Entries[0], Entries.size() * sizeof(RLTEntry));
    }

    static RLTTable FromSectionSizes(const uint32_t ImageSize, const uint32_t str_section_size, const uint32_t dict_section_size) {

        return {
            { '_', 'R', 'L', 'T' },
            BRTD_SECTION_START + (uint32_t)sizeof(BRTDSection) + (uint32_t)ImageSize,
            2,
            {
                RLTSection { 0, 0, START_OF_STR_SECTION + str_section_size + dict_section_size + (uint32_t)sizeof(BRTISection) + 0x208, 0, 4 },
                RLTSection { 0, BRTD_SECTION_START, (uint32_t)sizeof(BRTDSection) + (uint32_t)ImageSize, 4, 1 }
            },
            {
                RLTEntry { sizeof(BNTXHeader) + 0x8, 2, 1, (sizeof(NXHeader) + MEM_POOL_SIZE - 0x10) / 8 },
                RLTEntry { sizeof(BNTXHeader) + 0x18, 2, 2, (uint8_t)((START_OF_STR_SECTION + str_section_size + dict_section_size + 0x80 - HEADER_SIZE) / 8) },
                RLTEntry { START_OF_STR_SECTION + str_section_size + 0x10, 2, 1, 1 },
                RLTEntry { START_OF_STR_SECTION + str_section_size + dict_section_size + 0x60, 1, 3, 0 },
                RLTEntry { sizeof(BNTXHeader) + 0x10, 2, 1, (uint8_t)((START_OF_STR_SECTION + str_section_size + dict_section_size + sizeof(BRTISection) + 0x200 - (sizeof(BNTXHeader) + 0x18)) / 8) }
            }
        };
    }

    uint64_t GetSize() {
        return 0x10 + sizeof(RLTSection) * Sections.size() + sizeof(RLTEntry) * Entries.size();
    }
};

// Can be memory-mapped, but likely to change in the future.
struct DICSection {
    char Magic[4] = { '_', 'D', 'I', 'C' };
    uint32_t unk;
    int32_t  unk2;
    uint32_t unk3;
    uint64_t unk4;
    uint32_t unk5;
    uint32_t unk6;
    uint64_t unk7;

    static DICSection Default() {
        return {
            { '_', 'D', 'I', 'C' },
            1,
            -1,
            1,
            436,
            1,
            65536,
            440
        };
    }

    void Parse(std::istream& stream) {
        stream.read((char*)this, sizeof(*this));
    }

    void Write(std::ostream& stream) {
        stream.write((char*)this, sizeof(*this));
    }

    uint64_t GetSize() {
        return sizeof(*this);
    }
};

// CANNOT be memory-mapped.
struct STRSection {
    char Magic[4] = { '_', 'S', 'T', 'R' };
    uint32_t SizeOfSTRAndDIC;
    uint32_t SizeOfSTRAndDIC2;
    uint32_t unk3;
    uint32_t StringCount;
    uint32_t Reserved; // TODO: What does this actually do though?
    std::vector<std::string> Strings;

    static STRSection FromStringAndDICSection(const std::string& input, DICSection& dic_section) {
        STRSection ret = {
            { '_', 'S', 'T', 'R' },
            0xCC,
            0xCC,
            0,
            1,
            0,
            { input }
        };
        ret.SizeOfSTRAndDIC = (uint32_t)ret.GetSize() + dic_section.GetSize();
        ret.SizeOfSTRAndDIC2 = (uint32_t)ret.GetSize() + dic_section.GetSize();
        return ret;
    }

    void Parse(std::istream& stream) {
        stream.read((char*)this, 24);

        uint16_t size = 0;

        stream.seekg(4, std::ios_base::cur);

        for (int x = 0; x < StringCount; x++) {
            // ew gross
            stream.read((char*)&size, 2);
            stream.seekg(2, std::ios_base::cur);
            char* tmp = new char[size](); // zero-initalize
            stream.read(tmp, size);
            std::string str(tmp, size);
            delete[] tmp;
            Strings.push_back(str);
        }
    }

    void Write(std::ostream& stream) {
        stream.write((char*)this, 24);

        for (const auto& str : Strings) {
            uint16_t size = str.size();
            stream.write((char*)&size, 2);
            stream.write(str.c_str(), str.size());
        }

        int pad_num = 0x8 - stream.tellp() % 0x8;
        stream.seekp(pad_num, std::ios_base::cur);

    }

    uint64_t GetSize() {
        uint64_t ret = 24;
        for (const auto& str : Strings)
            ret += str.size() + 3; // 2 bytes of length, 1 of extra space
        ret = (ret / 0x8 + 1) * 0x8;
        return ret;
    }
};

#pragma pack(pop)

class BNTX {
private:
    BNTXHeader header;
    NXHeader nx_header;
    char mem_pool[0x150] = { 0 };
    uint64_t BRTIOffset;

    // At this point the file needs to be manually parsed.
    STRSection str_section;

    DICSection dic_section;
    BRTISection brti_section;
    BRTDSection brtd_section;

    std::vector<uint8_t> IMAGE_DATA;

    RLTTable relocation_table;
public:
    BNTX(cv::Mat& mat, const std::string& name) {

        /*
        DirectX::Image dximg{
            mat.cols,
            mat.rows,
            DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
            mat.cols * 4,
            mat.cols * mat.rows * 4,
            mat.data
        };

        DirectX::ScratchImage SImg;

        std::cout << "[SteveModMaker::BNTX::BNTX] Compressing image to BC7_UNORM..." << std::endl;

        // Should be similar to using Fast compression mode in Switch Toolbox.
        DirectX::Compress(dximg, DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM, DirectX::TEX_COMPRESS_BC7_QUICK, DirectX::TEX_THRESHOLD_DEFAULT, SImg);

        std::cout << "[SteveModMaker::BNTX::BNTX] Swizzling image via tegra_swizzle..." << std::endl;

        uint64_t ImageSize = swizzled_mip_size(
            div_round_up(mat.cols, 4),
            div_round_up(mat.rows, 4),
            1,
            block_height_mip0(div_round_up(mat.rows, 4)),
            16
        );

        IMAGE_DATA.resize(ImageSize);

        swizzle_block_linear(
            div_round_up(mat.cols, 4),
            div_round_up(mat.rows, 4),
            1,
            SImg.GetPixels(),
            SImg.GetPixelsSize(),
            IMAGE_DATA.data(),
            ImageSize,
            block_height_mip0(div_round_up(mat.rows, 4)),
            16
        );
        */

        uint64_t ImageSize = swizzled_mip_size(
            mat.cols,
            mat.rows,
            1,
            block_height_mip0(mat.rows),
            4
        );

        IMAGE_DATA.resize(ImageSize);

        swizzle_block_linear(
            mat.cols,
            mat.rows,
            1,
            mat.data,
            mat.cols * mat.rows * 4,
            IMAGE_DATA.data(),
            ImageSize,
            block_height_mip0(mat.rows),
            4
        );

        dic_section = DICSection::Default();

        str_section = STRSection::FromStringAndDICSection(name, dic_section);
        
        BRTIOffset = START_OF_STR_SECTION + str_section.GetSize() + dic_section.GetSize();

        relocation_table = RLTTable::FromSectionSizes(ImageSize, str_section.GetSize(), dic_section.GetSize());

        brti_section = {
            { 'B', 'R', 'T', 'I' },
            3592,
            3592,
            1,
            2,
            0,
            0,
            1,
            1,
            0x0C06, // BC7_UNORM is 0x2001, R8G8B8A8_UNORM_SRGB is 0x0B06, 0x0C06 is B8G8R8A8_UNORM_SRGB
            32,
            (uint32_t)mat.cols,
            (uint32_t)mat.rows,
            1,
            1,
            4, // Use 1 for BC7_UNORM and 4 for R8G8B8A8_UNORM_SRGB and B8G8R8A8_UNORM_SRGB
            {65543, 0, 0, 0, 0, 0},
            (uint32_t)ImageSize,
            512,
            84148994,
            1,
            FILENAME_STR_OFFSET,
            32,
            START_OF_STR_SECTION + str_section.GetSize() + dic_section.GetSize() + sizeof(BRTISection) + 0x200,
            0,
            START_OF_STR_SECTION + str_section.GetSize() + dic_section.GetSize() + sizeof(BRTISection),
            START_OF_STR_SECTION + str_section.GetSize() + dic_section.GetSize() + sizeof(BRTISection) + 0x100,
            0,
            0
        };

        brtd_section = {
            { 'B', 'R', 'T', 'D'},
            0,
            ImageSize + 0x10
        };

        header = {
            { 'B', 'N', 'T', 'X'},
            0,
            {0, 4},
            0xFEFF,
            0x400C,
            FILENAME_STR_OFFSET + 0x2,
            0,
            (uint16_t)START_OF_STR_SECTION,
            0xFF0 + (uint32_t)sizeof(BRTDSection) + (uint32_t)ImageSize,
            0xFF0 + (uint32_t)sizeof(BRTDSection) + (uint32_t)ImageSize + (uint32_t)relocation_table.GetSize(), // size of file
        };

        nx_header = {
            { 'N', 'X', ' ', ' ' },
            1,
            START_OF_STR_SECTION - 0x8,
            BRTD_SECTION_START,
            START_OF_STR_SECTION + str_section.GetSize(),
            str_section.GetSize() + dic_section.GetSize() + 0x10,
        };

    }

    void Write(const std::string& path) {
        std::cout << "[SteveModMaker::BNTX::Write] Writing image to BNTX..." << std::endl;
        std::ofstream file(path, std::ios::out | std::ios::binary);
        file.write((char*)&header, sizeof(BNTXHeader));
        file.write((char*)&nx_header, sizeof(NXHeader));
        file.write(mem_pool, MEM_POOL_SIZE);
        file.write((char*)&BRTIOffset, sizeof(BRTIOffset));
        str_section.Write(file);
        dic_section.Write(file);
        file.write((char*)&brti_section, sizeof(BRTISection));
        file.seekp(START_OF_STR_SECTION + str_section.GetSize() + dic_section.GetSize() + sizeof(BRTISection) + 0x200);
        uint32_t unk_redirect_offset = 4096;
        file.write((char*)&unk_redirect_offset, sizeof(uint32_t));
        file.seekp(0xFF0);
        file.write((char*)&brtd_section, sizeof(BRTDSection));
        file.write((char*)&IMAGE_DATA[0], IMAGE_DATA.size());
        relocation_table.Write(file);
        file.close();
    }
};