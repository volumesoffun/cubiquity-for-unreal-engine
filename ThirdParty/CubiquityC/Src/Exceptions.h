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

#ifndef __CUBIQUITY_EXCEPTIONS_H__
#define __CUBIQUITY_EXCEPTIONS_H__

namespace Cubiquity
{
	// Exceptions listed in the order we added them, as this helps match the order
	// to the defines in the C interface (where they are also ordered numerically).

	class DatabaseError : public std::runtime_error
	{
	public:
		DatabaseError(const std::string& what_arg)
			:runtime_error(what_arg)
		{
		}
	};

	class CompressionError : public std::runtime_error
	{
	public:
		CompressionError(const std::string& what_arg)
			:runtime_error(what_arg)
		{
		}
	};
}

#endif //__CUBIQUITY_EXCEPTIONS_H__
