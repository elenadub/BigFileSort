#include <iostream>
#include "ThreadedBigFileSorter.hpp"
#include "LifeTimer.hpp"


int main()
{
	const size_t BUFFER_SIZE = 2000;
	const size_t NUMBERS_QTY = 2000000;

	try
	{
		{
			BigFileSorter bfm("big_file", BUFFER_SIZE);
			bfm.generate(NUMBERS_QTY);
			{
				LifeTimer timer(" --- Time sorting in one thread = ");
				bfm.sort();
			}
			if(!bfm.is_valid())
				std::cout << " --- Note! Sorted in one thread file is not valid " << std::endl;
		}
		{
			const size_t NUM_THREADS = 4;
			ThreadedBigFileSorter bfm("big_file", BUFFER_SIZE, NUM_THREADS);
			bfm.generate(NUMBERS_QTY);
			{
				LifeTimer timer(" --- Time sorting with additional threads = ");
				bfm.sort();
			}
			if(!bfm.is_valid())
				std::cout << " --- Note! Sorted with additional threads file is not valid " << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << " --- EXCEPTION! " << e.what() << std::endl;
	};

	return 0;
}
