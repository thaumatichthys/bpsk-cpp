#include "DataClockRecovery.hpp"

DataClockRecovery::DataClockRecovery(float sample_rate, float initial_baudrate, float max_dev_baud) : 
	pll_(sample_rate, initial_baudrate, max_dev_baud),
	input_filter_(FilterMode::BandPass, 2, sample_rate, initial_baudrate)
{
	// decrease PLL loop bandwidth
	pll_.SetLoopFilterKParams(1000000.0f, 100.0f);

}

void DataClockRecovery::Update(float input) {
	bool zerocross = (bool)(input > 0.0f);
	int pulses = abs(derivator_.PushValue(zerocross));
	float filtered = input_filter_.PushValue((float)pulses);

	pll_.Update(filtered);


}
