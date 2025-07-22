#include "PLL.hpp"
#include <iostream>
#include <algorithm>
#include "constants.hpp"
#include <math.h>


PFD::PFD() {
	printf("pfd constructor ran\n");
}

int PFD::Update(bool reference, bool input) {
	// detect edges
	bool ref_edge = reference && !prev_ref_;
	bool inp_edge = input && !prev_inp_;

	if (ref_edge)
		up_ = 1;
	if (inp_edge)
		down_ = 1;

	if (up_ && down_) {
		up_ = 0;
		down_ = 0;
	}

	prev_inp_ = input;
	prev_ref_ = reference;

	return up_ - down_;
}

NCO::NCO(float sample_rate) {
	sample_rate_ = sample_rate;
}

void NCO::SetFreq(float freq) {
	// freq = sample_rate * tuning_word / 2^32
	// 2^32 * freq / sample_rate = tuning_word
	frequency_ = freq;
	UpdateFreq_();
}

void NCO::ChangeFreq(float increment) {
	// may be problematic
	frequency_ += increment;
	UpdateFreq_();
}

bool NCO::Update() {
	output_ = phase_accum_ & 0x80000000; // value = MSB of phase accum
	phase_accum_ += tuning_word_;
	return output_;
}

bool NCO::GetOutput() {
	return output_;
}

float NCO::GetFreq() {
	return frequency_;
}

float NCO::GetSineValue() {
	float phase = 2.0f * constants::pi * ((float)phase_accum_ / constants::uint32_t_max);
	return sinf(phase);
}

float NCO::GetCosValue() {
	float phase = 2.0f * constants::pi * ((float)phase_accum_ / constants::uint32_t_max);
	return cosf(phase);
}

void NCO::UpdateFreq_() {
	tuning_word_ = (uint32_t)((frequency_ / sample_rate_) * constants::uint32_t_max);
}

PIController::PIController(float sample_rate, float max_deviation, float Kp = 1000000.0f, float Ki = 100.0f) {
	max_deviation_ = max_deviation;
	sample_rate_ = sample_rate;
	printf("loop filter constructor ran\n");
	Ki_ = Ki;
	Kp_ = Kp;
	
}

float PIController::PushValue(float input) {
	float proportional_term_scaled = input * Kp_;
	float integral_term_scaled = integral_term_ * Ki_;

	float total = proportional_term_scaled + integral_term_scaled;

	if (integral_term_scaled < max_deviation_ && integral_term_scaled > -max_deviation_)
		integral_term_ += input;

	return total;
}

void PIController::SetKParams(float Kp, float Ki) {
	Ki_ = Ki;
	Kp_ = Kp;
}

FreqDivider::FreqDivider(int denom) {
	SetDenom(denom);
}

void FreqDivider::SetDenom(int denom) {
	counter_ = 0;
	counter_ovf_ = denom;
}

bool FreqDivider::PushValue(bool val) {
	if (counter_ovf_ == 0)
		return val;
	if (!prev_state && val) {
		counter_++;
	}
	if (counter_ >= counter_ovf_) {
		output_state_ = !output_state_;
		counter_ = 0;
	}
	prev_state = val;
	return output_state_;
}

PLL::PLL(float sample_rate, float initial_freq, float max_deviation) : nco_(sample_rate), loop_filter_(sample_rate, max_deviation), fb_divider_(1), ref_divider_(1) {
	sample_rate_ = sample_rate;
	initial_freq_ = initial_freq;

	nco_.SetFreq(initial_freq);
}

bool PLL::Update(bool ref_in) {
	bool ref_divided = ref_divider_.PushValue(ref_in);
	bool nco_val = nco_.GetOutput();
	bool nco_val_divided = fb_divider_.PushValue(nco_val);
	float pfd_val = (float) pfd_.Update(ref_divided, nco_val_divided) / sample_rate_;
	float control_signal = loop_filter_.PushValue(pfd_val);

	//nco_.SetFreq(control_signal + initial_freq_);

	nco_.Update();

	return nco_.GetOutput();
}

float PLL::GetNCOFreq() {
	return nco_.GetFreq();
}

void PLL::SetRefDivider(int denom) {
	ref_divider_.SetDenom(denom);
}

void PLL::SetFBDivider(int denom) {
	fb_divider_.SetDenom(denom);
}

void PLL::SetLoopFilterCenterFreq(float initial_freq) {
	initial_freq_ = initial_freq;
}

void PLL::SetLoopFilterKParams(float Kp, float Ki) {
	loop_filter_.SetKParams(Kp, Ki);
}

