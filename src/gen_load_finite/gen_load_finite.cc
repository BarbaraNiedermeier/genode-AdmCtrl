/*
 * \brief  put some load on the machine by using finite tasks
 * \date   2017/10/11
 */
 
#include <limits>
#include <random>
#include <base/sleep.h>
#include <timer_session/connection.h>
#include <base/printf.h>

double compute_e()
{

	double e = 1.0;
	double kf;

	for (int k = 1; k < 32000000; k++) {

			kf *= (double) k;

		e += (1 / kf);

	}

	return e;

}

int main ()
{
	PINF("Hi I'm a finite test task!");
	int num_iterations = 100;
	
	static Timer::Connection _timer;

	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(1, std::numeric_limits<int>::max());

	for (int i=0; i < num_iterations; ++i) {

		double e = compute_e();
		int result = (int) e * 10000000000;
		PINF("e = %d", result);
		_timer.msleep(20000);

	}

	PINF("Bye from a finite test task!");
	return 0;

}
