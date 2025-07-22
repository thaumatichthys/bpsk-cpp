#include "SquaringLoop.hpp"

SquaringLoop::SquaringLoop(float sample_rate, float initial_freq, float max_deviation, int input_filter_order) : 
		pll_(sample_rate, initial_freq, max_deviation), 
		iirfilter_(FilterMode::BandPass, input_filter_order, sample_rate, 2.0f * initial_freq - max_deviation, 2.0f * initial_freq + max_deviation) {
	
	if ((initial_freq + max_deviation) > sample_rate / 2) {
		// insufficient sample rate
		while (1) printf("squaring loop insufficient sample rate\n");
	}
	// set PLL ref div to 2
	pll_.SetRefDivider(2);

	// decrease PLL loop bandwidth
	pll_.SetLoopFilterKParams(1000000.0f, 100.0f);


}

float SquaringLoop::Update(float input) {
	float squared = input * input;

	float cleaned = iirfilter_.PushValue(squared);
	//printf("cleand %f\n", cleaned);
	//printf("input %f\n", input);
	bool zero_cross = (bool)(cleaned > 0.0f);

	bool output = pll_.Update(zero_cross);


	return output;
}
