int swc1Pin = A1;
int swc2Pin = A3;

int baseMeasurement = 0;

// emperically derived values here...
int swcEdges[] = { 10, 22, 40, 90, -1 };

/*
typedef enum {
  SWC_SEEK_UP     = 0,
  SWC_SEEK_DOWN   = 1,
  SWC_VOLUME_UP   = 2,
  SWC_VOLUME_DOWN = 3,
  SWC_MODE        = 4,
  SWC_HANG_UP     = 5,
  SWC_CALL        = 6,
  SWC_SPEAK       = 7,
  SWC_NONE        = 8
} SWC_EVENT;
*/

#define  SWC_SEEK_UP      0
#define  SWC_SEEK_DOWN    1
#define  SWC_VOLUME_UP    2
#define  SWC_VOLUME_DOWN  3
#define  SWC_MODE         4
#define  SWC_HANG_UP      5
#define  SWC_CALL         6
#define  SWC_SPEAK        7
#define  SWC_NONE         8

#define KEYBOARD  0x07
#define CONSUMER  0x0c

typedef struct {
  int page, code1, code2;
} EVENT_KEY;

EVENT_KEY eventKeys[] = {
      {CONSUMER, 0x52, 0x00}, // seek up (dpad up)
      {CONSUMER, 0x51, 0x00}, // seek down (dpad down)
      {CONSUMER, 0xe9, 0x00}, // vol up
      {CONSUMER, 0xea, 0x00}, // vol down
      ///{KEYBOARD, 0x65, 0x13}, // mode (SEARCH+p)
      {KEYBOARD, 0x3a, 0x00}, // mode (menu)
      {KEYBOARD, 0x3d, 0x00}, // hang up (F4, endcall)
      {KEYBOARD, 0x3c, 0x00}, // call (F3, call)
      {KEYBOARD, 0x3c, 0x00}, // voice menu
      {0x00, 0x00, 0x00},
};

unsigned long buttonPressTime[] = {0, 0, 0, 0, 0, 0, 0, 0};
int buttonRepeatTime = 200;

const char *eventMap[] = {
  ">",
  "<",
  "+",
  "-",
  "M",
  "H",
  "C",
  "S",
  "_",
};

int swc1Buttons[] = {
  SWC_SEEK_UP, 
  SWC_SEEK_DOWN,
  SWC_VOLUME_UP,
  SWC_VOLUME_DOWN,
  SWC_NONE
};

int swc2Buttons[] = {
  SWC_MODE, 
  SWC_HANG_UP,
  SWC_CALL,
  SWC_SPEAK,
  SWC_NONE
};

void setup() {
  pinMode(swc1Pin, INPUT_PULLUP);
  pinMode(swc2Pin, INPUT_PULLUP);
  delay(500); // let analog system settle

  baseMeasurement = (analogRead(swc1Pin) + analogRead(swc2Pin)) / 2;
  swcEdges[4] = baseMeasurement / 100 * 100; // round to 100s
}

void loop() {
  int swc1Value = analogRead(swc1Pin);
  int swc2Value = analogRead(swc2Pin);

  debounceButtons(swc1Value, swc2Value);
}

int swc1LastButton = SWC_NONE;
int swc2LastButton = SWC_NONE;
unsigned long swc1DebounceTime = 0;
unsigned long swc2DebounceTime = 0;
int debounceDelay = 50;

void debounceButtons(int swc1Value, int swc2Value) {
  unsigned long sampleTime = millis();

  int swc1Button = valueToEvent(swc1Value, swc1Buttons);
  int swc2Button = valueToEvent(swc2Value, swc2Buttons);

  #ifdef debug
  Keyboard.print("at ");
  Keyboard.print(sampleTime);
  Keyboard.print(" swc1=");
  Keyboard.print(swc1Value);
  Keyboard.print("\tswc2=");
  Keyboard.println(swc2Value);
  delay(1000);
  #endif
  
  if (swc1Button != swc1LastButton) {
    swc1DebounceTime = sampleTime;
    
  }
  if (swc2Button != swc2LastButton) {
    swc2DebounceTime = sampleTime;
  }
    
  if (swc1Button != SWC_NONE && (sampleTime - swc1DebounceTime) > debounceDelay){
    fireEvent(swc1Button); 
  }
  
  if (swc2Button != SWC_NONE && (sampleTime - swc2DebounceTime) > debounceDelay){   
    fireEvent(swc2Button);
  }
  
  swc1LastButton = swc1Button;
  swc2LastButton = swc2Button;
}

int valueToEvent(int value, int *events) {

  int event = SWC_NONE;
  
  for (int i=0; i<5; i++) {
    if (value >= swcEdges[i]) {
      event = events[i];
    }
  }
  
  return event;
}
 
void fireEvent(int event) { 
  unsigned long sampleTime = millis();
  if ((sampleTime - buttonPressTime[event]) > buttonRepeatTime) {
    //Keyboard.print(eventMap[event]);
    EVENT_KEY key = eventKeys[event];
    if (key.page == CONSUMER) {
      Keyboard.set_media(key.code1);
      Keyboard.send_now();
      Keyboard.releaseAll();
    }
    else {
      Keyboard.set_key1(key.code1);
      if (key.code2) {
        Keyboard.set_key2(key.code2);
      }
      Keyboard.send_now();
      Keyboard.releaseAll();
    }
    buttonPressTime[event] = sampleTime;
  }
}
