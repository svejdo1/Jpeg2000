#ifndef _COMMON_H_
#define _COMMON_H_

#include <boost\cstdint.hpp>
#include <ostream>

namespace BJPEG
{

class JpegAccess
{
	JpegAccess() {}
public:
	static inline void WriteUint8(std::ostream& out, uint8_t value)
	{
		char bytes[1] = { (char)value };
		out.write(bytes, 1);
	}

	static inline void WriteUint16(std::ostream& out, uint16_t value)
	{
		char bytes[2] = { (char)(value >> 8), (char)value };
		out.write(bytes, 2);
	}

	static inline void WriteUint32(std::ostream& out, uint32_t value)
	{
		char bytes[4] = { (char)(value >> 24), (char)(value >> 16), (char)(value >> 8), (char)value };
		out.write(bytes, 4);
	}

	static inline void WriteUint64(std::ostream& out, uint64_t value)
	{
		char bytes[8] = { 
			(char)(value >> 56), (char)(value >> 48), (char)(value >> 40), (char)(value >> 32),
			(char)(value >> 24), (char)(value >> 16), (char)(value >> 8),  (char)value 
		};
		out.write(bytes, 8);
	}

	static inline uint8_t ReadUint8(const uint8_t* buffer, int offset)
	{
		return buffer[offset];
	}

	static inline uint16_t ReadUint16(const uint8_t* buffer, int offset)
	{
		return (buffer[offset] << 8) + buffer[offset + 1];
	}
	static inline uint32_t ReadUint32(const uint8_t* buffer, int offset)
	{
		return (buffer[offset] << 24) + (buffer[offset + 1] << 16) + (buffer[offset + 2] << 8) + (buffer[offset + 3]);
	}

	static inline uint64_t ReadUint64(const uint8_t* buffer, int offset)
	{
		return (((uint64_t)buffer[offset]) << 56) + (((uint64_t)buffer[offset + 1]) << 48) + (((uint64_t)buffer[offset + 2]) << 40) + (((uint64_t)buffer[offset + 3]) << 32) +
			(buffer[offset + 4] << 24) + (buffer[offset + 5] << 16) + (buffer[offset + 6] << 8) + (buffer[offset + 7]);
	}

	static inline bool VerifyReadUint16(const uint8_t* buffer, int offset, uint16_t value)
	{
		return ReadUint16(buffer, offset) == value;
	}

	static inline bool VerifyReadUint32(const uint8_t* buffer, int offset, uint32_t value)
	{
		return ReadUint32(buffer, offset) == value;
	}

	static inline bool VerifyReadUint64(const uint8_t* buffer, int offset, uint64_t value)
	{
		return ReadUint64(buffer, offset) == value;
	}
};

enum ErrorCode
{
	SUCCESS,
	FILE_CANNOT_SEEK,
	
	J2K_COM_DOESNT_MATCH,
	J2K_LCOM_DOESNT_MATCH,
	J2K_RCOM_DOESNT_MATCH,
	J2K_SOT_DOESNT_MATCH,
	J2K_LSOT_DOESNT_MATCH,
	J2K_SOC_DOESNT_MATCH,
	J2K_SIZ_DOESNT_MATCH,
	J2K_COD_DOESNT_MATCH,
	J2K_QCD_DOESNT_MATCH,
	J2K_QCC_DOESNT_MATCH,
	J2K_EOC_DOESNT_MATCH,
	J2K_COC_DOESNT_MATCH,
	J2K_SOD_DOESNT_MATCH,
	J2K_PLT_DOESNT_MATCH,

	J2P_FILE_MAGIC_STRING_DOESNT_MATCH,
	J2P_FILE_TYPE_DESCRIPTOR_DOESNT_MATCH,
	J2P_UNKNOWN_FILE_TYPE,
	J2P_HEADER_DESCRIPTOR_DOESNT_MATCH,
	J2P_UNKOWN_BOX,
	J2P_IMAGE_HEADER_INVALID_SIZE,
	J2P_IMAGE_HEADER_DESCRIPTOR_DOESNT_MATCH,
	J2P_COLOUR_SPECIFICATION_DOESNT_MATCH,
	J2P_RESOLUTION_DESCRIPTOR_DOESNT_MATCH,
	J2P_CAPTURE_RESOLUTION_DESCRIPTOR_DOESNT_MATCH,
	J2P_DEFAULT_DISPLAY_RESOLUTION_DESCRIPTOR_DOESNT_MATCH,
	J2P_CONTIGUOUS_CODESTREAM_DOESNT_MATCH
};

class ImageFilePart
{
public:
	virtual ErrorCode load(const uint8_t* buffer, int offset) = 0;
	virtual void save(std::ostream& stream) const = 0;
};

class ImageFile : public ImageFilePart
{
public:
	virtual ErrorCode loadFile(const std::string& fileName);
	virtual void saveFile(const std::string& fileName) const;
};

}


#endif /*_COMMON_H_*/