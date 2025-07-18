#pragma once
#include <vector>


enum class FilterMode {
	LowPass,
	HighPass,
	BandPass
};

class IIRFilter {
	IIRFilter(FilterMode mode, int order, float freq);
	IIRFilter(FilterMode mode, int order, float freq_low, float freq_high);
	float PushValue(float new_val);
private:
	int order_;
	int n_coeffs_;
	std::vector<float> denom_coeffs_;
	std::vector<float> numer_coeffs_;
};