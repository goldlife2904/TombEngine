#include "framework.h"
#include "Specific/BitField.h"

#include <limits>

using std::vector;

namespace TEN::Utils
{
	constexpr auto BIT_FIELD_SIZE_MAX = std::numeric_limits<uint>::digits;

	BitField::BitField()
	{
	}

	BitField::BitField(uint size)
	{
		size = std::min<uint>(size, BIT_FIELD_SIZE_MAX);
		this->Bits.resize(size);
	}

	BitField::BitField(uint size, uint packedBits)
	{
		size = std::min<uint>(size, BIT_FIELD_SIZE_MAX);
		this->Bits.resize(size);

		for (uint i = 0; i < size; i++)
		{
			uint bit = uint(1 << i);
			if ((packedBits & bit) == bit)
				this->Bits[i] = true;
		}
	}

	uint BitField::GetSize()
	{
		return Bits.size();
	}

	uint BitField::GetPackedBits() const
	{
		uint packedBits = 0;
		for (uint i = 0; i < Bits.size(); i++)
		{
			if (Bits[i])
			{
				uint bit = uint(1 << i);
				packedBits |= bit;
			}
		}

		return packedBits;
	}

	void BitField::Set(const vector<uint>& indices)
	{
		for (auto& index : indices)
		{
			if (index < Bits.size())
				this->Bits[index] = true;
		}
	}

	void BitField::Set(uint index)
	{
		this->Set({ index });
	}

	void BitField::SetAll()
	{
		this->Fill(true);
	}

	void BitField::Clear(const vector<uint>& indices)
	{
		for (auto& index : indices)
		{
			if (index < Bits.size())
				this->Bits[index] = false;
		}
	}
	
	void BitField::Clear(uint index)
	{
		this->Clear({ index });
	}

	void BitField::ClearAll()
	{
		this->Fill(false);
	}
	
	void BitField::Flip(const vector<uint>& indices)
	{
		for (auto& index : indices)
		{
			if (index < Bits.size())
				this->Bits[index].flip();
		}
	}
	
	void BitField::Flip(uint index)
	{
		this->Flip({ index });
	}

	void BitField::FlipAll()
	{
		this->Bits.flip();
	}

	bool BitField::Test(const vector<uint>& indices)
	{
		for (auto& index : indices)
		{
			if (Bits[index])
				return true;
		}

		return false;
	}

	bool BitField::Test(uint index)
	{
		return this->Test({ index });
	}

	bool BitField::TestAny()
	{
		for (auto& bit : this->Bits)
		{
			if (bit)
				return true;
		}

		return false;
	}

	bool BitField::TestAll()
	{
		for (auto& bit : this->Bits)
		{
			if (bit)
				return true;
		}

		return false;
	}

	bool BitField::IsFull()
	{
		for (auto& bit : this->Bits)
		{
			if (!bit)
				return false;
		}

		return true;
	}

	bool BitField::IsEmpty()
	{
		for (auto& bit : this->Bits)
		{
			if (bit)
				return false;
		}

		return true;
	}

	BitField::operator uint() const
	{
		return this->GetPackedBits();
	}

	BitField& BitField::operator =(uint packedBits)
	{
		for (uint i = 0; i < Bits.size(); i++)
		{
			uint bit = uint(1 << i);
			if ((packedBits & bit) == bit)
				this->Bits[i] = true;
			else
				this->Bits[i] = false;
		}

		return *this;
	}

	BitField& BitField::operator |=(uint packedBits)
	{
		for (uint i = 0; i < Bits.size(); i++)
		{
			uint bit = uint(1 << i);
			if ((packedBits & bit) == bit)
				this->Bits[i] = true;
		}

		return *this;
	}
	
	BitField BitField::operator &(uint packedBits)
	{
		BitField newBitField = {};
		vector<uint> indices = {};

		for (uint i = 0; i < Bits.size(); i++)
		{
			uint bit = uint(1 << i);
			if (Bits[i] && (packedBits & bit) == bit)
				indices.push_back(i);
		}

		newBitField.Set(indices);
		return newBitField;
	}

	void BitField::Fill(bool value)
	{
		std::fill(this->Bits.begin(), this->Bits.end(), value);
	}
}
