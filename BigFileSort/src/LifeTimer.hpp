#ifndef LIFETIMER_HPP_
#define LIFETIMER_HPP_
#include <string>


class LifeTimer
{
public:
	LifeTimer(const std::string msg);
	virtual ~LifeTimer();

private:
	const time_t CREATE_TIME;
	const std::string MESSAGE;
};

#endif /* LIFETIMER_HPP_ */
