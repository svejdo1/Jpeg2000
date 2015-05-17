#include "j2p.h"
#include <fstream>

using namespace BJPEG;
using namespace std;

J2PPartPtr J2PPart::createByMarkerId(uint32_t markerId)
{
	switch (markerId)
	{
		case J2PFileType::MARKER_ID:
			return shared_ptr<J2PFileType>(new J2PFileType());
		case J2PHeader::MARKER_ID:
			return shared_ptr<J2PHeader>(new J2PHeader());
		case J2PImageHeader::MARKER_ID:
			return shared_ptr<J2PImageHeader>(new J2PImageHeader());
		case J2PBitsPerComponent::MARKER_ID:
			return shared_ptr<J2PBitsPerComponent>(new J2PBitsPerComponent());
		case J2PColourSpecification::MARKER_ID:
			return shared_ptr<J2PColourSpecification>(new J2PColourSpecification());
		case J2PFile::MARKER_ID:
			return shared_ptr<J2PFile>(new J2PFile());
		case J2PResolution::MARKER_ID:
			return shared_ptr<J2PResolution>(new J2PResolution());
	}
	return nullptr;
}

ErrorCode J2PFile::load(const uint8_t* buffer, int offset)
{
	if (!JpegAccess::VerifyReadUint32(buffer, offset, MARKER0) ||
		!JpegAccess::VerifyReadUint32(buffer, offset + 4, MARKER1) ||
		!JpegAccess::VerifyReadUint32(buffer, offset + 8, MARKER2))
	{
		return J2P_FILE_MAGIC_STRING_DOESNT_MATCH;
	}
	ErrorCode result;
	result = fileType.load(buffer, offset + 12);
	if (result != SUCCESS)
	{
		return result;
	}
	offset += 12 + fileType.size();

	result = header.load(buffer, offset);
	if (result != SUCCESS)
	{
		return result;
	}
	offset += header.size();

	codestream.load(buffer, offset);
	offset += codestream.size();

	return SUCCESS;
}

void J2PFile::save(std::ostream& stream) const
{
}

ErrorCode J2PFileType::load(const uint8_t* buffer, int offset)
{
	readLength = JpegAccess::ReadUint32(buffer, offset);

	if (!JpegAccess::VerifyReadUint32(buffer, offset + 4, MARKER_ID))
	{
		return J2P_FILE_TYPE_DESCRIPTOR_DOESNT_MATCH;
	}
	BR = string(buffer + offset + 8, buffer + offset + 12);
	if (BR != "jp2 ")
	{
		return J2P_UNKNOWN_FILE_TYPE;
	}

	MinV = JpegAccess::ReadUint32(buffer, offset + 12);
	CL = string(buffer + offset + 16, buffer + offset + 20);

	return SUCCESS;
}

void J2PFileType::save(std::ostream& stream) const
{
}

ErrorCode J2PHeader::load(const uint8_t* buffer, int offset)
{
	readLength = JpegAccess::ReadUint32(buffer, offset);
	if (!JpegAccess::VerifyReadUint32(buffer, offset + 4, MARKER_ID))
	{
		return J2P_HEADER_DESCRIPTOR_DOESNT_MATCH;
	}
	int endOffset = offset + readLength;
	offset += 8;
	boxes.clear();
	while(true)
	{
		uint32_t markerId = JpegAccess::ReadUint32(buffer, offset + 4);
		J2PPartPtr part = J2PPart::createByMarkerId(markerId);

		if (part == nullptr)
		{
			return J2P_UNKOWN_BOX;
		}
		else
		{
			part.get()->load(buffer, offset);
			boxes.push_back(part);
			offset += part.get()->size();
			if (offset == endOffset)
			{
				return SUCCESS;
			}
		}
	}
}

void J2PHeader::save(std::ostream& stream) const
{
}

ErrorCode J2PImageHeader::load(const uint8_t* buffer, int offset)
{
	readLength = 22;
	if (!JpegAccess::VerifyReadUint32(buffer, offset, readLength))
	{
		return J2P_IMAGE_HEADER_INVALID_SIZE;
	}
	if (!JpegAccess::VerifyReadUint32(buffer, offset + 4, MARKER_ID))
	{
		return J2P_IMAGE_HEADER_DESCRIPTOR_DOESNT_MATCH;
	}
	Height = JpegAccess::ReadUint32(buffer, offset + 8);
	Width = JpegAccess::ReadUint32(buffer, offset + 12);
	NumberOfComponents = JpegAccess::ReadUint16(buffer, offset + 16);
	BitsPerComponent = JpegAccess::ReadUint8(buffer, offset + 18);
	CompressionType = JpegAccess::ReadUint8(buffer, offset + 19);
	ColourspaceUnkown = JpegAccess::ReadUint8(buffer, offset + 20);
	IntellectualProperty = JpegAccess::ReadUint8(buffer, offset + 21);

	return SUCCESS;
}

void J2PImageHeader::save(std::ostream& stream) const
{
}

ErrorCode J2PBitsPerComponent::load(const uint8_t* buffer, int offset)
{
	return SUCCESS;
}

void J2PBitsPerComponent::save(std::ostream& stream) const
{
}

ErrorCode J2PColourSpecification::load(const uint8_t* buffer, int offset)
{
	readLength = JpegAccess::ReadUint32(buffer, offset);
	if (!JpegAccess::VerifyReadUint32(buffer, offset + 4, MARKER_ID))
	{
		return J2P_COLOUR_SPECIFICATION_DOESNT_MATCH;
	}
	SpecificationMethod = JpegAccess::ReadUint8(buffer, offset + 8);
	Precedence = JpegAccess::ReadUint8(buffer, offset + 9);
	ColourspaceApproximation = JpegAccess::ReadUint8(buffer, offset + 10);
	
	offset += 11;
	if (SpecificationMethod == 1)
	{
		EnumeratedColourspace = JpegAccess::ReadUint32(buffer, offset);
		offset += 4;
	}

	if (SpecificationMethod == 2)
	{
		// TODO - load PROFILE
	}
	
	return SUCCESS;
}

void J2PColourSpecification::save(std::ostream& stream) const
{
}

ErrorCode J2PResolution::load(const uint8_t* buffer, int offset)
{
	readLength = JpegAccess::ReadUint32(buffer, offset);
	if (!JpegAccess::VerifyReadUint32(buffer, offset + 4, MARKER_ID))
	{
		return J2P_RESOLUTION_DESCRIPTOR_DOESNT_MATCH;
	}
	offset += 8;
	ErrorCode result = captureResolution.load(buffer, offset);
	if (result != SUCCESS)
	{
		return result;
	}
	offset += captureResolution.size();
	result = defaultDisplayResolution.load(buffer, offset);
	return result;
}

void J2PResolution::save(std::ostream& stream) const
{
}

ErrorCode J2PCaptureResolution::load(const uint8_t* buffer, int offset)
{
	readLength = JpegAccess::ReadUint32(buffer, offset);
	if (!JpegAccess::VerifyReadUint32(buffer, offset + 4, MARKER_ID))
	{
		return J2P_CAPTURE_RESOLUTION_DESCRIPTOR_DOESNT_MATCH;
	}

	VRcN = JpegAccess::ReadUint16(buffer, offset + 8);
	VRcD = JpegAccess::ReadUint16(buffer, offset + 10);
	HRcN = JpegAccess::ReadUint16(buffer, offset + 12);
	HRcD = JpegAccess::ReadUint16(buffer, offset + 14);
	VRcE = JpegAccess::ReadUint8(buffer, offset + 16);
	HRcE = JpegAccess::ReadUint8(buffer, offset + 17);
	return SUCCESS;
}

void J2PCaptureResolution::save(std::ostream& stream) const
{
}

ErrorCode J2PDefaultDisplayResolution::load(const uint8_t* buffer, int offset)
{
	readLength = JpegAccess::ReadUint32(buffer, offset);
	if (readLength == 0)
	{
		return SUCCESS;
	}

	if (!JpegAccess::VerifyReadUint32(buffer, offset + 4, MARKER_ID))
	{
		return J2P_DEFAULT_DISPLAY_RESOLUTION_DESCRIPTOR_DOESNT_MATCH;
	}

	VRcN = JpegAccess::ReadUint16(buffer, offset + 8);
	VRcD = JpegAccess::ReadUint16(buffer, offset + 10);
	HRcN = JpegAccess::ReadUint16(buffer, offset + 12);
	HRcD = JpegAccess::ReadUint16(buffer, offset + 14);
	VRcE = JpegAccess::ReadUint8(buffer, offset + 16);
	HRcE = JpegAccess::ReadUint8(buffer, offset + 17);
	return SUCCESS;
}

void J2PDefaultDisplayResolution::save(std::ostream& stream) const
{
}

ErrorCode J2PContiguousCodestream::load(const uint8_t* buffer, int offset)
{
	readLength = JpegAccess::ReadUint32(buffer, offset);

	if (!JpegAccess::VerifyReadUint32(buffer, offset + 4, MARKER_ID))
	{
		return J2P_CONTIGUOUS_CODESTREAM_DOESNT_MATCH;
	}

	ErrorCode result = file.load(buffer, offset + 8);

	return result;
}

void J2PContiguousCodestream::save(std::ostream& stream) const
{
}

uint32_t J2PContiguousCodestream::size() const
{
	return 8 + file.size();
}
