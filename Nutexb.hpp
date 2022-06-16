#pragma once
#include <fstream>

enum class NUTEXBFormat : uint8_t {
    R8G8B8A8_UNORM = 0,
    R8G8B8A8_SRGB = 5,
    R32G32B32A32_FLOAT = 52,
    B8G8R8A8_UNORM = 80,
    B8G8R8A8_SRGB = 85,
    BC1_UNORM = 128,
    BC1_SRGB = 133,
    BC2_UNORM = 144,
    BC2_SRGB = 149,
    BC3_UNORM = 160,
    BC3_SRGB = 165,
    BC4_UNORM = 176,
    BC4_SNORM = 181,
    BC5_UNORM = 192,
    BC5_SNORM = 197,
    BC6_UFLOAT = 215,
    BC7_UNORM = 224,
    BC7_SRGB = 229,
};

#pragma pack(push, 1)

struct NUTEXBFooter {
    uint32_t mip_sizes[16];
    const char XNT[4] = { ' ', 'X', 'N', 'T' };
    char internal_name[0x40];
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    NUTEXBFormat format;
    uint8_t unk;
    uint16_t PADDING;
    uint32_t unk2;
    uint32_t mip_count;
    uint32_t alignment;
    uint32_t array_count;
    uint32_t size;
    const char XET[4] = { ' ', 'X', 'E', 'T' };
    uint16_t major_version;
    uint16_t minor_version;
    
    NUTEXBFooter() {
        memset(&this->mip_sizes, 0, 64);
        memset(&this->internal_name, 0, 64);
        memset(&this->major_version, 0, 4);
        // zero-initialize everything
    }
};

#pragma pack(pop)

class NUTEXB {
private:
    std::vector<uint8_t> IMAGE_DATA;
    NUTEXBFooter footer;
public:
    bool Open(std::string filepath) {
        std::ifstream FSTREAM(filepath, std::ios::in | std::ios::binary);
        if (!FSTREAM) {
            return false;
        }
        FSTREAM.seekg(-0xB0, std::ios_base::end);
        FSTREAM.read((char*)&footer, sizeof(footer));
        FSTREAM.seekg(0);
        IMAGE_DATA.resize(footer.size);
        FSTREAM.read((char*)&IMAGE_DATA[0], footer.size);
        FSTREAM.close();
        return true;
    };

    NUTEXB(const std::string& internal_name, void* data, size_t size) {
        IMAGE_DATA.resize(size);
        memcpy(&IMAGE_DATA[0], data, size);
		footer.size = size;
        strcpy_s(footer.internal_name, 0x40, internal_name.c_str());
    }
	
	NUTEXB(const std::string& internal_name, cv::Mat& mat) {

        IMAGE_DATA.resize(mat.cols * mat.rows * 4);
        memcpy(&IMAGE_DATA[0], mat.data, mat.cols * mat.rows * 4);

        footer.mip_sizes[0] = mat.cols * mat.rows * 4;

        strcpy_s(footer.internal_name, 0x40, internal_name.c_str());

		footer.width = mat.cols;
		footer.height = mat.rows;
		footer.depth = 1;
		footer.format = NUTEXBFormat::B8G8R8A8_SRGB;
		footer.unk = 4;
		footer.PADDING = 0;
		footer.unk2 = 4;
		footer.mip_count = 1;
		footer.alignment = 0;
		footer.array_count = 1;
		footer.size = mat.cols * mat.rows * 4;
		footer.major_version = 1;
		footer.minor_version = 2;
    }
	
	NUTEXBFooter& GetFooter() { return footer; }

    bool Save(std::string filepath, unsigned int pad = 0) {
        std::ofstream NUT_OUT(filepath, std::ios::out | std::ios::binary);
        if (!NUT_OUT) {
            return false;
        }
        NUT_OUT.write((char*)&IMAGE_DATA[0], footer.size);

        char null = 0;
        for (unsigned int x = 0; x < pad; x++)
            NUT_OUT.write(&null, 1);

        NUT_OUT.write(reinterpret_cast<char *>(&footer), sizeof(footer));
        NUT_OUT.close();
        return true;
    }
	
    bool Save(std::ostream& NUT_OUT, unsigned int pad = 0) {
        if (!NUT_OUT) {
            return false;
        }
        NUT_OUT.write((char*)&IMAGE_DATA[0], footer.size);

        char null = 0;
        for (unsigned int x = 0; x < pad; x++)
            NUT_OUT.write(&null, 1);

        NUT_OUT.write(reinterpret_cast<char*>(&footer), sizeof(footer));
        return true;
    }

};