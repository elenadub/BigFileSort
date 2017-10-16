#include "LifeTimer.hpp"
#include <iostream>


LifeTimer::LifeTimer(const std::string msg)
	: CREATE_TIME(time(0))
	, MESSAGE(msg)
{
}


LifeTimer::~LifeTimer()
{
	std::cout << MESSAGE << difftime(time(0), CREATE_TIME) << std::endl;
}

