#include "j2k.h"
#include <fstream>

using namespace std;

namespace BJPEG
{
	J2KPartPtr J2KPart::createByMarkerId(uint16_t markerId)
	{
		switch (markerId)
		{
			case CodingStyleComponent::MARKER_ID:
				return shared_ptr<CodingStyleComponent>(new CodingStyleComponent());
			case QuantizationComponent::MARKER_ID:
				return shared_ptr<QuantizationComponent>(new QuantizationComponent());
			case PacketLengthTilePartHeader::MARKER_ID:
				return shared_ptr<PacketLengthTilePartHeader>(new PacketLengthTilePartHeader());
			case CodingStyleDefault::MARKER_ID:
				return shared_ptr<CodingStyleDefault>(new CodingStyleDefault());
		}
		return nullptr;
	}

	ErrorCode StartOfFrameSegment::load(const uint8_t* buffer, int offset)
	{
		if (!JpegAccess::VerifyReadUint16(buffer, offset, Com))
		{
			return J2K_COM_DOESNT_MATCH;
		}
		if (!JpegAccess::VerifyReadUint16(buffer, offset + 2, Lcom))
		{
			return J2K_LCOM_DOESNT_MATCH;
		}
		if (!JpegAccess::VerifyReadUint16(buffer, offset + 4, Rcom))
		{
			return J2K_RCOM_DOESNT_MATCH;
		}
		this->Ccom1 = JpegAccess::ReadUint16(buffer, offset + 6);
		this->Ccom2 = JpegAccess::ReadUint32(buffer, offset + 8);
		this->Ccom3 = JpegAccess::ReadUint32(buffer, offset + 12);
		return SUCCESS;
	}

	void PacketLength::save(ostream& stream) const
	{
		uint8_t buffer[4];
		uint32_t clone = value;
		int index = 0;
		do
		{
			buffer[index++] = (clone & 127) | 128;
			clone = clone >> 7;
		} while(clone != 0);
			
		for(int j = index - 1; j > 0; j--)
		{
			stream.write((const char*)buffer + j, 1);
		}

		buffer[0] = buffer[0] & 127;
		stream.write((const char*)buffer, 1);
	}

	uint32_t PacketLength::size() const
	{
		uint32_t clone = value;
		int result = 0;
		do
		{
			clone = clone >> 7;
			result++;
		} while(clone != 0);
		return result;
	}

	ErrorCode PacketLengthTilePartHeader::load(const uint8_t* buffer, int offset)
	{
		if (!JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID))
		{
			return J2K_PLT_DOESNT_MATCH;
		}
		Lplt = JpegAccess::ReadUint16(buffer, offset + 2);
		Zplt = JpegAccess::ReadUint8(buffer, offset + 4);
		packetLengths.clear();

		bool terminate = false;
		uint32_t currentValue = 0;
		for(int i = 0; i < Lplt - 3; i++)
		{
			uint8_t cb = JpegAccess::ReadUint8(buffer, offset + 5 + i);
			terminate = (cb & (1 << 7)) == 0;
			cb = cb & 127;

			if (terminate)
			{
				terminate = false;
				currentValue += cb;
				packetLengths.push_back(PacketLength(currentValue));
				currentValue = 0;
			}
			else
			{
				currentValue += cb;
				currentValue *= 128;
			}
		}
		return SUCCESS;
	}

	void PacketLengthTilePartHeader::save(ostream& stream) const
	{
		JpegAccess::WriteUint16(stream, MARKER_ID);
		JpegAccess::WriteUint16(stream, Lplt);
		JpegAccess::WriteUint8(stream, Zplt);
		for (vector<PacketLength>::const_iterator it = this->packetLengths.begin(); it != this->packetLengths.end(); ++it) {
			it->save(stream);
		}
	}
	
	void TilePart::save(ostream& stream) const
	{
		JpegAccess::WriteUint16(stream, MARKER_ID);
		JpegAccess::WriteUint16(stream, Lsot);
		JpegAccess::WriteUint16(stream, Isot);
		JpegAccess::WriteUint32(stream, size());
		JpegAccess::WriteUint8(stream, TPsot);
		JpegAccess::WriteUint8(stream, TNsot);
		for (vector<J2KPartPtr>::const_iterator it = this->markers.begin(); it != this->markers.end(); ++it) {
			it->get()->save(stream);
		}
		stream.write((const char*)Raw.data(), Raw.size());
	}

	ErrorCode TilePart::load(const uint8_t* buffer, int offset)
	{
		int index = offset;
		if (!JpegAccess::VerifyReadUint16(buffer, index, MARKER_ID))
		{
			return J2K_SOT_DOESNT_MATCH;
		}
		index += 2;
		if (!JpegAccess::VerifyReadUint16(buffer, index, Lsot))
		{
			return J2K_LSOT_DOESNT_MATCH;
		}
		index += 2;

		this->Isot = JpegAccess::ReadUint16(buffer, index);
		index += 2;

		uint32_t Psot = JpegAccess::ReadUint32(buffer, index);
		index += 4;

		this->TPsot = JpegAccess::ReadUint8(buffer, index);
		index++;

		this->TNsot = JpegAccess::ReadUint8(buffer, index);
		index++;

		this->markers.clear();
		while (true)
		{
			uint16_t marker = JpegAccess::ReadUint16(buffer, index);
			J2KPartPtr part = J2KPart::createByMarkerId(marker);
			if (part == nullptr)
			{
				if (!JpegAccess::VerifyReadUint16(buffer, index, J2KMarkers::SOD))
				{
					return J2K_SOD_DOESNT_MATCH;
				}
				break;
			}
			else
			{
				part.get()->load(buffer, index);
				markers.push_back(part);
				index += part.get()->size();
			}
		}



		this->Raw.clear();
		this->Raw.insert(this->Raw.end(), buffer + index, buffer + offset + Psot);

		return SUCCESS;
	}

	ComponentHeader::ComponentHeader(const ComponentHeader& header)
	{
		this->Ssiz = header.Ssiz;
		this->XRsiz = header.XRsiz;
		this->YRsiz = header.YRsiz;
	}

	void ComponentHeader::save(ostream& stream) const
	{
		JpegAccess::WriteUint8(stream, Ssiz);
		JpegAccess::WriteUint8(stream, XRsiz);
		JpegAccess::WriteUint8(stream, YRsiz);
	}

	void ComponentHeader::load(const uint8_t* buffer, int offset)
	{
		this->Ssiz = buffer[offset];
		this->XRsiz = buffer[offset + 1];
		this->YRsiz = buffer[offset + 2];
	}

	void Header::save(ostream& stream) const
	{
		JpegAccess::WriteUint16(stream, MARKER_ID);
		JpegAccess::WriteUint16(stream, Lsiz);
		JpegAccess::WriteUint16(stream, Rsiz);
		JpegAccess::WriteUint32(stream, Xsiz);
		JpegAccess::WriteUint32(stream, Ysiz);
		JpegAccess::WriteUint32(stream, XOsiz);
		JpegAccess::WriteUint32(stream, YOsiz);
		JpegAccess::WriteUint32(stream, XTsiz);
		JpegAccess::WriteUint32(stream, YTsiz);
		JpegAccess::WriteUint32(stream, XTOsiz);
		JpegAccess::WriteUint32(stream, YTOsiz);
		JpegAccess::WriteUint16(stream, Csiz);

		for (vector<ComponentHeader>::const_iterator it = Components.begin(); it != Components.end(); ++it) {
			it->save(stream);
		}
	}

	ErrorCode Header::load(const uint8_t* buffer, int offset)
	{
		if (!JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID))
		{
			return J2K_SIZ_DOESNT_MATCH;
		}
		this->Lsiz = JpegAccess::ReadUint16(buffer, offset + 2);
		this->Rsiz = JpegAccess::ReadUint16(buffer, offset + 4);
		this->Xsiz = JpegAccess::ReadUint32(buffer, offset + 6);
		this->Ysiz = JpegAccess::ReadUint32(buffer, offset + 10);
		this->XOsiz = JpegAccess::ReadUint32(buffer, offset + 14);
		this->YOsiz = JpegAccess::ReadUint32(buffer, offset + 18);
		this->XTsiz = JpegAccess::ReadUint32(buffer, offset + 22);
		this->YTsiz = JpegAccess::ReadUint32(buffer, offset + 26);
		this->XTOsiz = JpegAccess::ReadUint32(buffer, offset + 30);
		this->YTOsiz = JpegAccess::ReadUint32(buffer, offset + 34);
		this->Csiz = JpegAccess::ReadUint16(buffer, offset + 38);

		this->Components.clear();
		if (this->Csiz > 0)
		{
			for (uint16_t i = 0; i < this->Csiz; i++)
			{
				ComponentHeader componentHeader;
				componentHeader.load(buffer, offset + 40 + i * 3);
				this->Components.push_back(componentHeader);
			}
		}

		return SUCCESS;
	}

	void CodingStyleDefault::save(ostream& stream) const
	{
		JpegAccess::WriteUint16(stream, MARKER_ID);
		JpegAccess::WriteUint16(stream, Lcod);
		JpegAccess::WriteUint8(stream, Scod);
		JpegAccess::WriteUint8(stream, ProgressionOrder);
		JpegAccess::WriteUint16(stream, NumberOfLayers);
		JpegAccess::WriteUint8(stream, MultipleComponentTransformation);
		JpegAccess::WriteUint8(stream, NumberOfDecompositionLevels);
		JpegAccess::WriteUint8(stream, CodeBlockWidth);
		JpegAccess::WriteUint8(stream, CodeBlockHeight);
		JpegAccess::WriteUint8(stream, CodeBlockStyle);
		JpegAccess::WriteUint8(stream, Transformation);
		if (isEntropyCoderWithDefinedPrecints())
		{
			stream.write((const char*)PrecintSizes.data(), PrecintSizes.size());
		}
	}

	ErrorCode CodingStyleDefault::load(const uint8_t* buffer, int offset)
	{
		if (!JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID))
		{
			return J2K_COD_DOESNT_MATCH;
		}
		this->Lcod = JpegAccess::ReadUint16(buffer, offset + 2);
		this->Scod = JpegAccess::ReadUint8(buffer, offset + 4);
		this->ProgressionOrder = JpegAccess::ReadUint8(buffer, offset + 5);
		this->NumberOfLayers = JpegAccess::ReadUint16(buffer, offset + 6);
		this->MultipleComponentTransformation = JpegAccess::ReadUint8(buffer, offset + 8);
		this->NumberOfDecompositionLevels = JpegAccess::ReadUint8(buffer, offset + 9);
		this->CodeBlockWidth = JpegAccess::ReadUint8(buffer, offset + 10);
		this->CodeBlockHeight = JpegAccess::ReadUint8(buffer, offset + 11);
		this->CodeBlockStyle = JpegAccess::ReadUint8(buffer, offset + 12);
		this->Transformation = JpegAccess::ReadUint8(buffer, offset + 13);

		this->PrecintSizes.clear();
		if (this->isEntropyCoderWithDefinedPrecints())
		{
			this->PrecintSizes.insert(this->PrecintSizes.end(), buffer + offset + 14, buffer + offset + this->size());
		}
		return SUCCESS;
	}

	void QuantizationDefaultParameter::save(ostream& stream) const
	{
		JpegAccess::WriteUint16(stream, MARKER_ID);
		JpegAccess::WriteUint16(stream, Lqcd);
		JpegAccess::WriteUint8(stream, Sqcd);

		stream.write((const char*)Raw.data(), Raw.size());
	}

	ErrorCode QuantizationDefaultParameter::load(const uint8_t* buffer, int offset)
	{
		if (!JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID))
		{
			return J2K_QCD_DOESNT_MATCH;
		}
		this->Lqcd = JpegAccess::ReadUint16(buffer, offset + 2);
		this->Sqcd = JpegAccess::ReadUint8(buffer, offset + 4);
		this->Raw.clear();
		this->Raw.insert(this->Raw.end(), buffer + offset + 5, buffer + offset + this->size());
		return SUCCESS;
	}

	void QuantizationComponent::save(ostream& stream) const
	{
		JpegAccess::WriteUint16(stream, MARKER_ID);
		JpegAccess::WriteUint16(stream, Lqcc);
		stream.write((const char*)Raw.data(), Raw.size());
	}

	ErrorCode QuantizationComponent::load(const uint8_t* buffer, int offset)
	{
		if (!JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID))
		{
			return J2K_QCC_DOESNT_MATCH;
		}
		this->Lqcc = JpegAccess::ReadUint16(buffer, offset + 2);
		this->Raw.clear();
		this->Raw.insert(this->Raw.end(), buffer + offset + 4, buffer + offset + this->size());
		return SUCCESS;
	}

	void Comment::save(ostream& stream) const
	{
		JpegAccess::WriteUint16(stream, MARKER_ID);
		JpegAccess::WriteUint16(stream, Lcom);
		JpegAccess::WriteUint16(stream, Rcom);
		stream.write((const char*)Raw.data(), Raw.size());
	}

	void Comment::load(const string& text)
	{
		Lcom = text.length() + 4;
		Rcom = 1;
		Raw.clear();
		Raw.insert(this->Raw.end(), text.begin(), text.end());
	}
	
	ErrorCode Comment::load(const uint8_t* buffer, int offset)
	{
		if (!JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID))
		{
			return J2K_COM_DOESNT_MATCH;
		}
		this->Lcom = JpegAccess::ReadUint16(buffer, offset + 2);
		this->Rcom = JpegAccess::ReadUint16(buffer, offset + 4);
		this->Raw.clear();
		this->Raw.insert(this->Raw.end(), buffer + offset + 6, buffer + offset + this->size());
		return SUCCESS;
	}

	void CodingStyleComponent::save(ostream& stream) const
	{
		JpegAccess::WriteUint16(stream, MARKER_ID);
		JpegAccess::WriteUint16(stream, Lcoc);
		stream.write((const char*)Raw.data(), Raw.size());
	}
	
	ErrorCode CodingStyleComponent::load(const uint8_t* buffer, int offset)
	{
		if (!JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID))
		{
			return J2K_COC_DOESNT_MATCH;
		}

		this->Lcoc = JpegAccess::ReadUint16(buffer, offset + 2);
		this->Raw.clear();
		this->Raw.insert(this->Raw.end(), buffer + offset + 4, buffer + offset + this->size());
		return SUCCESS;
	}
	
	void J2KFile::save(ostream& stream) const
	{
		JpegAccess::WriteUint16(stream, MARKER_ID);
		header.save(stream);
		codingStyleDefault.save(stream);
		quantizationDefaultParameter.save(stream);
		for (vector<Comment>::const_iterator it = comments.begin(); it != comments.end(); ++it) {
			it->save(stream);
		}
		for (vector<QuantizationComponent>::const_iterator it = componentQccs.begin(); it != componentQccs.end(); ++it) {
			it->save(stream);
		}
		for (vector<TilePart>::const_iterator it = tiles.begin(); it != tiles.end(); ++it) {
			it->save(stream);
		}
		JpegAccess::WriteUint16(stream, EOC);
	}

	uint32_t J2KFile::size() const
	{
		uint32_t result = 4 + header.size() + codingStyleDefault.size() + quantizationDefaultParameter.size();
		BOOST_FOREACH(const Comment& ptr, comments)
		{
			result += ptr.size();
		}
		BOOST_FOREACH(const QuantizationComponent& ptr, componentQccs)
		{
			result += ptr.size();
		}
		BOOST_FOREACH(const TilePart& ptr, tiles)
		{
			result += ptr.size();
		}
		return result;
	}

	ErrorCode J2KFile::load(const uint8_t* buffer, int offset)
	{
		if (!JpegAccess::VerifyReadUint16(buffer, offset, MARKER_ID))
		{
			return J2K_SOC_DOESNT_MATCH;
		}
		offset += 2;

		ErrorCode result;
		result = this->header.load(buffer, offset);
		if (result != SUCCESS)
		{
			return result;
		}
		offset += this->header.size();

		result = this->codingStyleDefault.load(buffer, offset);
		if (result != SUCCESS)
		{
			return result;
		}
		offset += this->codingStyleDefault.size();

		result = this->quantizationDefaultParameter.load(buffer, offset);
		if (result != SUCCESS)
		{
			return result;
		}
		offset += this->quantizationDefaultParameter.size();

		this->comments.clear();
		while (Comment::isValid(buffer, offset))
		{
			Comment c;
			result = c.load(buffer, offset);
			if (result != SUCCESS)
			{
				return result;
			}
			this->comments.push_back(c);
			offset += c.size();
		}

		this->componentQccs.clear();
		while (QuantizationComponent::isValid(buffer, offset))
		{
			QuantizationComponent qcc;
			result = qcc.load(buffer, offset);
			if (result != SUCCESS)
			{
				return result;
			}
			this->componentQccs.push_back(qcc);
			offset += qcc.size();
		}

		this->tiles.clear();
		while (TilePart::isValid(buffer, offset))
		{
			TilePart sot;
			result = sot.load(buffer, offset);
			if (result != SUCCESS)
			{
				return result;
			}
			this->tiles.push_back(sot);
			offset += sot.size();
		}

		if (!JpegAccess::VerifyReadUint16(buffer, offset, EOC))
		{
			return J2K_EOC_DOESNT_MATCH;
		}

		return SUCCESS;
	}
}