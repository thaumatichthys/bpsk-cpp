#include "IIRFilter.hpp"
#include <math.h>
#include <complex>
#include <vector>
#include "constants.hpp"
#include <iostream>

IIRFilter::IIRFilter(FilterMode mode, int order, float sample_rate, float freq) {
	std::vector<std::vector<float>> coefficients;
	float nyquist = sample_rate / 2;
	float cutoff_frac = freq / nyquist;
	if (cutoff_frac > 1.0f) {
		// error state!
		while (1) printf("error: cutoff frequency cannot exceed nyquist\n");
	}
	if (mode == FilterMode::LowPass) {
		coefficients = IIRFilter::GetButterworthLowpassCoefficients(order, cutoff_frac);
	}
	else if (mode == FilterMode::HighPass) {
		// not implemented yet
		while (1) printf("error: i didnt code the high pass yet");
	}
	else {
		// invalid mode 
		while (1) printf("error: invalid filter mode");
	}
	// unpack coefficients
	numer_coeffs_ = coefficients[0];
	denom_coeffs_ = coefficients[1];
	
	n_coeffs_ = denom_coeffs_.size();
	// init buffers
	x_.resize(n_coeffs_, 0.0f);
	y_.resize(n_coeffs_, 0.0f);
}

IIRFilter::IIRFilter(FilterMode mode, int order, float sample_rate, float freq_low, float freq_high) {
	std::vector<std::vector<float>> coefficients;
	float nyquist = sample_rate / 2;
	float cutoff_frac_low = freq_low / nyquist;
	float cutoff_frac_high = freq_high / nyquist;

	if (cutoff_frac_low > 1.0f || cutoff_frac_high > 1.0f) {
		// error state!
		while (1) printf("error: cutoff frequency cannot exceed nyquist\n");
	}
	if (mode == FilterMode::BandPass) {
		coefficients = IIRFilter::GetButterworthBandpassCoefficients(order, cutoff_frac_low, cutoff_frac_high);
	}
	else {
		// invalid mode 
		while (1) printf("error: invalid filter mode");
	}
	// unpack coefficients
	numer_coeffs_ = coefficients[0];
	denom_coeffs_ = coefficients[1];

	n_coeffs_ = denom_coeffs_.size();
	// init buffers
	x_.resize(n_coeffs_, 0.0f);
	y_.resize(n_coeffs_, 0.0f);
}

float IIRFilter::PushValue(float new_val) {
	x_[0] = new_val;

	// dot product
	float sum = 0.0f;
	for (size_t i = 0; i < n_coeffs_; i++) {
		sum += x_[i] * numer_coeffs_[i];
	}
	for (size_t i = 1; i < n_coeffs_; i++) {
		sum -= y_[i] * denom_coeffs_[i];
	}
	
	y_[0] = sum;

	// shift values down
	for (size_t i = n_coeffs_ - 1; i > 0; i--) {
		y_[i] = y_[i - 1];
		x_[i] = x_[i - 1];
	}

	return sum;
}

std::vector<std::vector<float>> IIRFilter::GetButterworthBandpassCoefficients(int order, float low_frac, float high_frac) {
	float warped_low = 2.0f * tanf(constants::pi * low_frac / 2.0f);
	float warped_high = 2.0f * tanf(constants::pi * high_frac / 2.0f);
	float B = warped_high - warped_low;
	float w0 = sqrtf(warped_high * warped_low);  // center frequency

	std::vector<std::complex<float>> lp_poles;
	// compute butterworth poles
	for (int k = 0; k < order; k++) {
		float theta = constants::pi / 2.0f * (1.0f + (2.0f * k + 1.0f) / order);
		std::complex<float> pole = std::polar(1.0f, theta);
		lp_poles.push_back(pole);
	}
	// transform to band pass
	std::vector<std::complex<float>> bp_poles;
	for (auto& p : lp_poles) {
		// Correct quadratic: s^2 - B*p*s + w0^2 = 0
		std::complex<float> Bp = B * p;
		std::complex<float> disc = sqrt(Bp * Bp - std::complex<float>(4.0f * w0 * w0, 0.0f));
		// roots = [ Bp ± disc ] / 2
		std::complex<float> s1 = (Bp + disc) * 0.5f;
		std::complex<float> s2 = (Bp - disc) * 0.5f;
		bp_poles.push_back(s1);
		bp_poles.push_back(s2);
	}
	std::vector<std::complex<float>> digital_poles;
	// bilinear transform
	for (auto& pole : bp_poles) {
		std::complex<float> digital_pole = (1.0f + pole / 2.0f) / (1.0f - pole / 2.0f);
		digital_poles.push_back(digital_pole);
	}
	// polynomial expansion
	std::vector<std::complex<float>> a = { std::complex<float>(1.0f, 0.0f) }; // initialize to 1
	for (auto& pole : digital_poles) {
		auto a_copy = a;
		a_copy.push_back(std::complex<float>(0.0f, 0.0f));
		a.insert(a.begin(), std::complex<float>(0.0f, 0.0f));
		for (auto& element : a)
			element *= -pole;
		for (size_t i = 0; i < a.size(); i++) {
			a[i] += a_copy[i];
		}
	}

	// discard imaginary parts of a, if all poles are complex conjugates they should cancel out
	std::vector<float> a_real;
	a_real.reserve(a.size());
	for (const auto& element : a) {
		a_real.push_back(element.real());
	}
	// get numerator
	std::vector<float> b_unscaled(2 * order + 1, 0.0f);
	int coeff = 1;
	for (short k = 0; k <= order; k++) {
		coeff = (k == 0) ? 1 : coeff * (order - k + 1) / k;
		b_unscaled[2 * k] = coeff * powf(-1, k); // alternate sign
	}
	auto b_real = b_unscaled;
	// scale the numbers
	float wd0 = 2.0f * atanf(w0 / 2.0f);
	std::complex<float> num(0, 0), den(0, 0);
	for (int k = 0; k < (int)a_real.size(); ++k) {
		auto zexp = std::polar(1.0f, -wd0 * k);
		num += b_unscaled[k] * zexp;
		den += a_real[k] * zexp;
	}
	float mag = std::abs(num / den);
	float K = 1.0f / mag;
	for (auto& x : b_real) x *= K;

	return { b_real, a_real };
}

std::vector<std::vector<float>> IIRFilter::GetButterworthLowpassCoefficients(int order, float cutoff) {
	const float warped = 2.0f * tanf(constants::pi * cutoff / 2);
	std::vector<std::complex<float>> poles;
	// compute butterworth poles
	for (int k = 0; k < order; k++) {
		float theta = constants::pi / 2.0f * (1.0f + (2.0f * k + 1.0f) / order);
		std::complex<float> pole = warped * std::polar(1.0f, theta);
		poles.push_back(pole);
	}
	std::vector<std::complex<float>> digital_poles;
	// bilinear transform
	for (auto& pole : poles) {
		std::complex<float> digital_pole = (1.0f + pole / 2.0f) / (1.0f - pole / 2.0f);
		digital_poles.push_back(digital_pole);
	}
	// polynomial expansion
	std::vector<std::complex<float>> a = { std::complex<float>(1.0f, 0.0f) }; // initialize to 1
	for (auto& pole : digital_poles) {
		auto a_copy = a;
		a_copy.push_back(std::complex<float>(0.0f, 0.0f));
		a.insert(a.begin(), std::complex<float>(0.0f, 0.0f));
		for (auto& element : a)
			element *= -pole;
		for (size_t i = 0; i < a.size(); i++) {
			a[i] += a_copy[i];
		}
	}
	// discard imaginary parts of a, if all poles are complex conjugates they should cancel out
	std::vector<float> a_real;
	a_real.reserve(a.size());
	for (const auto& element : a) {
		a_real.push_back(element.real());
	}
	std::vector<float> b_real = { 1.0f };
	short coeff = 1;
	for (int k = 1; k < order + 1; k++) {
		coeff = (coeff * (order - k + 1)) / k;
		b_real.push_back(coeff);
	}
	// scale the numbers
	float numerator_unnorm_sum = 0;
	float denom_sum = 0;
	for (int i = 0; i < order + 1; i++) {
		numerator_unnorm_sum += b_real[i];
		denom_sum += a_real[i];
	}
	for (auto& element : b_real) {
		element *= denom_sum / numerator_unnorm_sum;
	}

	return { b_real, a_real }; // numerator, denominator
}

