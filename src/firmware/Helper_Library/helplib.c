// helplib.c

#include "helplib.h"

// I define these two based on the fact that the maximum
// high resolution delay you can have for some boards/setups
// is 262.14 / (F_CPU / 1e6) so in otherwords, about 16.
// This led me to decide that we should stick with 10ms/us delay
// increments and thse #defines reflect this knowledge
#define MAX_MS_DELAY 1e-2
#define MAX_US_DELAY 1e-5

// I use these to check if the number of digits after the decimal
// should apply to a ms delay or us delay
#define MS_CHECK 1e2
#define US_CHECK 1e5

#define DELAY_INCREMENT 10


// Current implementation of a delay function in seconds
// Utilizes lower order delay functions in a somewhat optimized
// manner
void custom_delay_sec(double _s) {
	while (_s * MS_CHECK != (double) ((int)(_s * MS_CHECK))) {
		// if there is less time left than DELAY_INCREMENT ms, run for that much time, else DELAY_INCREMENT ms
		_delay_ms(DELAY_INCREMENT < _s * MS_CHECK ? DELAY_INCREMENT : _s * MS_CHECK);
		if (_s -= MAX_MS_DELAY <= 0) return;
	}
	while (_s * US_CHECK != (double) ((int)(_s * US_CHECK))) {
		// if there is less time left than DELAY_INCREMENT us, run for that much time, else DELAY_INCREMENT us
		_delay_us(DELAY_INCREMENT < _s * US_CHECK ? DELAY_INCREMENT : _s * US_CHECK);
		if (_s -= MAX_US_DELAY <= 0) return;
	}
}

// The following was an attempt at a really fancy implementation
// that may not be necessary
/*
// I try to optimize how well defined a delay can be based off
// what delay.h says about maximum delays. The maximum delay in
// one of the built in delay functions is based off whether
// _HAS_DELAY_CYCLES is defined. This is setup similarly
// to that of _delay_ms() and _delay_us() to reflect this.

void _delay_s(double _s) {

#if __HAS_DELAY_CYCLES && defined(__OPTIMIZE__) && \
!defined(__DELAY_BACKWARD_COMPATIBLE__) &&       \
__STDC_HOSTED__



#else



#endif

}
*/
