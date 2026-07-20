# TKR901
This code implements a programmer for the TKR901 repeater to tune it to amateur frequencies. It runs on a PIC microcontroller
and the component cost is less that $10. No Linux, no PI, just a dedicatd microcontroller that programs the TKR901 in
seconds, and constantly monitors the symthesizer lock status. It is programmed with a DIP swith,

It is an improvement over the original code by Neal Reasoner, KB5ERY, SK, and some of his code has been included.

The key improvement areas are:

First, the programming data is precomputed and stored in table, which has five sections:
  1) All amateur frequencies from 927/902 to 928/903 in 12.5 KHz steps. There are no 'holes' in table.
  2) 100 commercial frequencies which match those in the original setup using the switches.
  3) A set of frequencies for the transmitter between the bottom end of the commercial band and the amateur band for tuning.
  4) A similar set of frequencies for the receiver for tuning.
  5) A set of debug tests that outputs a square wave on each lead to aid in debugging the wiring.
     
Second, an allowance has been made for the lock detect pin from the synthesizer to monitor success and reprogram if needed when
a power black or brownout occurs.

73, de VE6VH


