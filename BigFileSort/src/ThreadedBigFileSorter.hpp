#ifndef THREADEDBIGFILESORTER_HPP_
#define THREADEDBIGFILESORTER_HPP_
#include "BigFileSorter.hpp"
#include "ThreadedQueue.hpp"


class ThreadedBigFileSorter : public BigFileSorter
{
public:
	/** \brief Конструктор.
	 * \param block_size Количество элементов, с которыми можно работать одновременно.
	 * \param file_name Имя файла, с которым будем работать.
	 * \param num_threads Общее количество потоков, которые можно задействовать во время сортировки.
	 */
	ThreadedBigFileSorter(
		const std::string file_name,
		const size_t block_size,
		const size_t num_threads
	);

	virtual ~ThreadedBigFileSorter();

	/** \brief Отсортировать файл.
	 */
	void sort() override;

protected:
	/** \brief Обработать файл в определенной последовательности.
	 * \param file Файл, который будем обрабатывать.
	 * \param block Буффер.
	 * \param from Нижняя граница итератора.
	 * \param to Верхняя граница итератора.
	 * \param seek_offset Смещение указателя чтения/записи в файл.
	 * \param process_size Размер в байтах новых данных, считываемых из файла для обработки.
	 * \param rw_block_offset Смещение относительно указателя блока, с которым нужно читать/писать в блок.
	 */
	void process_file (
			std::fstream& file,
			uint64_t* block,
			const uint64_t from,
			const uint64_t to,
			const uint64_t seek_offset,
			const uint64_t process_size,
			const uint64_t rw_block_offset
	) override;

private:
	ThreadedQueue* m_queue;	///< Потоковая чередь задач на обработку частей файла.
	std::mutex m_lock_file;	///< Синхронизатор работы с файлом.

	/** \brief Подготовить блок данных.
	 * \param file Файл, который будем обрабатывать.
	 * \param block Блок.
	 * \param seek_offset Смещение указателя чтения/записи в файл.
	 * \param process_size Размер в байтах новых данных, считываемых из файла для подготовки.
	 * \param callback Обработчик подготовленноого блока.
	 */
	void prepare_block(
		std::fstream& file,
		uint64_t* block,
		const uint64_t seek_offset,
		const uint64_t process_size,
		std::function<void()> callback
	);
};

#endif /* THREADEDBIGFILESORTER_HPP_ */
