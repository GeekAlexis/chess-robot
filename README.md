# Smart-Chess-Robot
Two Arduino boards are required for this project due to limited GPIO pins  
Arduino Mega (sensor readings and UART communication)  
Arduino Uno (control motors)  
Motors, seven segments displays and LCDs are controlled using I2C
The Chess Helper app links the commandline Sjeng Chess Engine with Arduino Mega through serial UART  
Sjeng outputs computer moves and reports invalid moves made by a human player   
## Some features
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
