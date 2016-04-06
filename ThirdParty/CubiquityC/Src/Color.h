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

#ifndef __COLOUR_H__
#define __COLOUR_H__

#include "PolyVox/CubicSurfaceExtractor.h"
#include "PolyVox/Vertex.h"

#include "BitField.h"

#include <cassert>

namespace Cubiquity
{
	class Color
	{
	public:

		static const uint32_t MaxInOutValue = 255;

		static const uint32_t RedMSB = 31;
		static const uint32_t RedLSB = 27;		
		static const uint32_t GreenMSB = 26;
		static const uint32_t GreenLSB = 21;
		static const uint32_t BlueMSB = 20;
		static const uint32_t BlueLSB = 16;
		static const uint32_t AlphaMSB = 15;
		static const uint32_t AlphaLSB = 12;
		
		static const uint32_t NoOfRedBits = RedMSB - RedLSB + 1;
		static const uint32_t NoOfGreenBits = GreenMSB - GreenLSB + 1;
		static const uint32_t NoOfBlueBits = BlueMSB - BlueLSB + 1;
		static const uint32_t NoOfAlphaBits = AlphaMSB - AlphaLSB + 1;
		
		static const uint32_t RedScaleFactor = MaxInOutValue / ((0x01 << NoOfRedBits) - 1);
		static const uint32_t GreenScaleFactor = MaxInOutValue / ((0x01 << NoOfGreenBits) - 1);
		static const uint32_t BlueScaleFactor = MaxInOutValue / ((0x01 << NoOfBlueBits) - 1);
		static const uint32_t AlphaScaleFactor = MaxInOutValue / ((0x01 << NoOfAlphaBits) - 1);

		Color()
		{
			mChannels.clearAllBits();
		}

		Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = MaxInOutValue)
		{
			mChannels.clearAllBits();
			setColor(red, green, blue, alpha);
		}

		bool operator==(const Color& rhs) const throw()
		{
			return mChannels == rhs.mChannels;
		}

		bool operator!=(const Color& rhs) const throw()
		{
			return !(*this == rhs);
		}

		uint8_t getRed(void)
		{
			return static_cast<uint8_t>(mChannels.getBits(RedMSB, RedLSB) * RedScaleFactor);
		}

		uint8_t getGreen(void)
		{
			return static_cast<uint8_t>(mChannels.getBits(GreenMSB, GreenLSB) * GreenScaleFactor);
		}

		uint8_t getBlue(void)
		{
			return static_cast<uint8_t>(mChannels.getBits(BlueMSB, BlueLSB) * BlueScaleFactor);
		}

		uint8_t getAlpha(void)
		{
			return static_cast<uint8_t>(mChannels.getBits(AlphaMSB, AlphaLSB) * AlphaScaleFactor);
		}

		void setRed(uint8_t value)
		{
			mChannels.setBits(RedMSB, RedLSB, value / RedScaleFactor);
		}

		void setGreen(uint8_t value)
		{
			mChannels.setBits(GreenMSB, GreenLSB, value / GreenScaleFactor);
		}

		void setBlue(uint8_t value)
		{
			mChannels.setBits(BlueMSB, BlueLSB, value / BlueScaleFactor);
		}

		void setAlpha(uint8_t value)
		{
			mChannels.setBits(AlphaMSB, AlphaLSB, value / AlphaScaleFactor);
		}

		void setColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
		{
			setRed(red);
			setGreen(green);
			setBlue(blue);
			setAlpha(alpha);
		}

	private:
		BitField<uint32_t> mChannels;
	};

	// These operations are used by the smooth raycast to perform trilinear interpolation.
	// We never actually do that on this type (because colors are used for cubic surfaces
	// not smooth ones) but our use of templates means that this code path still gets compiled.
	// The actual implementations simply assert if they are ever called by mistake.
	Color operator+(const Color& lhs, const Color& rhs) throw();
	Color operator-(const Color& lhs, const Color& rhs) throw();
	Color operator*(const Color& lhs, float rhs) throw();
	Color operator/(const Color& lhs, float rhs) throw();

	class ColoredCubesIsQuadNeeded
	{
	public:
		bool operator()(Color back, Color front, Color& materialToUse)
		{
			if((back.getAlpha() > 0) && (front.getAlpha() == 0))
			{
				materialToUse = back;
				return true;
			}
			else
			{
				return false;
			}
		}
	};

	typedef ::PolyVox::CubicVertex<Color> ColoredCubesVertex;
	typedef ::PolyVox::Mesh<ColoredCubesVertex, uint16_t> ColoredCubesMesh;
}

#endif //__COLOUR_H__
