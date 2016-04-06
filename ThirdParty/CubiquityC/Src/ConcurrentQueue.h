// Based on http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
//
// Note from the comments on licensing: "Yes, you can just copy the code presented here and use
// it for whatever you like. There won't be any licensing issues. I'm glad you find it helpful."

#ifndef CUBIQUITY_CONCURRENTQUEUE_H
#define CUBIQUITY_CONCURRENTQUEUE_H

#include <cstdint>
#include <queue>

namespace Cubiquity
{
	// FIXME: THIS QUEUE IS CURRENTLY NOT THREAD SAFE DESPITE IT'S NAME. THE THREADING CODE WAS BASED ON BOOST AND IS COMMENTED OUT
	// PENDING A STD C++ VERSION. THE STD FUNCTIONS/TYPES DO NOT HAVE A ONE-TO-ONE CORRESPONDANCE TO THE BOOST FUNCTIONS/TYPES SO
	// I WANT TO TAKE SOME CARE OVER THIS.
	template<class Data, class Compare>
	class concurrent_queue
	{
	private:
		std::priority_queue<Data, std::vector<Data>, Compare> the_queue;
		//mutable boost::mutex the_mutex;
		//boost::condition_variable the_condition_variable;
	public:
		void push(Data const& data)
		{
			//boost::mutex::scoped_lock lock(the_mutex);
			the_queue.push(data);
			//lock.unlock();
			//the_condition_variable.notify_one();
		}

		bool empty() const
		{
			//boost::mutex::scoped_lock lock(the_mutex);
			return the_queue.empty();
		}

		uint32_t size() const
		{
			//boost::mutex::scoped_lock lock(the_mutex);
			return the_queue.size();
		}

		bool try_pop(Data& popped_value)
		{
			//boost::mutex::scoped_lock lock(the_mutex);
			if(the_queue.empty())
			{
				return false;
			}
        
			popped_value=the_queue.top();
			the_queue.pop();
			return true;
		}

		void wait_and_pop(Data& popped_value)
		{
			//boost::mutex::scoped_lock lock(the_mutex);
			//while(the_queue.empty())
			//{
			//	the_condition_variable.wait(lock);
			//}
        
			popped_value=the_queue.top();
			the_queue.pop();
		}

	};
}

#endif //CUBIQUITY_CONCURRENTQUEUE_H
