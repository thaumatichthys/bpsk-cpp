## DSSS Demodulator
This is a Direct Sequence Spread Spectrum demodulator I have been working on.
It is meant to demodulate audio frequency DSSS signals, mostly tested with carrier center frequencies around 6-8 kHz, and around 50 Hz baud rate.
Currently, with a chipping factor of 150 chips per data bit, I am able to achieve ""mostly correct"" data at around -15 dB SNR. At -10 dB, it is quite reliable.

Please see main.py at https://github.com/thaumatichthys/bpsk-demodulator to generate the signal. The parameters are filled out in parameters.py. Currently this is very crude and messy and completely not presentable but I will hopefully fix that at some point.

### How it works
I will draw out a diagram at some point
