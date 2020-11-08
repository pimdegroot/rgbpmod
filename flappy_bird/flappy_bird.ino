#define SDA 2
#define SCL 3

void i2cStart() {
  //generates a high to low SDA transition during SCL high as a start condition
  pinMode(SDA, OUTPUT);
  pinMode(SCL, OUTPUT);
  digitalWrite(SCL, LOW);
  digitalWrite(SDA, HIGH);

  digitalWrite(SCL, HIGH);
  digitalWrite(SDA, LOW);

  digitalWrite(SCL,LOW);
}

void i2cStop() {
  //generates a low to high SDA transition during SCL high as a start condition
  pinMode(SDA, OUTPUT);
  pinMode(SCL, OUTPUT);
  digitalWrite(SCL, LOW);
  digitalWrite(SDA, LOW);

  digitalWrite(SCL, HIGH);
  digitalWrite(SDA, HIGH);

  digitalWrite(SCL,LOW); 
}

byte i2cSend(byte data) {
  //sends data to device and checks for ack
  byte ack = 2;
  pinMode(SDA, OUTPUT);
  pinMode(SCL, OUTPUT);

  digitalWrite(SCL, LOW);
  //send out data
  shiftOut(SDA,SCL,LSBFIRST,data);

  //the GN1640/TN1640 doesn't need an ack

  /*//check for ack
  pinMode(SDA, INPUT);

  digitalWrite(SCL,HIGH);

  ack = digitalRead(SDA);

  digitalWrite(SCL, LOW);
  pinMode(SDA, OUTPUT);*/

  return ack;  
}

void red(){
  //function to create the red game over screen
  i2cStart();
  i2cSend(B11000000); //Address 0
  i2cSend(B11111111);
  i2cSend(B00000000);
  i2cSend(B00000000);
  i2cSend(B11111111);
  i2cSend(B00000000);
  i2cSend(B00000000);
  i2cSend(B11111111);
  i2cSend(B00000000);
  i2cSend(B00000000);
  i2cSend(B11111111);
  i2cSend(B00000000);
  i2cSend(B00000000);
  i2cSend(B11111111);
  i2cSend(B00000000);
  i2cSend(B00000000);
  i2cSend(B01010101);
  i2cStop();
}

byte framebuffer [5][3]; //This array contains the image data, with the first index being the row number, and the second color
byte flappy_height; //Row number of flappy, same as the frame row number
byte pipecounter, pipelocation; //counter for the generation of pipes and the row number of the opening
unsigned int difficulty; //This stores the delay between frames and determines the difficulty

void setup() {

  //filling the initial screen into the framebuffer

  framebuffer[0][0] = B00000001; //Red
  framebuffer[0][1] = B11111111; //Green
  framebuffer[0][2] = B10111110; //Blue

  framebuffer[1][0] = B00000000; //Red
  framebuffer[1][1] = B11111111; //Green
  framebuffer[1][2] = B11111111; //Blue

  framebuffer[2][0] = B00000000; //Red
  framebuffer[2][1] = B11111111; //Green
  framebuffer[2][2] = B11111111; //Blue

  framebuffer[3][0] = B00000000; //Red
  framebuffer[3][1] = B11111111; //Green
  framebuffer[3][2] = B10111111; //Blue

  framebuffer[4][0] = B00000000; //Red
  framebuffer[4][1] = B11111111; //Green
  framebuffer[4][2] = B10111111; //Blue


  i2cStop();
  
  i2cStart();
  i2cSend(B10001001); //Torning the screen on and using the last 3 bits setting the brightness
  i2cStop();

  //setting the defaults for the variables

  flappy_height = 0;
  pipecounter = 2;
  pipelocation = 1;
  difficulty = 800;

  //Pin 16 is the button on the microcontroller board, 22 and 23 the LEDs
  pinMode(16,INPUT);
  pinMode(22,OUTPUT);
  pinMode(23,OUTPUT);

  digitalWrite(22,LOW);
  digitalWrite(23,LOW);
}

void loop() {
  //Sending the frame to the screen.
  i2cStart();
  i2cSend(B11000000); //Address 0
  i2cSend(framebuffer[0][0]);
  i2cSend(framebuffer[0][1]);
  i2cSend(framebuffer[0][2]);
  i2cSend(framebuffer[1][0]);
  i2cSend(framebuffer[1][1]);
  i2cSend(framebuffer[1][2]);
  i2cSend(framebuffer[2][0]);
  i2cSend(framebuffer[2][1]);
  i2cSend(framebuffer[2][2]);
  i2cSend(framebuffer[3][0]);
  i2cSend(framebuffer[3][1]);
  i2cSend(framebuffer[3][2]);
  i2cSend(framebuffer[4][0]);
  i2cSend(framebuffer[4][1]);
  i2cSend(framebuffer[4][2]);

  i2cSend(B11111111); //This row is not connected on the screen
  i2cStop();

  delay(difficulty/2); //Waiting for half the difficulty, to give some time to release the button after the frame change

  flappy_height = flappy_height + 1; //by default flappy drops by one row each frame
  if(digitalRead(16) == true) {
    //if the button is pressed, flappy moves up;
    flappy_height = flappy_height - 2;
  }
  if(flappy_height == 5) {
    //if the bottom is reached, flappy chrashes and game over.
    while(1){
      red();
      digitalWrite(22,HIGH); //the reason why the game is over is shown here
      delay(1000);
    }
  }
  if(flappy_height == 255) {
    //flappy can't exit the screen
    flappy_height = 0;
  }

  delay(difficulty/2); //the other half of the delay is used here

  if(bitRead(framebuffer[flappy_height][2],1) == 0){
    //this part checks for a colision between a tube and flappy, by checking if the pixel ahead contains blue. No air = chrash
    while(1){
      red();
      digitalWrite(23,HIGH);
      delay(1000);
    }
  }

  pipecounter = pipecounter + 1;
  if(pipecounter == 6) {
    //generate a pipe every 5 frames, with a random height;
    pipecounter = 0;
    pipelocation = random(0,4);
    //the delay is decreased every pipe with 10ms
    difficulty = difficulty - 10;
  }

  //the entire frame is shifted by one column
  for(byte y=0;y<5;y++) {
    for(byte c=0;c<3;c++){ 
      //shift entire field
      framebuffer[y][c] = framebuffer[y][c] >> 1;
      if(c==1 || c==2) {
        //Create sky
        framebuffer[y][c] = framebuffer[y][c] | B10000000;
      }

      //regenerate flappy, by adding red and removing blue
      if(y == flappy_height) {
        if(c==0) {
          framebuffer[y][c] = framebuffer[y][c] | B00000001;
        }
        if(c==2) {
          framebuffer[y][c] = framebuffer[y][c] & B11111110;
        }
      }

      //place pipes, by removing blue
      if(pipecounter == 0) {
        if(pipelocation != y && pipelocation+1 != y && c == 2) {
           framebuffer[y][c] = framebuffer[y][c] & B01111111;
        }
      }
    }
  }

  
  
    

}
