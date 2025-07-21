// bpsk.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <math.h>
#include <complex>
#include <vector>
#include <fstream>

#include "IIRFilter.hpp"
#include "PLL.hpp"

#define PI 3.14159265358979323846


void write_vector_to_file(const std::vector<float>& vec, const std::string& path) {
	std::ofstream file(path);
	if (!file) {
		std::cerr << "Failed to open file: " << path << "\n";
		return;
	}

	for (float val : vec) {
		file << val << "\n";
	}

	file.close();
}

int main()
{
	float sample_rate = 48000;
	float initial_freq = 1000;
	float max_dev = 100;
	PLL pll(sample_rate, initial_freq, max_dev);

	float actual_freq = 1090;

	NCO ref(sample_rate);
	ref.ChangeFreq(actual_freq);

	int num_cycles = sample_rate * 30;

	std::vector<float> pll_freqs;

	std::vector<float> pll_out;
	std::vector<float> ref_out;


	for (int i = 0; i < num_cycles; i++) {
		bool ref_val = ref.Update();
		//printf("%d\n", ref_val);

		bool pll_val = pll.Update(ref_val);


		float pll_freq = pll.GetNCOFreq();
		if (i % 100 == 0) {
			printf("NCO freq: %f\n", pll_freq);
		}

		pll_freqs.push_back(pll_freq);
		pll_out.push_back(pll_val);
		ref_out.push_back(ref_val);
	}


	std::string output_path = "C:/Users/thaumatichthys/PycharmProjects/bpsk/data.txt"; // or absolute path like "/tmp/data.txt"
	std::string output_path1 = "C:/Users/thaumatichthys/PycharmProjects/bpsk/data1.txt"; // or absolute path like "/tmp/data.txt"
	std::string output_path2 = "C:/Users/thaumatichthys/PycharmProjects/bpsk/data2.txt"; // or absolute path like "/tmp/data.txt"


	write_vector_to_file(pll_freqs, output_path);
	write_vector_to_file(pll_out, output_path1);
	write_vector_to_file(ref_out, output_path2);
	
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file


