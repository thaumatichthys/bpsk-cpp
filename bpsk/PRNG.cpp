#include "PRNG.hpp"
#include <random>


PRNG::PRNG(int sample_rate, int seed, int seq_length, int data_bitrate, int chip_coeff) {
	sample_rate_ = sample_rate;
	//seed_ = seed;
	seq_len_ = seq_length;
	data_bitrate_ = data_bitrate;
	chip_coeff_ = chip_coeff;

	phase_samples_ = 0;
	samples_per_chip_ = sample_rate / (chip_coeff * data_bitrate);
	samples_per_seq_ = samples_per_chip_ * seq_length;
	
	// generate PRN sequence

	printf("samples per chip: %d\n", samples_per_chip_);
	
	std::vector<bool> dummy_seq = { 0,1,1,1,0,1,1,0,1,1,0,0,1,0,0,1,0,1,0,0,0,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,0,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,0,1,1,1,1,0,1,1,1,1,1,0,0,1,1,0,0,0,1,1,1,1,0,0,1,0,1,0,1,0,0,1,1,1,1,0,1,1,0,0,1,0,0,0,0,1,1,1,1,0,1,0,0,1,0,0,1,0,1,1,1,0,0,0,0,0,0,0,0,1,0,1,0,1,1,0,0,0,0,0,1,1,1,1,0,0,1,0,1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,0,0,1,1,0,0,0,1,0,0,1,0,1,1,1,1,0,1,0,1,1,1,0,1,1,0,0,1,1,1,1,1,1,1,0,0,0,1,0,0,1,0,1,1,1,0,1,0,0,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,0,0,1,0,1,0,1,0,0,1,1,0,0,1 };


	const int mst_bits = 32;
	std::mt19937 mt_engine(seed);

	int n_runs = seq_length / mst_bits + 1;
	for (int j = 0; j < n_runs; j++) {
		uint32_t prn = mt_engine();
		for (int i = 0; (i < mst_bits) && (i + j * mst_bits) < seq_length; i++) {
			bool bit = (bool)(((uint32_t)1 << i) & prn);
			//prn_sequence_.push_back(bit);
			prn_sequence_.push_back(dummy_seq[i + j * mst_bits]);
		}
	}
	//prn_sequence_.push_back(0);
}

bool PRNG::GetSample() {
	int rounded_index = phase_samples_ / samples_per_chip_;
	return prn_sequence_[rounded_index];
}

bool PRNG::GetSampleAdvance45() {
	int rounded_index = (phase_samples_ + samples_per_chip_ / 8) / samples_per_chip_; // 8 comes from 360 / 45 = 8
	int wrapped_index = (rounded_index + seq_len_) % seq_len_;
	return prn_sequence_[wrapped_index];
}

bool PRNG::GetSampleRetard45() {
	int rounded_index = (phase_samples_ - samples_per_chip_ / 8) / samples_per_chip_; // 8 comes from 360 / 45 = 8
	int wrapped_index = (rounded_index + seq_len_) % seq_len_;
	return prn_sequence_[wrapped_index];
}

void PRNG::IncrementPhase() {
	phase_samples_++;
	while (phase_samples_ < 0)
		phase_samples_ += samples_per_seq_;
	while (phase_samples_ >= samples_per_seq_)
		phase_samples_ -= samples_per_seq_;
}

void PRNG::AdvancePhaseNHalfPeriods(int n) {
	phase_samples_ += n * samples_per_chip_ / 2 - 1;
	IncrementPhase();
}

void PRNG::AdvancePhaseSamples(int samples) {
	phase_samples_ += samples - 1;
	IncrementPhase();
}
