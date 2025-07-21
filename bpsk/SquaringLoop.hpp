#pragma once 
#include "PLL.hpp"
#include "IIRFilter.hpp"


class SquaringLoop {
public:
	SquaringLoop(float sample_rate, float initial_freq, float max_deviation, int input_filter_order = 4);
	PLL pll_;
	bool Update(float input);
private:
	IIRFilter iirfilter_;

};