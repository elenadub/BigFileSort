#ifndef THREADEDQUEUE_HPP_
#define THREADEDQUEUE_HPP_
#include <mutex>
#include <condition_variable>
#include <list>
#include <functional>


class ThreadedQueue
{
public:
	/** \brief Конструктор.
	 * \param num_threads Количество потоков в очереди.
	 */
	ThreadedQueue(const size_t num_threads);
	virtual ~ThreadedQueue();

	/** \brief Сбросить счетчик успешно завершенных задач.
	 */
	void reset_done_counter();

	/** \brief Заставить ждать основной поток, пока задачи в очереди не отработают.
	 * \param num_tasks Количество задач, которые должны отработать успшно.
	 */
	void wait(const size_t num_tasks);

	/** \brief Добавить в очередь свежую задачу.
	 * \param task Задача.
	 */
	void enqueue(const std::function<void()>& task);

private:
	bool m_enabled;							///< Флаг, чтобы при удалении очереди потоки закрылись.
	size_t m_done;							///< Счетчик успешно завершенных задач.
	std::mutex m_lock_done;					///< Синхронизатор счетчика.
	std::mutex m_queue_lock;				///< Синхронизатор данных очереди.
	std::condition_variable m_done_check;	///< Чекер пробуждения основного потока.
	std::condition_variable m_queue_check;	///< Чекер пробуждения одного из дополнительных потоков.

	std::list<std::function<void()>> m_queue;	///< Очередь с тасками.

	/** \brief Рабочая функция потока.
	 */
	void process_thread();
};

#endif /* THREADEDQUEUE_HPP_ */
