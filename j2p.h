#ifndef _J2P_H_
#define _J2P_H_

#include <boost\cstdint.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\foreach.hpp>
#include <vector>
#include <string>
#include "common.h"
#include "j2k.h"

namespace BJPEG
{

class J2PPart;
typedef std::shared_ptr<J2PPart> J2PPartPtr;

class J2PPart : public ImageFilePart
{
public:
	uint32_t readLength;

	virtual uint32_t getMarker() const = 0;
	virtual uint32_t size() const
	{
		return readLength;
	}
	static J2PPartPtr createByMarkerId(uint32_t markerId);
};

class J2PFileType : public J2PPart
{
public:
	static const uint32_t MARKER_ID = 0x66747970;
	std::string BR;
	uint32_t MinV;
	std::string CL;

	uint32_t getMarker() const
	{
		return MARKER_ID;
	}

	ErrorCode load(const uint8_t* buffer, int offset);
	void save(std::ostream& stream) const;
};

class J2PHeader : public J2PPart
{
public:
	static const uint32_t MARKER_ID = 0x6A703268;
	std::vector<J2PPartPtr> boxes;

	uint32_t getMarker() const
	{
		return MARKER_ID;
	}
	ErrorCode load(const uint8_t* buffer, int offset);
	void save(std::ostream& stream) const;
};

class J2PImageHeader : public J2PPart
{
public:
	static const uint32_t MARKER_ID = 0x69686472;
	uint32_t Height;
	uint32_t Width;
	uint16_t NumberOfComponents;
	uint8_t BitsPerComponent;
	uint8_t CompressionType;
	uint8_t ColourspaceUnkown;
	uint8_t IntellectualProperty;
	
	uint32_t getMarker() const
	{
		return MARKER_ID;
	}
	ErrorCode load(const uint8_t* buffer, int offset);
	void save(std::ostream& stream) const;
};

class J2PBitsPerComponent : public J2PPart
{
public:
	static const uint32_t MARKER_ID = 0x62706363;

	uint32_t getMarker() const
	{
		return MARKER_ID;
	}
	ErrorCode load(const uint8_t* buffer, int offset);
	void save(std::ostream& stream) const;
};

class J2PColourSpecification : public J2PPart
{
public:
	static const uint32_t MARKER_ID = 0x636F6C72;
	uint8_t SpecificationMethod;
	uint8_t Precedence;
	uint8_t ColourspaceApproximation;
	uint32_t EnumeratedColourspace;

	uint32_t getMarker() const
	{
		return MARKER_ID;
	}
	ErrorCode load(const uint8_t* buffer, int offset);
	void save(std::ostream& stream) const;
};

class J2PCaptureResolution : public J2PPart
{
public:
	static const uint32_t MARKER_ID = 0x72657363;
	uint16_t VRcN;
	uint16_t VRcD;
	uint16_t HRcN;
	uint16_t HRcD;
	int8_t VRcE;
	int8_t HRcE;

	uint32_t getMarker() const
	{
		return MARKER_ID;
	}

	ErrorCode load(const uint8_t* buffer, int offset);
	void save(std::ostream& stream) const;
};

class J2PDefaultDisplayResolution : public J2PPart
{
public:
	static const uint32_t MARKER_ID = 0x72657364;
	uint16_t VRcN;
	uint16_t VRcD;
	uint16_t HRcN;
	uint16_t HRcD;
	int8_t VRcE;
	int8_t HRcE;

	uint32_t getMarker() const
	{
		return MARKER_ID;
	}

	ErrorCode load(const uint8_t* buffer, int offset);
	void save(std::ostream& stream) const;
};

class J2PResolution : public J2PPart
{
public:
	static const uint32_t MARKER_ID = 0x72657320;
	J2PCaptureResolution captureResolution;
	J2PDefaultDisplayResolution defaultDisplayResolution;

	uint32_t getMarker() const
	{
		return MARKER_ID;
	}
	ErrorCode load(const uint8_t* buffer, int offset);
	void save(std::ostream& stream) const;
};

class J2PContiguousCodestream : public J2PPart
{
public:
	static const uint32_t MARKER_ID = 0x6A703263;
	J2KFile file;

	uint32_t getMarker() const
	{
		return MARKER_ID;
	}
	ErrorCode load(const uint8_t* buffer, int offset);
	void save(std::ostream& stream) const;
	uint32_t size() const;
};

class J2PFile : public J2PPart, public ImageFile
{
private:
	static const uint32_t MARKER0 = 0x0000000C;
	static const uint32_t MARKER1 = 0x6A502020;
	static const uint32_t MARKER2 = 0x0D0A870A;
public:
	static const uint32_t MARKER_ID = MARKER1;

	J2PFileType fileType;
	J2PHeader header;
	J2PContiguousCodestream codestream;

	uint32_t getMarker() const
	{
		return MARKER_ID;
	}
	ErrorCode load(const uint8_t* buffer, int offset);
	void save(std::ostream& stream) const;
};

}

#endif
