# Chess-Robot
## Components
- Each chess piece needs a neodymium magnet attached to the base
- A grid of 64 reed swithes and diodes to localize chess pieces
- 2 stepper motors and 1 servo motor for the 2-axis robotic arm and its magnetic head
- Two Arduino boards are required due to limited GPIO pins  
  - Arduino Mega (sensor reading processing and main control)  
  - Arduino Uno (motor control)  
- 2 seven-segment displays and 1 LCD for the chess clock 
- Sjeng chess engine on an external PC to output AI moves and report invalid moves 
## Features
- Chess clock can respond to sensors and automatically switch side  
- Moves, captures, castling, and pawn promotions are smartly detected and displayed on the LCD 
- Invalid moves can be corrected by a human player  
- Difficulty level and game mode can be adjusted in the Chess Helper app (macOS)
## Supported game modes
- human vs human  
- human vs AI (human white)  
- AI vs human (AI white)  
- AI vs AI  
## Demo (AI vs AI)
[![thumbnail](https://img.youtube.com/vi/QaSgTOTe4k4/0.jpg)](https://www.youtube.com/watch?v=QaSgTOTe4k4 "Smart Chess Robot Demo")
