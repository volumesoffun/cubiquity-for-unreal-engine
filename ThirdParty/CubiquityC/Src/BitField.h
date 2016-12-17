/*******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2016 David Williams and Matthew Williams
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*******************************************************************************/

#ifndef CUBIQUITY_BITFIELD_H
#define CUBIQUITY_BITFIELD_H

#include <limits>

namespace Cubiquity
{
	template <typename StorageType>
	class BitField
	{
	public:
		BitField(StorageType initialValue = 0)
			:mBits(initialValue)
		{
		}

		bool operator==(const BitField& rhs) const throw()
		{
			return mBits == rhs.mBits;
		};

		bool operator!=(const BitField& rhs) const throw()
		{
			return !(*this == rhs);
		}

		StorageType getBits(size_t MSB, size_t LSB) const
		{
			const size_t noOfBitsToGet = (MSB - LSB) + 1;

			// Build a mask containing all '0's except for the least significant bits (which are '1's).
			StorageType mask = (std::numeric_limits<StorageType>::max)(); //Set to all '1's
			mask = mask << noOfBitsToGet; // Insert the required number of '0's for the lower bits
			mask = ~mask; // And invert

			// Move the desired bits into the LSBs and mask them off
			StorageType result = (mBits >> LSB) & mask;

			return result;
		}

		void setBits(size_t MSB, size_t LSB, StorageType bitsToSet)
		{
			const size_t noOfBitsToSet = (MSB - LSB) + 1;

			StorageType mask = (std::numeric_limits<StorageType>::max)(); //Set to all '1's
			mask = mask << noOfBitsToSet; // Insert the required number of '0's for the lower bits
			mask = ~mask; // And invert
			mask = mask << LSB;

			bitsToSet = (bitsToSet << LSB) & mask;

			mBits = (mBits & ~mask) | bitsToSet;
		}

		StorageType allBits(void)
		{
			return mBits;
		}

		void clearAllBits(void)
		{
			mBits = 0;
		}

	private:
		StorageType mBits;
	};
}

#endif //CUBIQUITY_BITFIELD_H
