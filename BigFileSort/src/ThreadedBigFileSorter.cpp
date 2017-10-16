#include "ThreadedBigFileSorter.hpp"
#include <cmath>
#include <fstream>


ThreadedBigFileSorter::ThreadedBigFileSorter(
	const std::string file_name,
	const size_t block_size,
	const size_t num_threads
) : BigFileSorter(file_name, block_size)
{
	m_queue = new ThreadedQueue(num_threads);
}


void ThreadedBigFileSorter::prepare_block(
	std::fstream& file,
	uint64_t* block,
	const uint64_t seek_offset,
	const uint64_t process_size,
	std::function<void()> callback
)
{
	{
		std::unique_lock<std::mutex> locker(m_lock_file);
		file.seekg(seek_offset);
		file.read(reinterpret_cast<std::ios::char_type *>(block), process_size);
	}
	callback();
	{
		std::unique_lock<std::mutex> locker(m_lock_file);
		file.seekp(seek_offset);
		file.write(reinterpret_cast<std::ios::char_type *>(block), process_size);
	}
};


void ThreadedBigFileSorter::process_file (
	std::fstream& file,
	uint64_t* block,
	const uint64_t from,
	const uint64_t to,
	const uint64_t seek_offset,
	const uint64_t process_size,
	const uint64_t rw_block_offset
)
{
	if(BLOCK_SIZE_HF == process_size)
	{
		for(uint64_t i = from; i < to; ++i)
		{
			uint64_t curr_seek_offset = seek_offset + BLOCK_SIZE * i;
			prepare_block(file, block + rw_block_offset, curr_seek_offset, process_size, [this, block] {
				qsort(block, BLOCK_LENGTH, TYPE_SIZE, comparator);
			});
		}
	}
	else
	{
		for(uint64_t i = from; i < to; ++i)
		{
			uint64_t curr_seek_offset = seek_offset + BLOCK_SIZE * i;
			uint64_t* new_buf = new uint64_t[BLOCK_LENGTH];
			m_queue->enqueue([this, &file, new_buf, rw_block_offset, curr_seek_offset, process_size] {
				prepare_block(file, new_buf + rw_block_offset, curr_seek_offset, process_size, [this, new_buf] {
					qsort(new_buf, BLOCK_LENGTH, TYPE_SIZE, comparator);
				});
				delete new_buf;
			});
		}
	}
}


void ThreadedBigFileSorter::sort()
{
	std::fstream file(FILE_NAME, std::ios::in | std::ios::out | std::ios::binary);
	uint64_t* block = new uint64_t[BLOCK_LENGTH];

	for(int64_t blocks_qty = m_blocks_qty, step = 0; blocks_qty > 0; --blocks_qty, ++step)
	{
		m_queue->reset_done_counter();
		process_file(file, block, 0, blocks_qty, BLOCK_SIZE_HF * step, BLOCK_SIZE, 0);
		m_queue->wait(blocks_qty);

		m_queue->reset_done_counter();

		m_queue->enqueue([&] {
			uint64_t* new_buf = new uint64_t[BLOCK_LENGTH];
			prepare_block(
				file,
				new_buf,
				BLOCK_SIZE_HF * step,
				BLOCK_SIZE_HF,
				[&] { process_file(file, new_buf, 1, blocks_qty, BLOCK_SIZE_HF * step, BLOCK_SIZE_HF, BLOCK_LENGTH_HF); }
			);
			delete new_buf;
		});

		m_queue->enqueue([&] {
			uint64_t* new_buf = new uint64_t[BLOCK_LENGTH];
			prepare_block(
				file,
				new_buf + BLOCK_LENGTH_HF,
				BLOCK_SIZE_HF * step + BLOCK_SIZE * blocks_qty - BLOCK_SIZE_HF,
				BLOCK_SIZE_HF,
				[&] { process_file(file, new_buf, 0, blocks_qty - 1, BLOCK_SIZE_HF * (step + 1), BLOCK_SIZE_HF, 0); }
			);
			delete new_buf;
		});

		m_queue->wait(2);
	}

	file.close();
	delete block;
}


ThreadedBigFileSorter::~ThreadedBigFileSorter()
{
	delete m_queue;
}

