#include "DataClockRecovery.hpp"

DataClockRecovery::DataClockRecovery(float sample_rate, float initial_baudrate, float max_dev_baud) : 
	input_filter_(FilterMode::BandPass, 3, sample_rate, initial_baudrate - max_dev_baud, initial_baudrate + max_dev_baud)
{
	// not much here
}

float DataClockRecovery::Update(float input) {
	bool zerocross = (bool)(input > 0.0f);
	int pulses = abs(derivator_.PushValue(zerocross));
	float filtered = input_filter_.PushValue((float)pulses);

	float phase_shifted = float_derivator_.PushValue(filtered);

	bool filtered_zerocross = (bool)(phase_shifted > 0.0f);

	int shifted_zerocross = out_derivator_.PushValue(filtered_zerocross);

	bool out = (bool)(shifted_zerocross > 0);

	return out;
}
