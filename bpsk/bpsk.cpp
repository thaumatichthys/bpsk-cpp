// bpsk.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <math.h>
#include <complex>
#include <vector>

#define PI 3.14159265358979323846

void test(int order, float cutoff) {
    //std::complex<float> pole(1, 2);

    float T = 1.0f;// / samplerate;
    //float warped = 2 * samplerate * tan(PI * cutoff / samplerate);
    float warped = 2 * tan(PI * cutoff / 2);

    std::vector<std::complex<float>> poles;

    std::cout << "analog poles\n";
    // compute butterworth poles
    for (int k = 0; k < order; k++) {
        float theta = PI / 2.0 * (1 + (2.0 * k + 1.0) / order);
        std::complex<float> pole = warped * std::polar(1.0f, theta);

        poles.push_back(pole);
        std::cout << pole << std::endl;
    }
    std::cout << "bilinear transformed" << std::endl;
    std::vector<std::complex<float>> digital_poles;
    // do bilinear transform (s --> z)
    for (auto & pole : poles) {
        std::complex<float> digital_pole = (1.0f + pole * T / 2.0f) / (1.0f - pole * T / 2.0f);
        digital_poles.push_back(digital_pole);

        std::cout << digital_pole << std::endl;
    }

    
    //std::vector<std::complex<float>> poles2 = { 
    //    std::complex<float>(1.0f, 0.0f) ,
    //    std::complex<float>(0.2f, 0.4f) ,
    //    std::complex<float>(0.2f, -0.4f) ,


    //}; // initialize to 1
    std::cout << "polynomial expansion\n";
    // polynomial expansion
    std::vector<std::complex<float>> a = { std::complex<float>(1.0f, 0.0f) }; // initialize to 1

    for (auto& pole : digital_poles) {
        std::cout << std::endl;

        // multiply by (s - pole)
        // array indices are s powers
        // 1 --> 10000
        // s --> 01000
        //2s^3 ->00020

        // take copy of array
        auto a_copy = a;

        // expand a by 1 
        a_copy.push_back(std::complex<float>(0.0f, 0.0f));
        
        // s part: shift contents to right by 1 
        a.insert(a.begin(), std::complex<float>(0.0f, 0.0f));
        // multiply all elements of copy by -pole
        for (auto& element : a)
            element *= -pole;
        // add shifted and multiplied together
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
    int coeff = 1;
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
    


    std::cout << std::endl << std::endl;
    for (auto& element : a_real) {
        std::cout << element << ", ";
    }
    std::cout << std::endl << std::endl;
    for (auto& element : b_real) {
        std::cout << element << ", ";
    }
}

int main()
{
    std::cout << "Hello World!\n";

    test(2, 0.5);
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


