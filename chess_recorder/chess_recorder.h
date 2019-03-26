struct Coord
{
  int x;
  int y;
};             

const Coord EMPTY = {-6,-6};

const byte HOMING[8] = {255,255,0,0,255,255,255,255};
byte dataPacket[8]; //Move x1, Move y1, Move x2, Move y2, Capture x, Capture y, Castling king x, Castling king y

//for serial communication with sjeng
bool moveSent;
String inputString;     
bool stringComplete;
String comMove;

bool islegal;   
bool whiteMates;
bool blackMates;
bool draw;

//for chess recording
const String FILES = "87654321";    //x
const String RANKS = "abcdefgh";    //y
int board_State[8][8];
int last_Board_State[8][8];

char piecesW[] = {'P','P','P','P','P','P','P','P','R','N','B','Q','K','B','N','R'};
Coord White[16];   //6   6   6   6   6   6   6   6   7   7   7   7   7   7   7   7
                   //0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7
char piecesB[] = {'r','n','b','q','k','b','n','r','p','p','p','p','p','p','p','p'}; 
Coord Black[16];   //0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1
                   //0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7   
                                  
Coord rising_Edge;
Coord lowering_Edge[2];            
char piece_Moved;
char piece_Captured; 

//for castling
int castleState;   //0: idle state 1: king moved queenside 2: king moved kingside 3: castling completed (rook moved correctly) 

//icons
byte pawnB[8] = {0b00000,0b00000,0b01110,0b01110,0b01110,0b00100,0b01110,0b01110};
byte pawnW[8] = {0b00000,0b00000,0b01110,0b01010,0b01110,0b00100,0b00100,0b01110};
byte rookB[8] = {0b10101,0b11111,0b01110,0b01110,0b01110,0b01110,0b11111,0b11111};
byte rookW[8] = {0b10101,0b11111,0b01010,0b01010,0b01010,0b01010,0b01010,0b11111};
byte knightB[8] = {0b00100,0b01110,0b00011,0b00111,0b00111,0b00110,0b01111,0b01111};
byte knightW[8] = {0b00100,0b01110,0b00011,0b00111,0b00011,0b00010,0b00010,0b01111};
byte bishopB[8] = {0b00100,0b01110,0b01110,0b01110,0b00100,0b00100,0b11111,0b11111};
byte bishopW[8] = {0b00100,0b01110,0b01010,0b01010,0b00100,0b00100,0b00100,0b11111};
byte queenB[8] = {0b10101,0b10101,0b11111,0b01110,0b00100,0b01110,0b01110,0b01110};
byte queenW[8] = {0b10101,0b10101,0b01110,0b00100,0b00100,0b00100,0b01110,0b01110};
byte kingB[8] = {0b00100,0b01110,0b00100,0b11111,0b11111,0b01110,0b11111,0b11111};
byte kingW[8] = {0b00100,0b01010,0b00100,0b11111,0b11011,0b01010,0b01010,0b11111};
