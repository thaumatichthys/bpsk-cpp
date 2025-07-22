#include <vector>


class PRNG {
public:
	PRNG(int sample_rate, int seed, int seq_length, int data_bitrate, int chip_coeff);
	bool GetSample();
	bool GetSampleAdvance45();
	bool GetSampleRetard45();
	void IncrementPhase();
	void AdvancePhaseNHalfPeriods(int n);
	void AdvancePhaseSamples(int samples);
private:
	int sample_rate_;
	//int seed_;
	int seq_len_;
	int data_bitrate_;
	int chip_coeff_;
	std::vector<bool> prn_sequence_;

	int phase_samples_ = 0; // phase with units of samples
	int samples_per_chip_; // carrier samples
	int samples_per_seq_;  // carrier samples
};