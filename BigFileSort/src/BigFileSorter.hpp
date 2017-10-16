#ifndef BIGFILESORTER_HPP_
#define BIGFILESORTER_HPP_
#include <functional>


class BigFileSorter
{
public:
	/** \brief Конструктор.
	 * \param block_size Количество элементов, с которыми можно работать одновременно.
	 * \param file_name Имя файла, с которым будем работать.
	 */
	BigFileSorter(
		const std::string file_name,
		const size_t block_size
	);

	virtual inline ~BigFileSorter() {};

	/** \brief Сгенерировать или перезаписать файл.
	 * \param numbers_qty Количество элементов в файле.
	 */
	void generate(const size_t numbers_qty);

	/** \brief Отсортировать файл.
	 */
	virtual void sort();

	/** \brief Вывести содержимое файла.
	 */
	void print() const;

	/** \brief Проверить, что файл отсортирован.
	 */
	bool is_valid();

	/** \brief Сравнить 2 элемента.
	 * \details Используется для стандартной функции qsort.
	 * \param el1 1 элемент.
	 * \param el2 2 элемент.
	 */
	static int comparator(const void* el1, const void* el2);

protected:
	const size_t TYPE_SIZE = sizeof(int64_t);	///< Размер типа базового элемента сортировки.
	const std::string FILE_NAME;		///< Имя файла, с которым будет вестись работа.
	const size_t BLOCK_LENGTH;			///< Размер буффера в количестве элементов, который можем себе позволить.
	const size_t BLOCK_LENGTH_HF;		///< Половина размера буффера в количестве элементов.
	const uint64_t BLOCK_SIZE;			///< Размер буффера в байтах, который можем себе позволить.
	const uint64_t BLOCK_SIZE_HF;		///< Половина размера буффера в байтах.

	uint64_t m_file_size;				///< Размер файла.
	size_t m_numbers_qty;				///< Количество элементов в файле.
	size_t m_blocks_qty;				///< Общее количество блоков, на которые можно поделить файл.
	size_t m_total_blocks_qty;			///< Общее количество блоков + 1, если нацело не делится.
	size_t m_tail_length;				///< Длина некратного не сортируемого остатка.

	/** \brief Обработать файл в определенной последовательности.
	 * \param file Файл, который будем обрабатывать.
	 * \param block Буффер.
	 * \param from Нижняя граница итератора.
	 * \param to Верхняя граница итератора.
	 * \param seek_offset Смещение указателя чтения/записи в файл.
	 * \param process_size Размер в байтах новых данных, считываемых из файла для обработки.
	 * \param rw_block_offset Смещение относительно указателя блока, с которым нужно читать/писать в блок.
	 */
	virtual void process_file(
		std::fstream& file,
		uint64_t* block,
		const uint64_t from,
		const uint64_t to,
		const uint64_t seek_offset,
		const uint64_t process_size,
		const uint64_t rw_block_offset
	);
};

#endif /* BIGFILESORTER_HPP_ */
