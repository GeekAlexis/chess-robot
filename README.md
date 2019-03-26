# Smart-Chess-Robot
Two Arduino boards are used for this project, one Arduino Mega and one Arduino Uno:
Arduino Mega (sensor readings and UART communication)
Arduino Uno (control motors)

The applescript app connects the commandline Sjeng Chess Engine to the Arduino through serial UART.
Sjeng provides computer moves and reports invalid moves made by a human player

4 game modes supported:
human vs human
human vs computer (human first)
computer vs human (computer first)
computer vs computer

