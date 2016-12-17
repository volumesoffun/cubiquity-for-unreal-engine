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

#ifndef CUBIQUITY_BACKGROUNDTASKPROCESSOR_H_
#define CUBIQUITY_BACKGROUNDTASKPROCESSOR_H_

#include "ConcurrentQueue.h"
#include "TaskProcessor.h"

//#include "boost/thread.hpp"

#include <list>

namespace Cubiquity
{
	/*class BackgroundTaskProcessor : public TaskProcessor
	{
	public:
		BackgroundTaskProcessor(uint32_t noOfThreads);
		virtual ~BackgroundTaskProcessor();

		void addTask(Task* task);

		bool hasAnyFinishedTasks(void);
		Task* removeFirstFinishedTask(void);

		void processTasks(void);

		concurrent_queue<Task*, TaskSortCriterion> mPendingTasks;

		std::list<boost::thread*> mThreads;
	};

	extern BackgroundTaskProcessor gBackgroundTaskProcessor;*/

	class BackgroundTaskProcessor : public TaskProcessor
	{
	public:
		BackgroundTaskProcessor();
		virtual ~BackgroundTaskProcessor();

		void addTask(Task* task);
		bool hasTasks(void);

		virtual void processOneTask(void)/* = 0*/;
		virtual void processAllTasks(void)/* = 0*/;

		std::list<Task*> mPendingTasks;
	};

	//extern BackgroundTaskProcessor gBackgroundTaskProcessor;
}

#endif //CUBIQUITY_BACKGROUNDTASKPROCESSOR_H_
