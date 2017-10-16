#include "BigFileSorter.hpp"
#include <iostream>
#include <random>
#include <fstream>


BigFileSorter::BigFileSorter(
	const std::string file_name,
	const size_t block_size
)
	: FILE_NAME(file_name)
	, BLOCK_LENGTH(block_size - block_size % 2)
	, BLOCK_LENGTH_HF(BLOCK_LENGTH / 2)
	, BLOCK_SIZE(BLOCK_LENGTH * TYPE_SIZE)
	, BLOCK_SIZE_HF(BLOCK_SIZE / 2)
	, m_file_size(0)
	, m_numbers_qty(0)
	, m_blocks_qty(0)
	, m_total_blocks_qty(0)
	, m_tail_length(0)
{
}


void BigFileSorter::generate(const size_t numbers_qty)
{
	m_numbers_qty = numbers_qty;
	m_file_size = m_numbers_qty * TYPE_SIZE;
	m_total_blocks_qty = m_blocks_qty = m_file_size / BLOCK_SIZE;
	m_tail_length = m_numbers_qty - m_blocks_qty * BLOCK_LENGTH;

	if(m_file_size % BLOCK_SIZE > 0) m_total_blocks_qty += 1;

	std::fstream file(FILE_NAME, std::ios::in | std::ios::out | std::fstream::binary | std::fstream::trunc);

	std::mt19937_64 generator(time(0));
	std::uniform_int_distribution<int64_t> uid(
		std::numeric_limits<int64_t>::min(),
		std::numeric_limits<int64_t>::max()
	);

	uint64_t* block = new uint64_t[BLOCK_LENGTH];

	for(size_t i = 0, curr_block_length = BLOCK_LENGTH; i < m_total_blocks_qty; ++i)
	{
		if((i + 1) * curr_block_length > m_numbers_qty)
		{
			curr_block_length = m_tail_length;
		}

		for(size_t j = 0; j < curr_block_length; ++j)
		{
			block[j] = uid(generator);
		}
		file.seekp(BLOCK_SIZE * i);
		file.write(reinterpret_cast<std::ios::char_type *>(block), curr_block_length * TYPE_SIZE);
	}
	file.close();
}


void BigFileSorter::print() const
{
	std::ifstream file(FILE_NAME, std::ifstream::binary);

	if(!file.is_open())
	{
		std::cout << " --- Can't open file '" << FILE_NAME << "'" << std::endl;
		return;
	}

	uint64_t* block = new uint64_t[BLOCK_LENGTH];

	for(size_t i = 0, curr_block_length = BLOCK_LENGTH; i < m_total_blocks_qty; ++i)
	{
		if((i + 1) * curr_block_length > m_numbers_qty)
		{
			curr_block_length = m_tail_length;
		}

		file.seekg(BLOCK_SIZE * i);
		file.read(reinterpret_cast<std::ios::char_type *>(block), curr_block_length * TYPE_SIZE);

		for(size_t j = 0; j < curr_block_length; ++j)
		{
			std::cout << block[j] << std::endl;
		}
	}
	file.close();
}


bool BigFileSorter::is_valid()
{

	std::ifstream file(FILE_NAME, std::ifstream::binary);

	if(!file.is_open())
	{
		std::cout << " --- Can't open file '" << FILE_NAME << "'" << std::endl;
		return false;
	}

	uint64_t* block = new uint64_t[BLOCK_LENGTH];

	for(size_t i = 0, curr_block_length = BLOCK_LENGTH; i < m_total_blocks_qty; ++i)
	{
		if((i + 1) * curr_block_length > m_numbers_qty)
		{
			curr_block_length = m_tail_length;
		}

		file.seekg(BLOCK_SIZE * i);
		file.read(reinterpret_cast<std::ios::char_type *>(block), curr_block_length * TYPE_SIZE);

		for(size_t j = 0; j < curr_block_length - 1; ++j)
		{
			if(comparator(block + j, block + (j + 1)) > 0) return false;
		}
	}
	file.close();
	return true;
}


int BigFileSorter::comparator(const void* p1, const void* p2)
{
	return *static_cast<const uint64_t *>(p1) > *static_cast<const uint64_t *>(p2)
		? 1
		: -1
	;
}


void BigFileSorter::process_file (
	std::fstream& file,
	uint64_t* block,
	const uint64_t from,
	const uint64_t to,
	const uint64_t seek_offset,
	const uint64_t process_size,
	const uint64_t rw_block_offset
)
{
	for(uint64_t i = from; i < to; ++i)
	{
		file.seekg(seek_offset + BLOCK_SIZE * i);
		file.read(reinterpret_cast<std::ios::char_type *>(block + rw_block_offset), process_size);

		qsort(block, BLOCK_LENGTH, TYPE_SIZE, comparator);

		file.seekp(seek_offset + BLOCK_SIZE * i);
		file.write(reinterpret_cast<std::ios::char_type *>(block + rw_block_offset), process_size);
	}
}


void BigFileSorter::sort()
{
	std::fstream file(FILE_NAME, std::ios::in | std::ios::out | std::ios::binary);
	uint64_t* block = new uint64_t[BLOCK_LENGTH];

	for(int64_t blocks_qty = m_blocks_qty, step = 0; blocks_qty > 0; --blocks_qty, ++step)
	{
		process_file(file, block, 0, blocks_qty, BLOCK_SIZE_HF * step, BLOCK_SIZE, 0);

		file.seekg(BLOCK_SIZE_HF * step);
		file.read(reinterpret_cast<std::ios::char_type *>(block), BLOCK_SIZE_HF);

		process_file(file, block, 1, blocks_qty, BLOCK_SIZE_HF * step, BLOCK_SIZE_HF, BLOCK_LENGTH_HF);

		file.seekp(BLOCK_SIZE_HF * step);
		file.write(reinterpret_cast<std::ios::char_type *>(block), BLOCK_SIZE_HF);

		const uint64_t seek_offset = BLOCK_SIZE_HF * step + BLOCK_SIZE * blocks_qty - BLOCK_SIZE_HF;

		file.seekg(seek_offset);
		file.read(reinterpret_cast<std::ios::char_type *>(block + BLOCK_LENGTH_HF), BLOCK_SIZE_HF);

		process_file(file, block, 0, blocks_qty - 1, BLOCK_SIZE_HF * (step + 1), BLOCK_SIZE_HF, 0);

		file.seekp(seek_offset);
		file.write(reinterpret_cast<std::ios::char_type *>(block + BLOCK_LENGTH_HF), BLOCK_SIZE_HF);
	}

	// TODO: process tail

	file.close();
	delete block;
}

