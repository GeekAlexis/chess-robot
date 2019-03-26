# Smart-Chess-Robot
Two Arduino boards are required for this project due to limited GPIO pins  
Arduino Mega (sensor readings and UART communication)  
Arduino Uno (control motors)  
The Chess Helper app connects the commandline Sjeng Chess Engine to the Arduino through serial UART  
Sjeng provides computer moves and reports invalid moves made by a human player  
The output of Sjeng is logged and read back into the app  
## Some Features
Chess clock can respond to sensors and automatically switch side  
Mnimax search depth of Sjeng is constantly adjusted to react to the time limit  
Invalid moves can be corrected without the need to start over
## Supported game modes
human vs human  
human vs computer (human first)  
computer vs human (computer first)  
computer vs computer  
## Demo
[![thumbnail](https://img.youtube.com/vi/QaSgTOTe4k4/0.jpg)](https://www.youtube.com/watch?v=QaSgTOTe4k4 "Smart Chess Robot Demo")
