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

#include "BackgroundTaskProcessor.h"

/*#include "boost/bind.hpp"

namespace Cubiquity
{
	BackgroundTaskProcessor gBackgroundTaskProcessor(1); //Our global instance

	BackgroundTaskProcessor::BackgroundTaskProcessor(uint32_t noOfThreads)
		:TaskProcessor()
	{
		for(uint32_t ct = 0; ct < noOfThreads; ct++)
		{
			boost::thread* taskProcessingThread = new boost::thread(boost::bind(&BackgroundTaskProcessor::processTasks, this));
			mThreads.push_back(taskProcessingThread);
		}
	}

	BackgroundTaskProcessor::~BackgroundTaskProcessor()
	{
		for(std::list<boost::thread*>::iterator threadIter = mThreads.begin(); threadIter != mThreads.end(); threadIter++)
		{
			(*threadIter)->interrupt();
			(*threadIter)->join();
			delete *threadIter;
		}
	}

	void BackgroundTaskProcessor::addTask(Task* task)
	{
		mPendingTasks.push(task);
	}

	void BackgroundTaskProcessor::processTasks(void)
	{
		// This is commented out becaue we're not currently using this class, and having this running
		// causes problems on application shutdown. We probably need a way to stop this background processor.
		//while(true)
		//{
		//	Task* task = 0;
		//	mPendingTasks.wait_and_pop(task);
		//	task->process();
		//}
	}
}*/

namespace Cubiquity
{
	BackgroundTaskProcessor gBackgroundTaskProcessor; //Our global instance

	BackgroundTaskProcessor::BackgroundTaskProcessor()
		:TaskProcessor()
	{
	}

	BackgroundTaskProcessor::~BackgroundTaskProcessor()
	{
		mPendingTasks.clear();
	}

	void BackgroundTaskProcessor::addTask(Task* task)
	{
		mPendingTasks.push_back(task);
	}

	bool BackgroundTaskProcessor::hasTasks(void)
	{
		return mPendingTasks.size() > 0;
	}

	void BackgroundTaskProcessor::processOneTask(void)
	{
		if (mPendingTasks.size() > 0)
		{
			Task* task = mPendingTasks.front();
			mPendingTasks.pop_front();
			task->process();
		}
	}

	void BackgroundTaskProcessor::processAllTasks(void)
	{
		while (mPendingTasks.size() > 0)
		{
			Task* task = mPendingTasks.front();
			mPendingTasks.pop_front();
			task->process();
		}
	}
}
