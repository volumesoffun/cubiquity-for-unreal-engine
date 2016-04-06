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

#include "Clock.h"

#include "PolyVox/Impl/ErrorHandling.h"

#include <limits>

namespace Cubiquity
{
	// We initialise the clock to a reasonably sized value, so that we can initialise
	// timestamps to small values and be sure that they will immediatly be out-of-date.
	Timestamp Clock::mTimestamp = 100;

	Timestamp Clock::getTimestamp(void)
	{
		// I don't think we need to protect this operation with a mutex. Potentially two threads could enter this function and then
		// leave in a different order to which they entered, but I don't think that matters as long as the timestamps are unique?
		POLYVOX_ASSERT(mTimestamp < (std::numeric_limits<Timestamp>::max)(), "Time stamp is wrapping around.");
		return ++mTimestamp;
	}
}
