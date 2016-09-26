/*
 * \brief  put some load on the machine
 * \author Paul Nieleck
 * \date   2016/09/23
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

	static Timer::Connection _timer;

	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(1, std::numeric_limits<int>::max());

	while (true) {

		double e = compute_e();
		PINF("e = %d", (int) e * 10000000000);
		_timer.msleep(20000);

	}

	return 0;

}