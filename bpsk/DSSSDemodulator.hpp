#include "IIRFilter.hpp"
#include "SquaringLoop.hpp"
#include "PRNG.hpp"
#include "Misc.hpp"
#include "PeakFinder.hpp"


enum class RX_STATE {
	RX_STATE_ACQ = 0,
	RX_STATE_TRACK = 1
};

class DSSSDemodulator {
public:
	DSSSDemodulator(
		float initial_freq,
		float max_deviation,
		float peak_finder_min_above_average,
		int prng_seed,
		int prng_seq_len,
		int oversample_ratio,
		int chip_coeff,
		int data_bitrate);

	std::vector<float> Update(float sample);
private:
	RX_STATE receiver_state_ = RX_STATE::RX_STATE_ACQ;
	SquaringLoop squaring_loop_;
	PRNG prng_;
	IIRFilter i_filter_;
	IIRFilter q_filter_;
	IIRFilter i_early_filter_;
	IIRFilter q_early_filter_;
	IIRFilter i_late_filter_;
	IIRFilter q_late_filter_;
	Integrator i_integrator_;
	Integrator q_integrator_;
	PeakFinder peak_finder_;
	Integrator i_early_integrator_;
	Integrator q_early_integrator_;
	Integrator i_late_integrator_;
	Integrator q_late_integrator_;


	int samples_per_chip_;
	int samples_per_seq_;
	//float chip_cutoff_frac_;
	int carrier_sample_rate_;
	uint32_t index_ = 0;
};