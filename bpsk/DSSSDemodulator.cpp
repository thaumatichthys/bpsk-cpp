#include "DSSSDemodulator.hpp"


DSSSDemodulator::DSSSDemodulator( 
	float initial_freq, 
	float max_deviation, 
	float peak_finder_min_above_average,
	int prng_seed,
	int prng_seq_len,
	int oversample_ratio, 
	int chip_coeff, 
	int data_bitrate) 
	: 
	squaring_loop_((float)(chip_coeff*data_bitrate*oversample_ratio), initial_freq, 100),
	// PRNG(int sample_rate, int seed, int seq_length, int data_bitrate, int chip_coeff)
	prng_((chip_coeff*data_bitrate*oversample_ratio), prng_seed, prng_seq_len, data_bitrate, chip_coeff),
	// IIRFilter(FilterMode mode, int order, float sample_rate, float freq)
	i_filter_(FilterMode::LowPass, 2, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	q_filter_(FilterMode::LowPass, 2, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	i_early_filter_(FilterMode::LowPass, 2, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	q_early_filter_(FilterMode::LowPass, 2, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	i_late_filter_(FilterMode::LowPass, 2, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	q_late_filter_(FilterMode::LowPass, 2, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	peak_finder_(prng_seq_len, peak_finder_min_above_average)
{

	// why is it like this ^^^
	carrier_sample_rate_ = chip_coeff * data_bitrate * oversample_ratio;
	//chip_cutoff_frac_ = (float)data_bitrate / ((float)(chip_coeff * data_bitrate * oversample_ratio)); 
	samples_per_chip_ = carrier_sample_rate_ / (chip_coeff * data_bitrate);
	samples_per_seq_ = samples_per_chip_ * prng_seq_len;
}

float DSSSDemodulator::Update(float sample) {
	float dummy1 = 0;
	// update everything
	squaring_loop_.Update(sample);
	// end update everything
	
	// downconverter LO
	float LO_i = squaring_loop_.pll_.nco_.GetCosValue();
	float LO_q = squaring_loop_.pll_.nco_.GetSineValue();

	float downconverted_i = sample * LO_i;
	float downconverted_q = sample * LO_q;

	float prn = (float) prng_.GetSample() * 2.0f - 1.0f;
	//prng_.IncrementPhase();
	

	float despread_i = i_filter_.PushValue(downconverted_i * prn);
	float despread_q = q_filter_.PushValue(downconverted_q * prn);
	//printf("freq %f\n", squaring_loop_.pll_.GetNCOFreq());

	// compute correlation energy 
	i_integrator_.Accumulate(despread_i * despread_i);
	q_integrator_.Accumulate(despread_q * despread_q);

	dummy1 = despread_i;


	if (index_ % samples_per_seq_ == 0) { // run once per PRN sequence
		float i_integral = i_integrator_.DumpValue();
		float q_integral = q_integrator_.DumpValue();

		float correlation_energy = i_integral + q_integral;

		if (receiver_state_ == RX_STATE::RX_STATE_ACQ) {
			printf("Correlation Energy: %f\n", correlation_energy);

			if (peak_finder_.PushValue(correlation_energy)) {
				// alignment found, now scroll back to it
				int delta_halfperiods = peak_finder_.GetFoundIndex();
				prng_.AdvancePhaseNHalfPeriods(delta_halfperiods);
				receiver_state_ = RX_STATE::RX_STATE_TRACK;
			}
			else {
				prng_.AdvancePhaseNHalfPeriods(1);
			}
		}
	}
	if (receiver_state_ == RX_STATE::RX_STATE_TRACK) {
		float prn_early = (float)prng_.GetSampleAdvance45() * 2.0f - 1.0f;
		float prn_late = (float)prng_.GetSampleRetard45() * 2.0f - 1.0f;

		float i_early_despread = i_early_filter_.PushValue(downconverted_i * prn_early);
		float q_early_despread = q_early_filter_.PushValue(downconverted_q * prn_early);
		float i_late_despread = i_late_filter_.PushValue(downconverted_i * prn_late);
		float q_late_despread = q_late_filter_.PushValue(downconverted_q * prn_late);

		i_early_integrator_.Accumulate(i_early_despread * i_early_despread);
		q_early_integrator_.Accumulate(q_early_despread * q_early_despread);
		i_late_integrator_.Accumulate(i_late_despread * i_late_despread);
		q_late_integrator_.Accumulate(q_late_despread * q_late_despread);

		if (index_ % samples_per_seq_ == 0) {
			float i_early_integral = i_early_integrator_.DumpValue();
			float q_early_integral = q_early_integrator_.DumpValue();
			float i_late_integral = i_late_integrator_.DumpValue();
			float q_late_integral = q_late_integrator_.DumpValue();

			float correlation_energy_early = i_early_integral + q_early_integral;
			float correlation_energy_late = i_late_integral + q_late_integral;

			float alignment_error = correlation_energy_late - correlation_energy_early;

			float error_output = alignment_error * -10.0f;
			
			// limit error to two samples 
			const float error_max = 1.0f;
			if (abs(error_output > error_max))
				error_output = error_output * error_max / abs(error_output);
			prng_.AdvancePhaseSamples(roundf(error_output));
			
			printf("Alignment error: %f\n", alignment_error);
		}

		bool data_bit = (bool)(despread_i > 0.0f);
	}
	

	
	prng_.IncrementPhase();
	// end
	index_++;
	return dummy1;
}
