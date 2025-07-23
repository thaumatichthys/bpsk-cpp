#include "DSSSDemodulator.hpp"
#include "constants.hpp"


DSSSDemodulator::DSSSDemodulator(
	float initial_freq,
	float max_deviation,
	float peak_finder_min_above_average,
	int prng_seed,
	int prng_seq_len,
	int oversample_ratio,
	int chip_coeff,
	int data_bitrate,
	int clock_recovery_oversample_ratio // just set this equal to oversample ratio. this is the number of clock recovery PLL updates in each data bit. must be integer fraction of (oversample_ratio * chip_rate). 
)
	:
	prng_((chip_coeff* data_bitrate* oversample_ratio), prng_seed, prng_seq_len, data_bitrate, chip_coeff),
	i_filter_(FilterMode::LowPass, 3, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	q_filter_(FilterMode::LowPass, 3, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	i_early_filter_(FilterMode::LowPass, 3, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	q_early_filter_(FilterMode::LowPass, 3, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	i_late_filter_(FilterMode::LowPass, 3, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	q_late_filter_(FilterMode::LowPass, 3, (float)(chip_coeff* data_bitrate* oversample_ratio), (float)(data_bitrate)),
	peak_finder_(2 * prng_seq_len, peak_finder_min_above_average), // since every cycle it advances by half a chip
	downconverter_nco_((float)(chip_coeff* data_bitrate* oversample_ratio)),
	costas_loop_filter_((float)(chip_coeff* data_bitrate* oversample_ratio), max_deviation, 0.0f, 0.0f), // dummy Ki and Kp for now
	clock_recovery_((float)(data_bitrate* clock_recovery_oversample_ratio), data_bitrate + 1, max_deviation / 4.0f) // the 4.0f is a magic number.
{

	// why is it like this ^^^
	carrier_sample_rate_ = chip_coeff * data_bitrate * oversample_ratio;
	samples_per_chip_ = carrier_sample_rate_ / (chip_coeff * data_bitrate);
	samples_per_seq_ = samples_per_chip_ * prng_seq_len;
	printf("carrier samplerate = %d\n", carrier_sample_rate_);
	printf("samples per chip = %d\n", samples_per_chip_);
	printf("samples_per_seq = %d\n", samples_per_seq_);
	initial_freq_ = initial_freq;
	clock_recovery_oversample_ratio_ = clock_recovery_oversample_ratio;
	oversample_ratio_ = oversample_ratio;
	chip_coeff_ = chip_coeff;

	prng_.AdvancePhaseSamples(1234);
	downconverter_nco_.SetFreq(initial_freq);
	costas_loop_filter_.SetKParams(max_deviation * 10.0f, 0.005f);
}

std::vector<float> DSSSDemodulator::Update(float sample) {
	float dummy1 = 0;
	// update everything
	//squaring_loop_.Update(sample);
	downconverter_nco_.Update();
	// end update everything
	
	// downconverter LO
	float LO_i = downconverter_nco_.GetCosValue();
	float LO_q = downconverter_nco_.GetSineValue();

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

	float demodulated = despread_q; // not too sure why its Q and not I, but whatever i guess

	dummy1 = despread_q;
	//float dummy2 = 0;// = despread_i;

	if (index_ % samples_per_seq_ == 0) { // run once per PRN sequence
		

		if (receiver_state_ == RX_STATE::RX_STATE_ACQ) {
			float i_integral = i_integrator_.DumpValue();
			float q_integral = q_integrator_.DumpValue();

			float correlation_energy = i_integral + q_integral;

			if (parameters::PRINT_STUFF) printf("Correlation Energy: %f\n", correlation_energy);

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

		if (index_ % (samples_per_seq_) == 0) {
			float i_integral = i_integrator_.DumpValue();
			float q_integral = q_integrator_.DumpValue();

			float correlation_energy = i_integral + q_integral;

			float i_early_integral = i_early_integrator_.DumpValue();
			float q_early_integral = q_early_integrator_.DumpValue();
			float i_late_integral = i_late_integrator_.DumpValue();
			float q_late_integral = q_late_integrator_.DumpValue();

			float correlation_energy_early = i_early_integral + q_early_integral;
			float correlation_energy_late = i_late_integral + q_late_integral;

			float alignment_error = correlation_energy_late - correlation_energy_early;

			float error_output = alignment_error * -0.05f;
			
			// limit error to two samples 
			const float error_max = 1.0f;
			if (abs(error_output > error_max))
				error_output = error_output * error_max / abs(error_output);
			prng_.AdvancePhaseSamples(roundf(error_output));
			
			if (parameters::PRINT_STUFF) printf("Alignment error: %f. Correlation: %f\n", alignment_error, correlation_energy);
		}

		if (index_ % (chip_coeff_ * oversample_ratio_ / clock_recovery_oversample_ratio_) == 0) {
			bool clock_pulse = clock_recovery_.Update(demodulated);
			dummy2 = clock_pulse;

			if (clock_pulse) {
				bool data = (bool)(demodulated > 0.0f);
				printf("DECODED: %d\n", data);
			}
		}
	}
	
	// costas loop
	float error = despread_i * despread_q;
	float control_signal = costas_loop_filter_.PushValue(error);
	downconverter_nco_.SetFreq(initial_freq_ + control_signal);
	// end costas loop
	
	prng_.IncrementPhase();
	// end
	index_++;
	return { (float)((bool)(dummy1>0.0f)), dummy2 };
}
