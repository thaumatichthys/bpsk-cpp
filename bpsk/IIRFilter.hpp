#pragma once
#include <vector>


enum class FilterMode {
	LowPass = 0,
	BandPass = 1,
	HighPass = 2
};

class IIRFilter {
public:
	IIRFilter(FilterMode mode, int order, float sample_rate, float freq);
	IIRFilter(FilterMode mode, int order, float sample_rate, float freq_low, float freq_high);
	float PushValue(float new_val);
	std::vector<float> x_;
	std::vector<float> y_;
private:
	//int order_;
	size_t n_coeffs_;
	std::vector<float> denom_coeffs_;
	std::vector<float> numer_coeffs_;
	
	static std::vector<std::vector<float>> GetButterworthBandpassCoefficients(int order, float low_frac, float high_frac);
	static std::vector<std::vector<float>> GetButterworthLowpassCoefficients(int order, float cutoff);
};