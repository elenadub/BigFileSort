#include "ThreadedQueue.hpp"
#include <thread>


ThreadedQueue::ThreadedQueue(const size_t num_threads)
	: m_enabled(true)
	, m_done(0)
{
	auto process_thread_worker = std::bind(&ThreadedQueue::process_thread, this);
	for(size_t i = 0; i < num_threads; ++i)
	{
		new std::thread(process_thread_worker);
	}
}


ThreadedQueue::~ThreadedQueue()
{
	std::unique_lock<std::mutex> locker(m_queue_lock);
	m_enabled = false;
	m_queue_check.notify_all();
}


void ThreadedQueue::wait(const size_t num_tasks)
{
	std::unique_lock<std::mutex> locker(m_lock_done);
	while(m_done < num_tasks)
	{
		m_done_check.wait(locker);
	}
}


void ThreadedQueue::reset_done_counter()
{
	std::unique_lock<std::mutex> locker(m_lock_done);
	m_done = 0;
}


void ThreadedQueue::process_thread()
{
	for(std::unique_lock<std::mutex> locker(m_queue_lock); m_enabled; locker.lock())
	{
		while(m_queue.empty())
		{
			m_queue_check.wait(locker);

			if(!m_enabled) return;

			continue;
		}

		auto func(m_queue.front());
		m_queue.pop_front();

		locker.unlock();

		if(func) func();

		{
			std::unique_lock<std::mutex> locker_done(m_lock_done);
			++m_done;
			m_done_check.notify_one();
		}
	}
}


void ThreadedQueue::enqueue(const std::function<void()>& task)
{
	std::unique_lock<std::mutex> locker(m_queue_lock);
	m_queue.push_back(task);
	m_queue_check.notify_one();
}
