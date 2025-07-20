#include "PLL.hpp"

PFD::PFD() {
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

	tuning_word_ = (uint32_t)((freq / sample_rate_) * 4294967296.0f);
}

void NCO::ChangeFreq(float increment) {
	// may be problematic
	int32_t tuning_word_increment = (int32_t)((increment / sample_rate_) * 4294967296.0f);
	tuning_word_ += tuning_word_increment;
}

bool NCO::Update() {
	output_ = phase_accum_ & 0x80000000; // value = MSB of phase accum
	phase_accum_ += tuning_word_;
	return output_;
}

bool NCO::GetOutput() {
	return output_;
}

LoopFilter::LoopFilter(float sample_rate) {
	sample_rate_ = sample_rate;
}

float LoopFilter::PushValue(float input) { // not implemented yet
	return input;
}

PLL::PLL(float sample_rate, float initial_freq, float max_deviation) : nco_(sample_rate), loop_filter_(sample_rate) {
	sample_rate_ = sample_rate;
	initial_freq_ = initial_freq;
	max_deviation_ = max_deviation;

	nco_.SetFreq(initial_freq);
}

bool PLL::Update(bool ref_in) {
	bool nco_val = nco_.GetOutput();
	float pfd_val = (float) pfd_.Update(ref_in, nco_val);
	float control_signal = loop_filter_.PushValue(pfd_val);

	nco_.SetFreq(control_signal);

	nco_.Update();
}

