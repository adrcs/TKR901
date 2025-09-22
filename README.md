# TKR901
This code implements a programmer for the TKR901 repeater to tune it to amateur frequencies.

It is an improvement over the original code by Neal Reasoner, KB5ERY, SK. In two ways:
First, the programming data is precomputed and stored in table, which has five sections:
  1) All amateur frequencies from 927/902 to 928/903 in 12.5 KHz steps. There are no 'holes' in table.
  2) 100 commercial frequencies which match those in the original setup using the switches.
  3) A set of frequencies for the transmitter between the bottom end of the commercial band and the amateur band for tuning.
  4) A similar set of frequencies for the receiver for tuning.
  5) A set of debug tests that outputs a square wave on each lead to aid in debugging the wiring.
Second, an allowance has been made for the lock detect pin from the synthesizer to monitor success and reprogram if needed.
Third, the code has been written in a device-independent fashion, so as to make it easier to port to other platforms. As of this
release of the code, it works on both the Pi and PC. On the Pi it uses the 'real' pins, on the PC it dumps a message on the console
when it does I/O. This is useful for debugging. It has been compiled under both windows and linux.

The same code will be used on the PIC microcontroller, currently in development.

