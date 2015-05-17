#ifndef _J2K_H_
#define _J2K_H_

#include <boost\optional\optional.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\foreach.hpp>
#include <exception>
#include <string>
#include <vector>
#include <memory>
#include <numeric>
#include "common.h"

namespace BJPEG
{
	class J2KMarkers
	{
	public:
		static const uint16_t SOC = 0xFF4F;
		static const uint16_t SIZ = 0xFF51;
		static const uint16_t COD = 0xFF52;
		static const uint16_t COC = 0xFF53;
		static const uint16_t PLT = 0xFF58;
		static const uint16_t QCD = 0xFF5C;
		static const uint16_t QCC = 0xFF5D;
		static const uint16_t POC = 0xFF5F;
		static const uint16_t PPT = 0xFF61;
		static const uint16_t COM = 0xFF64;
		static const uint16_t SOT = 0xFF90;
		static const uint16_t SOD = 0xFF93;
	};

	class J2KPart;
	typedef std::shared_ptr<J2KPart> J2KPartPtr;

	class J2KPart : public ImageFilePart
	{
	public:
		virtual uint16_t getMarker() const = 0;
		virtual uint32_t size() const = 0;
		static J2KPartPtr createByMarkerId(uint16_t markerId);
	};
	

	class StartOfFrameSegment
	{
	public:
		static const uint16_t Com = 0xFF64;
		static const uint16_t Lcom = 0x000E;
		static const uint16_t Rcom = 0x4D4D;
		uint16_t Ccom1;
		uint32_t Ccom2;
		uint32_t Ccom3;

		StartOfFrameSegment()
		{
		}

		ErrorCode load(const uint8_t* buffer, int offset);
	};

	class ComponentHeader
	{
		public:
			uint8_t Ssiz;
			uint8_t XRsiz;
			uint8_t YRsiz;

			ComponentHeader() {}
			ComponentHeader(const ComponentHeader& header);
			void load(const uint8_t* buffer, int offset);
			void save(std::ostream& stream) const;
	};

	class Header : J2KPart
	{
		public:
			static const uint16_t MARKER_ID = J2KMarkers::SIZ;
			uint16_t Lsiz;
			uint16_t Rsiz;
			uint32_t Xsiz;
			uint32_t Ysiz;
			uint32_t XOsiz;
			uint32_t YOsiz;
			uint32_t XTsiz;
			uint32_t YTsiz;
			uint32_t XTOsiz;
			uint32_t YTOsiz;
			uint16_t Csiz;
			std::vector<ComponentHeader> Components;

			Header() {}

			uint16_t getMarker() const
			{
				return MARKER_ID;
			}

			uint32_t size() const
			{
				return 40 + this->Csiz * 3;
			}
			ErrorCode load(const uint8_t* buffer, int offset);
			void save(std::ostream& stream) const;
	};

	class CodingStyleDefault : public J2KPart
	{
	public:
		static const uint16_t MARKER_ID = J2KMarkers::COD;
		uint16_t Lcod;
		uint8_t Scod;
		uint8_t ProgressionOrder;
		uint16_t NumberOfLayers;
		uint8_t MultipleComponentTransformation;
		uint8_t NumberOfDecompositionLevels;
		uint8_t CodeBlockWidth;
		uint8_t CodeBlockHeight;
		uint8_t CodeBlockStyle;
		uint8_t Transformation;
		std::vector<uint8_t> PrecintSizes;

		CodingStyleDefault() {}

		uint16_t getMarker() const
		{
			return MARKER_ID;
		}

		inline bool isEntropyCoderWithDefinedPrecints() const
		{
			return (Scod & 1) == 1;
		}

		inline bool canUseSOPMarker() const
		{
			return (Scod & 2) == 2;
		}

		inline bool canUseEPHMarker() const
		{
			return (Scod & 4) == 4;
		}


		inline static bool isValid(const uint8_t* buffer, int offset)
		{
			return JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID);
		}

		uint32_t size() const
		{
			return Lcod + 2;
		}
		ErrorCode load(const uint8_t* buffer, int offset);
		void save(std::ostream& stream) const;
	};

	class QuantizationDefaultParameter : public J2KPart
	{
	public:
		static const uint16_t MARKER_ID = J2KMarkers::QCD;
		uint16_t Lqcd;
		uint8_t Sqcd;
		std::vector<uint8_t> Raw;

		QuantizationDefaultParameter() {}

		uint16_t getMarker() const
		{
			return MARKER_ID;
		}

		inline static bool isValid(const uint8_t* buffer, int offset)
		{
			return JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID);
		}

		inline uint32_t size() const
		{
			return Lqcd + 2;
		}
		ErrorCode load(const uint8_t* buffer, int offset);
		void save(std::ostream& stream) const;
	};

	class QuantizationComponent : public J2KPart
	{
	public:
		static const uint16_t MARKER_ID = J2KMarkers::QCC;
		uint16_t Lqcc;
		std::vector<uint8_t> Raw;

		QuantizationComponent() {}

		uint16_t getMarker() const
		{
			return MARKER_ID;
		}

		inline static bool isValid(const uint8_t* buffer, int offset)
		{
			return JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID);
		}

		uint32_t size() const
		{
			return Lqcc + 2;
		}
		ErrorCode load(const uint8_t* buffer, int offset);
		void save(std::ostream& stream) const;
	};

	class Comment : J2KPart
	{
	public:
		static const uint16_t MARKER_ID = J2KMarkers::COM;
		uint16_t Lcom;
		uint16_t Rcom;
		std::vector<uint8_t> Raw;

		Comment() {}

		uint16_t getMarker() const
		{
			return MARKER_ID;
		}

		inline bool isBinary() const
		{
			return Rcom == 0;
		}

		inline bool isLatin() const
		{
			return Rcom == 1;
		}

		
		inline uint32_t size() const
		{
			return Lcom + 2;
		}

		inline static bool isValid(const uint8_t* buffer, int offset)
		{
			return JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID);
		}
		void load(const std::string& text);
		ErrorCode load(const uint8_t* buffer, int offset);
		void save(std::ostream& stream) const;
	};

	class CodingStyleComponent : public J2KPart
	{
	public:
		static const uint16_t MARKER_ID = J2KMarkers::COC;
		uint16_t Lcoc;
		std::vector<uint8_t> Raw;

		CodingStyleComponent() {}

		uint16_t getMarker() const
		{
			return MARKER_ID;
		}

		uint32_t size() const
		{
			return Lcoc + 2;
		}

		inline static bool isValid(const uint8_t* buffer, int offset)
		{
			return JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID);
		}
		ErrorCode load(const uint8_t* buffer, int offset);
		void save(std::ostream& stream) const;
	};

	class PacketLength
	{
	public:
		uint32_t value;
		PacketLength(uint32_t value)
		{
			this->value = value;
		}
		void save(std::ostream& stream) const;
		uint32_t size() const;
	};

	class PacketLengthTilePartHeader : public J2KPart
	{
	public:
		static const uint16_t MARKER_ID = J2KMarkers::PLT;
		uint16_t Lplt;
		uint8_t Zplt;
		std::vector<PacketLength> packetLengths;

		uint16_t getMarker() const
		{
			return MARKER_ID;
		}

		// Lplt + 2
		uint32_t size() const
		{
			//return Lplt + 2;
			uint32_t result = 5;
			BOOST_FOREACH(const PacketLength& ptr, packetLengths)
			{
				result += ptr.size();
			}
			return result;
		}

		inline static bool isValid(const uint8_t* buffer, int offset)
		{
			return JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID);
		}
		ErrorCode load(const uint8_t* buffer, int offset);
		void save(std::ostream& stream) const;
	};

	class TilePart : J2KPart
	{
	public:
		static const uint16_t MARKER_ID = J2KMarkers::SOT;
		static const uint16_t Lsot = 10;
		uint16_t Isot;
		//uint32_t Psot;
		uint8_t TPsot;
		uint8_t TNsot;
		std::vector<J2KPartPtr> markers;
		std::vector<uint8_t> Raw;

		uint16_t getMarker() const
		{
			return MARKER_ID;
		}

		// aka Psot
		uint32_t size() const
		{
			uint32_t result = 12 + Raw.size();
			BOOST_FOREACH(const J2KPartPtr ptr, markers)
			{
				result += ptr->size();
			}
			return result;
		}

		inline static bool isValid(const uint8_t* buffer, int offset)
		{
			return JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID);
		}
		ErrorCode load(const uint8_t* buffer, int offset);
		void save(std::ostream& stream) const;
	};

	class J2KFile : public J2KPart, public ImageFile
	{
	public:
		static const uint16_t MARKER_ID = J2KMarkers::SOC;
		static const uint16_t EOC = 0xFFD9;

		Header header;
		CodingStyleDefault codingStyleDefault;
		QuantizationDefaultParameter quantizationDefaultParameter;
		std::vector<QuantizationComponent> componentQccs;
		std::vector<Comment> comments;
		std::vector<TilePart> tiles;
		
		J2KFile() {}

		using ImageFile::load;

		uint16_t getMarker() const
		{
			return MARKER_ID;
		}

		uint32_t size() const;
		ErrorCode load(const uint8_t* buffer, int offset);
		void save(std::ostream& stream) const;
	};
}



#endif /*_J2K_H_*/