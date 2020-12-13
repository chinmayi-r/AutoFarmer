#define enc1 2 
#define enc2 3
#define enc3 18
#define enc4 19

const int dir =  4;
const int ste  = 5;

#define onerev 1890
#define steps_per_rev 200

volatile long enc_val1 = 0;
volatile long enc_val2 = 0;
volatile long enc_val3 = 0;
volatile long enc_val4 = 0;

bool enc_state[4] = {0, 0, 0, 0};

const byte buffSize = 40;
char inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;

char messageFromPC[buffSize] = {0};
int xval = 0;
int yval = 0;
float weedicide = 0.0; // fraction of servo range to move
char END = "D"
boolean ended = true;

unsigned long curMillis;
unsigned long prevReplyToPCmillis = 0;
unsigned long replyToPCinterval = 1000;


void setup()
{
    Serial.begin(9600);

    pinMode(dir, OUTPUT);
    pinMode(ste, OUTPUT);

    for(int i = 22; i < 30; i++)
    {
        pinMode(i, OUTPUT);
    }
    
    pinMode(enc1, INPUT_PULLUP);
    pinMode(enc2, INPUT_PULLUP);
    pinMode(enc3, INPUT_PULLUP);
    pinMode(enc4, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(enc1), updenc1, FALLING);
    attachInterrupt(digitalPinToInterrupt(enc2), updenc2, FALLING);
    attachInterrupt(digitalPinToInterrupt(enc3), updenc3, FALLING);
    attachInterrupt(digitalPinToInterrupt(enc4), updenc4, FALLING);

    delay(500);
    Serial.println("<Arduino is ready>");
}

void forward(int pixels)
{
    enc_val1 = 0;
    enc_val2 = 0;
    enc_val3 = 0;
    enc_val4 = 0;

    digitalWrite(22, LOW);
    digitalWrite(23, HIGH);
    digitalWrite(24, LOw);
    digitalWrite(25, HIGH);
    digitalWrite(26, HIGH);
    digitalWrite(27, LOW);
    digitalWrite(28, HIGH);
    digitalWrite(29, LOW);

    int mm = map(pixels, 0, 480, 0 , 200);

    int required = (int)((((float)onerev) / ((float)330)) * (float)(mm));
    while(1)
    {
        if(enc_state[0] && enc_state[1] && enc_state[2] && enc_state[3])
            break;
        if(enc_val1 >= required)
        {
            digitalWrite(22, LOW);
            digitalWrite(23, LOW);
            enc_state[0] = 1;            
        }
        if(enc_val2 >= required)
        {
            digitalWrite(24, LOW);
            digitalWrite(25, LOW);
            enc_state[1] = 1; 
        }
        if(enc_val3 >= required)
        {
            digitalWrite(26, LOW);
            digitalWrite(27, LOW);
            enc_state[2] = 1;
        }
        if(enc_val4 >= required)
        {
            digitalWrite(28, LOW);
            digitalWrite(29, LOW);
            enc_state[3] = 1;
        }
    }    
}

void loop()
{
    curMillis = millis();
    getDataFromPC();
}


void updenc1()
{
    enc_val1++;
}

void updenc2()
{
    enc_val2++;
}

void updenc3()
{
    enc_val3++;
}

void updenc4()
{
    enc_val4++; 
}

void step(int pix, int we)
{
    map(pix, 0, 640, 0, 290);
    mm = pix;
    bool direction;
    if(mm < 145)
    {
        mm = 145 - mm;
        direction = false;
    }
    else
    {
        mm = mm - 145
        direction = true;
    }
    
    if(direction)
        digitalWrite(dir, HIGH); 
    else
        digitalWrite(dir, LOW);

    int revolutions = (mm / 10) * 1.25;

    for(int i = 0; i < (revolutions * steps_per_rev); i++)
    {
        digitalWrite(ste, HIGH);
        delayMicroseconds(400);
        digitalWrite(ste, LOW);
        delayMicroseconds(400);
    }

    digitalWrite(31, HIGH);
    delay(map(we, 0, 10, 0, 3) * 1000);
    digitalWrite(31, LOW);

    if(!(direction))
        digitalWrite(dir, HIGH); 
    else
        digitalWrite(dir, LOW);

    for(int i = 0; i < (revolutions * steps_per_rev); i++)
    {
        digitalWrite(ste, HIGH);
        delayMicroseconds(400);
        digitalWrite(ste, LOW);
        delayMicroseconds(400);
    }
}

void getDataFromPC() {

    // receive data from PC and save it into inputBuffer

  if(Serial.available() > 0) {

    char x = Serial.read();

      // the order of these IF clauses is significant

    if (x == endMarker) {
      readInProgress = false;
      newDataFromPC = true;
      inputBuffer[bytesRecvd] = 0;
      parseData();
    }

    if(readInProgress) {
      inputBuffer[bytesRecvd] = x;
      bytesRecvd ++;
      if (bytesRecvd == buffSize) {
        bytesRecvd = buffSize - 1;
      }
    }

    if (x == startMarker) {
      bytesRecvd = 0;
      readInProgress = true;
    }

    if(x == END){
       Serial.println("E");

    }
  }
}

void parseData() 
{
    char * strtokIndx; // this is used by strtok() as an index
    int done = strcmp(inputBuffer, "done");  
    if(done != 0)
    {
    strtokIndx = strtok(inputBuffer,",");      
    xval = atoi(strtokIndx); 

    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    yval = atoi(strtokIndx);     // convert this part to an integer

    strtokIndx = strtok(NULL, ",");
    weedicide = atof(strtokIndx);     // convert this part to a float

    forward(yval);
    step(xval, weedicide);
    }
    else
    {
        gotoend();
        replyToPC();
        delay(2000);
    }
    
}

void replyToPC() 
{
  if (newDataFromPC) {
    newDataFromPC = false;
    Serial.print("<Msg ");
    Serial.print(messageFromPC);
    Serial.print("Moved>");
  }
}

void gotoend()
{
    forward(480 - yval);
}
