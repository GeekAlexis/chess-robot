//Chess playing robot master arduino
//Control of reed switches, two seven segs, and a LCD

//Functionality: takes readings of a move from the chess board, 
//plugs it into the sjeng chess engine thru applescript, and 
//sends the returned move to the slave arduino

//Developed by Alexis Yang and Ning Du

#include <Adafruit_LEDBackpack.h>
#include <LiquidCrystal.h>
#include "chess_recorder.h"

Adafruit_7segment matrix = Adafruit_7segment();
LiquidCrystal lcd(37, 36, 35, 34, 33, 32);
            
void setup() { 
  lcd.begin(16, 2);
  Serial.begin(9600);
  Serial1.begin(9600);
  for(int i = 2; i <= 9; i++) {    //pin #2-9 for digital output
    pinMode(i,OUTPUT);
    digitalWrite(i,LOW);
  }
  for(int j = 22; j <= 29; j++) {  //pin #22-29 for digital input
    pinMode(j,OUTPUT);
    digitalWrite(j,HIGH);
  }
}

void loop() {
  unsigned long millisPassed;
  float timeLimit_mins;
  long timeLimit_millis;
  long white_Time_Left, black_Time_Left;
  long white_Time_Passed = 0, black_Time_Passed = 0;
  long timePaused = 0;
  int motorComplete = 0; 
  bool Err1 = false;
  int gameMode;     //1: human vs human 2: human vs computer 3: computer vs human 4: computer vs computer
  int side = 1;
  
  Serial.println("S");              //start
  while(Serial.available() == 0);   
  gameMode = Serial.parseInt(); 

  initiate();
  Serial.println("T");           //time 
  lcd.setCursor(0,0);
  lcd.println("Setting the time");
  lcd.setCursor(0,1);
  lcd.println("limit...        ");
  while(Serial.available() == 0);
  timeLimit_mins = Serial.parseFloat();
  timeLimit_millis = timeLimit_mins*60000;
  lcd.clear();
  
  white_Time_Left = timeLimit_millis;
  matrix.begin(0x70);
  display_Clock(white_Time_Left);    
  black_Time_Left = timeLimit_millis;
  matrix.begin(0x71);
  display_Clock(black_Time_Left);
  delay(1000);
  millisPassed = millis();
  
  while (white_Time_Left > 0 && black_Time_Left > 0 && !whiteMates && !blackMates && !draw) {
    while (side == 1 && white_Time_Left > 0 && black_Time_Left > 0 && !blackMates) {
      matrix.begin(0x70);
      white_Time_Left = timeLimit_millis - (millis() - black_Time_Passed - millisPassed - timePaused);
      white_Time_Passed  = timeLimit_millis - white_Time_Left;
      display_Clock(white_Time_Left);

      if(gameMode == 1 || gameMode == 2) 
        takeHumanMoves(side);
      else
        takeComMoves(side, motorComplete, Err1, timePaused);
    }   
    while(side == 2 && white_Time_Left > 0 && black_Time_Left > 0 && !whiteMates){
      matrix.begin(0x71);
      black_Time_Left = timeLimit_millis - (millis() - white_Time_Passed - millisPassed - timePaused);
      black_Time_Passed  = timeLimit_millis - black_Time_Left;
      display_Clock(black_Time_Left);

      if(gameMode == 1 || gameMode == 3) 
        takeHumanMoves(side);
      else
        takeComMoves(side, motorComplete, Err1, timePaused);
    }
  }
  lcd.setCursor(0,0);
  lcd.print("----GAMEOVER----");
  lcd.setCursor(0,1);
  if(draw)
    lcd.print("      DRAW      ");     
  if(black_Time_Left <= 0 || whiteMates)
    lcd.print("   WHITE WINS   ");
  if(white_Time_Left <= 0 || blackMates) 
    lcd.print("   BLACK WINS   ");
  Serial1.write(HOMING,8);
}

void initiate() {
  matrix.begin(0x70);
  matrix.print(10000,DEC);
  matrix.writeDisplay();
  matrix.begin(0x71);
  matrix.print(10000,DEC);
  matrix.writeDisplay();
  lcd.clear();
  empty_Edges();
  Serial1.read();
  Serial1.read();
  bool isready = true;
  board_Scan();
  for(int row = 0; row < 2; row++) {
    for(int col = 0; col < 8; col++) {
      if(board_State[row][col] == 0) {
        isready = false;
      }
    }
  }
  for(int row = 6; row < 8; row++) {
    for(int col = 0; col < 8; col++) {
      if(board_State[row][col] == 0) {
        isready = false;
      }
    }
  }
  if(!isready) {
      Serial.println("I");    //incorrect
      lcd.setCursor(0,0);     
      lcd.print("Incorrect chess ");
      lcd.setCursor(0,1);
      lcd.print("position        ");
  }
  while(!isready) {
    isready = true;
    board_Scan();
    for(int row = 0; row < 2; row++) {
      for(int col = 0; col < 8; col++) {
        if(board_State[row][col] == 0) {
          isready = false;
        }
      }
    }
    for(int row = 6; row < 8; row++) {
      for(int col = 0; col < 8; col++) {
        if(board_State[row][col] == 0) {
          isready = false;
        }
      }
    }
  }
  Serial.println("R");         //ready
  lcd.clear();
  lcd.setCursor(0,0);     
  lcd.print("Initializing...");
  last_Board_State[8][8] = {{0}};
  for(int row = 0; row < 2; row++) {
    for(int col = 0; col < 8; col++) {
        last_Board_State[row][col] = 1;
    }
  }
  for(int row = 6; row < 8; row++) {
    for(int col = 0; col < 8; col++) {
        last_Board_State[row][col] = 1;  
    }
  }
  for(int ind = 0; ind <= 7; ind++) {
    Black[ind].x = 0;
    Black[ind].y = ind;
    White[ind].x = 6;
    White[ind].y = ind;
    piecesW[ind] = 'P';
  }
  for(int ind = 8; ind <= 15; ind++) {
    Black[ind].x = 1;
    Black[ind].y = ind - 8;
    White[ind].x = 7;
    White[ind].y = ind - 8;
    piecesB[ind] = 'p';
  }  
  islegal = false;
  whiteMates = false;
  blackMates = false;
  draw = false;
  inputString = "";     
  stringComplete = false;
  moveSent = false;
  castleState = 0;
  delay(2000);
}

void display_Clock(long count) {
  if((count % 6000000) / 600000 != 0 || (count / 6000000) > 0)
    matrix.writeDigitNum(0, (count % 6000000) / 600000);
  else
    matrix.writeDigitRaw(0, 0b00000000);
  matrix.writeDigitNum(1, (count % 600000) / 60000);
  matrix.drawColon(true);
  matrix.writeDigitNum(3, (count % 60000) / 10000);
  matrix.writeDigitNum(4, (count % 10000) / 1000);
  matrix.writeDisplay();
}

void display_LCD(bool isPromoted, int side) {
  lcd.clear();
  generate_Piece_Image(0,1,piece_Moved);
  lcd.print(" ");
  lcd.print(RANKS.charAt(lowering_Edge[0].y));
  lcd.print(FILES.charAt(lowering_Edge[0].x));
  if(piece_Captured == '\0') {
    lcd.print(" to ");
    lcd.print(RANKS.charAt(rising_Edge.y));
    lcd.print(FILES.charAt(rising_Edge.x));  
    if(castleState == 3) {
      if(lowering_Edge[0].y == 0) {
        lcd.setCursor(0,1);
        lcd.print("Castle queenside");
      }
      if(lowering_Edge[0].y == 7) {
        lcd.setCursor(0,1);
        lcd.print("Castle kingside ");
      }
      castleState = 0;
    }
  }  
  else {
    lcd.print(" took ");
    generate_Piece_Image(10,1,piece_Captured);
    lcd.print(" ");
    lcd.print(RANKS.charAt(lowering_Edge[1].y));
    lcd.print(FILES.charAt(lowering_Edge[1].x)); 
  } 
  if(isPromoted) {
    generate_Piece_Image(0,1,piece_Moved);
    lcd.print(" promoted to ");
    if(side == 1) 
      generate_Piece_Image(14,1,'Q');
    else
      generate_Piece_Image(14,1,'q');
  }
}

void generate_Piece_Image(int x, int y, char piece) {
  int ind = 0;
  switch(piece) {
        case 'p':
            lcd.createChar(1, pawnB);
            ind=1;
        break;
        
        case 'P':
            lcd.createChar(2, pawnW);
            ind=2;
        break;
        
        case 'r':
            lcd.createChar(7, rookB);
            ind=7;
        break;
        
        case 'R':
            lcd.createChar(7, rookW);
            ind=7;
        break;
        
        case 'k':
            lcd.createChar(4, kingB);
            ind=4;
        break;
        
        case 'K':
            lcd.createChar(4, kingW);
            ind=4;
        break;
        
        case 'b':
            lcd.createChar(5, bishopB);
            ind=5;
        break;
        
        case 'B':
            lcd.createChar(5, bishopW);
            ind=5;
        break;
        
        case 'q':
            lcd.createChar(3, queenB);
            ind=3;
        break;
        
        case 'Q':
            lcd.createChar(3, queenW);
            ind=3;
        break;
        
        case 'n':
            lcd.createChar(6, knightB);
            ind=6;
        break;
        
        case 'N':
            lcd.createChar(6, knightW);
            ind=6;
        break;
    }
    lcd.setCursor(x, y);
    lcd.write(ind);
}

void board_Scan() {
  for(int i = 2, row = 0; i <= 9 && row <= 7; i++, row++) {
    digitalWrite(i,HIGH);
    for(int j = 22, col = 0; j <= 29 && col <= 7; j++, col++) {       //scanning without pull-down resistors
      pinMode(j,OUTPUT);
      digitalWrite(j,LOW);
      pinMode(j,INPUT);
      board_State[row][col] = digitalRead(j);
      pinMode(j,OUTPUT);
      digitalWrite(j,HIGH);
    }
    digitalWrite(i,LOW);
  }
}

void board_Analysis(bool isComputer) {
  for(int row = 0; row < 8; row++) {
    for(int col = 0; col < 8; col++) {
      int diff = board_State[row][col] - last_Board_State[row][col];
      if(diff == 1 && rising_Edge.x == -6) { 
        if(isComputer || lowering_Edge[0].x != -6) {
          rising_Edge.x = row;
          rising_Edge.y = col;
        }
      }
      if(diff == -1 && lowering_Edge[0].x == -6) {
        for(int ind = 0; ind < 16; ind++) {
          if(White[ind].x == row) {
            if(White[ind].y == col) {
              lowering_Edge[0].x = row;
              lowering_Edge[0].y = col;
            }
          }
          if(Black[ind].x == row) {
            if(Black[ind].y == col) {
              lowering_Edge[0].x = row;
              lowering_Edge[0].y = col;
            }
          }
        }
      }
      else if(diff == -1) {
        for(int ind = 0; ind < 16; ind++) {
          if(White[ind].x == row) {
            if(White[ind].y == col) {
              lowering_Edge[1] = lowering_Edge[0];
              lowering_Edge[0].x = row;
              lowering_Edge[0].y = col;
            }
          }
          if(Black[ind].x == row) {
            if(Black[ind].y == col) {
              lowering_Edge[1] = lowering_Edge[0];
              lowering_Edge[0].x = row;
              lowering_Edge[0].y = col;
            }
          }
        }
      }   
    }
  }
  for(int row = 0; row < 8; row++) {
    for(int col = 0; col < 8; col++) {
      last_Board_State[row][col] = board_State[row][col];
    }
  }
}

void empty_Edges() {
  rising_Edge = EMPTY;
  lowering_Edge[0] = EMPTY;
  lowering_Edge[1] = EMPTY;
}

void chess_Record(int side) { 
  if(side == 1) {
    if(capture_Detect(side) == true) {
      for(int ind = 0; ind < 16; ind++) {
        if(Black[ind].x == lowering_Edge[1].x && Black[ind].y == lowering_Edge[1].y) {
          Black[ind].x = -1;
          Black[ind].y = -1;
        }
      }
    }
    for(int ind = 0; ind < 16; ind++) {
      if(White[ind].x == lowering_Edge[0].x && White[ind].y == lowering_Edge[0].y) {
        White[ind] = rising_Edge;
        if(promotion_Detect(side) == true)
          piecesW[ind] = 'Q';
      }
    }
  }
  else {
    if(capture_Detect(side) == true) {
      for(int ind = 0; ind < 16; ind++) {
        if(White[ind].x == lowering_Edge[1].x && White[ind].y == lowering_Edge[1].y) {
          White[ind].x = -1;
          White[ind].y = -1;
        }
      }
    }
    for(int ind = 0; ind < 16; ind++) {
      if(Black[ind].x == lowering_Edge[0].x && Black[ind].y == lowering_Edge[0].y) {
        Black[ind] = rising_Edge;
        if(promotion_Detect(side) == true)
          piecesB[ind] = 'q';
      }
    }
  }
}

bool capture_Detect(int side) {
  if(lowering_Edge[0].x != -6 && lowering_Edge[1].x != -6 && rising_Edge.x != -6) {
    if(side == 1) {
      for(int i = 0; i < 16; i++) {
        if(White[i].x == lowering_Edge[0].x && White[i].y == lowering_Edge[0].y) {
          for(int j = 0; j < 16; j++) {
            if(Black[j].x == lowering_Edge[1].x && Black[j].y == lowering_Edge[1].y) {
              piece_Moved = piecesW[i];
              piece_Captured = piecesB[j];
              return true;
            }
          }
        }
      }
    }
    else {
      for(int i = 0; i < 16; i++) {
        if(White[i].x == lowering_Edge[1].x && White[i].y == lowering_Edge[1].y) {
          for(int j = 0; j < 16; j++) {
            if(Black[j].x == lowering_Edge[0].x && Black[j].y == lowering_Edge[0].y) {
              piece_Moved = piecesB[j];
              piece_Captured = piecesW[i];
              return true;
            }
          }
        }
      }
    }
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("  Invalid Move  ");
    empty_Edges();
  }
  return false;
}

bool move_Detect(int side) {
  if(lowering_Edge[0].x != -6 && lowering_Edge[1].x == -6 && rising_Edge.x != -6) {
    if(lowering_Edge[0].x == rising_Edge.x && lowering_Edge[0].y == rising_Edge.y) {
      empty_Edges();
      return false;
    }
    if(side == 1) {
      for(int ind = 0; ind < 16; ind++) {
        if(Black[ind].x == rising_Edge.x && Black[ind].y == rising_Edge.y) {
          empty_Edges();
          return false;
        }
      }
      for(int ind = 0; ind < 16; ind++) {
        if(White[ind].x == lowering_Edge[0].x && White[ind].y == lowering_Edge[0].y) {
          piece_Moved = piecesW[ind];
          piece_Captured = '\0';
          return true;
        }
      }
    }
    else {
      for(int ind = 0; ind < 16; ind++) {
        if(White[ind].x == rising_Edge.x && White[ind].y == rising_Edge.y) {
          empty_Edges();
          return false;
        }
      }
      for(int ind = 0; ind < 16; ind++) {
        if(Black[ind].x == lowering_Edge[0].x && Black[ind].y == lowering_Edge[0].y) {
          piece_Moved = piecesB[ind];
          piece_Captured = '\0';
          return true;
        }
      }
    }
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("  Invalid Move  ");
    empty_Edges();
  }
  return false;
}
   
bool promotion_Detect(int side) {
  if(side == 1) {
    if(piece_Moved == 'P' && rising_Edge.x == 0) 
      return true;
  }
  else {
    if(piece_Moved == 'p' && rising_Edge.x == 7) 
      return true;
  }
  return false;
}

//process serial input from sjeng
void processString() {
    comMove = "";
    if(inputString == "Whit")
      whiteMates = true;
    else if(inputString == "Blac")
      blackMates = true;
    else if(inputString == "Draw") 
      draw = true;
    else if(inputString == "Ille")
      islegal = false;
    else if(inputString == "Lega") 
      islegal = true;
    else if(inputString.length() > 0)
      comMove = inputString;
    inputString = "";
    stringComplete = false;
}

//for human moves
void castling_Detect(int side) {                    
  if((piece_Moved == 'K' && lowering_Edge[0].x == 7 || piece_Moved == 'k' && lowering_Edge[0].x == 0) && lowering_Edge[0].y == 4) {
    if(side == 1 && rising_Edge.x == 7 && rising_Edge.y == 6 || side == 2 && rising_Edge.x == 0 && rising_Edge.y == 6) {      //kingside  
      if(!moveSent) {
        if(side == 1)
          Serial.println("e1g1");
        else
          Serial.println("e8g8");
        moveSent = true;
      }
      if(stringComplete) {
        moveSent = false;
        processString();
        if(islegal) {
          castleState = 2;
          chess_Record(side);
          lcd.clear();
          empty_Edges();
        }
        else {
          lcd.clear();
          lcd.setCursor(0,1);
          lcd.print("  Invalid Move  ");
          rising_Edge = EMPTY;
        }
      }
    }
    if(side == 1 && rising_Edge.x == 7 && rising_Edge.y == 2 || side == 2 && rising_Edge.x == 0 && rising_Edge.y == 2) {  //queenside
      if(!moveSent) {
        if(side == 1) 
          Serial.println("e1c1");
        else
          Serial.println("e8c8");
        moveSent = true;
      }
      if(stringComplete) {
        moveSent = false;
        processString();
        if(islegal) {
          castleState = 1;
          chess_Record(side);
          lcd.clear();
          empty_Edges();
        }
        else {
          lcd.clear();
          lcd.setCursor(0,1);
          lcd.print("  Invalid Move  ");
          rising_Edge = EMPTY;
        }
      }
    }
  }
  else if(castleState == 2) {
    if(side == 1 && lowering_Edge[0].x == 7 && lowering_Edge[0].y == 7 || side == 2 && lowering_Edge[0].x == 0 && lowering_Edge[0].y == 7) {
      if(side == 1 && rising_Edge.x == 7 && rising_Edge.y == 5 || side == 2 && rising_Edge.x == 0 && rising_Edge.y == 5) {
        castleState = 3;
      }
      else {
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("Invalid Castling");
        rising_Edge = EMPTY;
      }
    }
    else {
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Invalid Castling");
      empty_Edges();
    }
  }
  else if(castleState == 1) {
    if(side == 1 && lowering_Edge[0].x == 7 && lowering_Edge[0].y == 0 || side == 2 && lowering_Edge[0].x == 0 && lowering_Edge[0].y == 0) {
      if(side == 1 && rising_Edge.x == 7 && rising_Edge.y == 3 || side == 2 && rising_Edge.x == 0 && rising_Edge.y == 3) {
        castleState = 3;
      }
      else {
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("Invalid Castling");
        rising_Edge = EMPTY;
      }
    }
    else {
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Invalid Castling");
      empty_Edges();
    }
  }
}

//for computer moves
void decodeMove(String m, int side) {                 
  int oldx = FILES.indexOf(m.charAt(1));
  int oldy = RANKS.indexOf(m.charAt(0));
  int newx = FILES.indexOf(m.charAt(3));
  int newy = RANKS.indexOf(m.charAt(2));
  piece_Captured = '\0';
  for(int ind = 0; ind < 8; ind++) {
    dataPacket[ind] = 255;
  }
  for(int ind = 0; ind < 16; ind++) {
    if(side == 1) {
      if(Black[ind].x == newx) {
        if(Black[ind].y == newy) {
          dataPacket[4] = newx;
          dataPacket[5] = newy;
          piece_Captured = piecesB[ind];
        }
      }
    }
    else {
      if(White[ind].x == newx) {
        if(White[ind].y == newy) {
          dataPacket[4] = newx;
          dataPacket[5] = newy;
          piece_Captured = piecesW[ind];
        }
      }
    }
  }
  for(int ind = 0; ind < 16; ind++) {
    if(side == 1) {
      if(White[ind].x == oldx) {
        if(White[ind].y == oldy) {
          dataPacket[0] = oldx;
          dataPacket[1] = oldy;
          dataPacket[2] = newx;
          dataPacket[3] = newy;
          piece_Moved = piecesW[ind];
        }
      }
    }
    else {
      if(Black[ind].x == oldx) {
        if(Black[ind].y == oldy) {
          dataPacket[0] = oldx;
          dataPacket[1] = oldy;
          dataPacket[2] = newx;
          dataPacket[3] = newy;
          piece_Moved = piecesB[ind];
        }
      }
    }
  }
  if(piece_Moved == 'p' || piece_Moved == 'P') {  //En passant
    int dx = newx - oldx;
    int dy = newy - oldy;
    if(abs(dx) == 1 && abs(dy) == 1 && piece_Captured == '\0') {
      dataPacket[4] = oldx;
      dataPacket[5] = newy;
      if(side == 1) 
        piece_Captured = 'p';
      else
        piece_Captured = 'P';
    }
  }
  if(piece_Moved == 'k' || piece_Moved == 'K') {
    int dx = newx - oldx;
    int dy = newy - oldy;
    if(dx == 0 && abs(dy) == 2) { 
      dataPacket[6] = dataPacket[2];
      dataPacket[7] = dataPacket[3];
      if(side == 1) {
        dataPacket[0] = 7;
        dataPacket[2] = 7;
        if(dy > 0) {          //castle kingside
          dataPacket[1] = 7;
          dataPacket[3] = 5;
        } else {              //castle queenside
          dataPacket[1] = 0;
          dataPacket[3] = 3;
        }
      }
      if(side == 2) {
        dataPacket[0] = 0;
        dataPacket[2] = 0;
        if(dy > 0) {          //castle kingside
          dataPacket[1] = 7;
          dataPacket[3] = 5;
        } else {              //castle queenside
          dataPacket[1] = 0;
          dataPacket[3] = 3;
        }
      }
    }
  }
}

bool positionCheck(int numMove, bool isCom, int side) {
   board_Scan();
   board_Analysis(isCom);
   if(numMove == 1) {
     if(dataPacket[4] != 255) {
       if(lowering_Edge[0].x == dataPacket[4] && lowering_Edge[0].y == dataPacket[5] && lowering_Edge[1].x == -6 && rising_Edge.x == -6) 
        return true;
       else 
        empty_Edges();
     }
     if(dataPacket[6] != 255) {
       if(lowering_Edge[1].x != -6) 
         empty_Edges();
       if(move_Detect(side) == true) {
         if(side == 1) {
           if(lowering_Edge[0].x == 7 && lowering_Edge[0].y == 4 && rising_Edge.x == dataPacket[6] && rising_Edge.y == dataPacket[7]) {
             chess_Record(side);
             lcd.clear();
             empty_Edges();
             return true;
           }
           else {
             empty_Edges();
           }
         }
         if(side == 2) {
           if(lowering_Edge[0].x == 0 && lowering_Edge[0].y == 4 && rising_Edge.x == dataPacket[6] && rising_Edge.y == dataPacket[7]) {
             chess_Record(side);
             lcd.clear();
             empty_Edges();
             return true;
           }
           else {
             empty_Edges();
           }
         }
       }
     }
   }
   if(numMove == 2) {
     if(dataPacket[4] != 255) {
       if(move_Detect(side) == true)  
        empty_Edges();
       if(capture_Detect(side) == true) {   
         if(lowering_Edge[0].x == dataPacket[0] && lowering_Edge[0].y == dataPacket[1] && rising_Edge.x == dataPacket[2] && rising_Edge.y == dataPacket[3]) {
           return true;
         }
         else {
           lowering_Edge[0] = EMPTY;
           rising_Edge = EMPTY;
         }
       }
     }
     else if(dataPacket[6] != 255) {
       if(lowering_Edge[1].x != -6)
         empty_Edges();
       if(move_Detect(side) == true) {
         if(lowering_Edge[0].x == dataPacket[0] && lowering_Edge[0].y == dataPacket[1] && rising_Edge.x == dataPacket[2] && rising_Edge.y == dataPacket[3]) {
           castleState = 3;
           return true;
         }
         else {
           empty_Edges();
         }
       }
     }
     else {
       if(lowering_Edge[1].x != -6)
         empty_Edges();
       if(move_Detect(side) == true) {
         if(lowering_Edge[0].x == dataPacket[0] && lowering_Edge[0].y == dataPacket[1] && rising_Edge.x == dataPacket[2] && rising_Edge.y == dataPacket[3]) 
           return true;
         else 
           empty_Edges();
       }
     }
   }
   lcd.setCursor(0,0);
   lcd.print("Error: incorrect");
   lcd.setCursor(0,1);
   lcd.print("motor movement  ");
   return false;
}

void takeHumanMoves(int& side) {
  board_Scan();
  board_Analysis(false);
  while(Serial.available() && castleState == 0) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if(inputString.length() == 4) {
     stringComplete = true;
    }
  }

  if(move_Detect(side) == true) {
    castling_Detect(side);
  }
  if(move_Detect(side) == true || capture_Detect(side) == true) {
    if(!moveSent && castleState == 0) {
      Serial.print(RANKS.charAt(lowering_Edge[0].y)); 
      Serial.print(FILES.charAt(lowering_Edge[0].x));
      Serial.print(RANKS.charAt(rising_Edge.y)); 
      Serial.print(FILES.charAt(rising_Edge.x));
      if(promotion_Detect(side) == true)
        Serial.print("q");
      Serial.println();
      moveSent = true;
    }
    
    if(stringComplete || castleState == 3) {     
      moveSent = false;
      processString();           //check the legality of human moves 
      if(islegal) {
        chess_Record(side);
        display_LCD(promotion_Detect(side),side);
        empty_Edges();
        toggle(side);
      } 
      else {
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("  Invalid Move  ");
        if(piece_Captured == '\0')
          rising_Edge = EMPTY;
        else 
          empty_Edges();
      }
    }
  }
}

void takeComMoves(int& side, int& motorComplete, bool& Err1, long& timePaused) {
  while(Serial.available() && inputString.length() < 4) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inputString.length() == 4) {
     stringComplete = true;
    }
  }
     
  if(Serial1.available()) {  
    motorComplete = Serial1.read();
  }
  
  if(stringComplete) {
    processString();            //check checkmate and receive computer's moves
    if(comMove.length() > 0) {  
      decodeMove(comMove, side);
      Serial1.write(dataPacket,8);             
    }
  }
  
  if(motorComplete == 1 || motorComplete == 2) { 
    //Serial.println(motorComplete);
    if(motorComplete == 1 && positionCheck(1, true, side) == false)
       Err1 = true;
    if(motorComplete == 2) {
      unsigned long millis_Passed = millis();
      bool isCom = true;
      if(Err1)
        while(positionCheck(1, false, side) == false);
      while(positionCheck(2, isCom, side) == false) {
        isCom = false;          //activate manual correcting
      }
      timePaused += millis() - millis_Passed;
      chess_Record(side);
      display_LCD(promotion_Detect(side),side);
      empty_Edges();
      if(side == 1 && whiteMates || side == 2 && blackMates) {
        Serial.println("C");                 //Checkmate
      }  
      else {
        if(side == 1) 
          Serial.println("F1");              //Side1 motor finished 
        else
          Serial.println("F2");              //Side2 motor finished
      }
      toggle(side);
      Err1 = false;
    } 
    motorComplete = 0;
  }
}

void toggle(int& s) {
  if(draw) {
    s = 0;
  }
  else {
    if(s == 1) 
      s = 2;
    else 
      s = 1;
  }
}
