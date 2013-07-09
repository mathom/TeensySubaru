int swc1Pin = A1;
int swc2Pin = A3;

int baseMeasurement = 0;

// emperically derived values here...
int swcEdges[] = { 10, 22, 40, 90, -1 };

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

SWC_EVENT lastEvent = SWC_NONE;

#define KEYBOARD  0x07
#define CONSUMER  0x0c

typedef struct {
  int page, code;
} EVENT_KEY;

EVENT_KEY eventKeys[] = {
      {CONSUMER, 0x0224},
      {CONSUMER, 0x0225},
      {CONSUMER, 0xe9}, // vol up
      {CONSUMER, 0xea}, // vol down
      {CONSUMER, 0x01b7},
      {KEYBOARD, 0x3d}, // hang up
      {KEYBOARD, 0x3c}, // voice call prompt
      {CONSUMER, 0x95},
      {0x00, 0x00},
};

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

int eventDelay[] = {
  200,
  200,
  200,
  200,
  500,
  500,
  500,
  500,
  100, // repoll on SWC_NONE event
};

SWC_EVENT swc1Events[] = {
  SWC_SEEK_UP, 
  SWC_SEEK_DOWN,
  SWC_VOLUME_UP,
  SWC_VOLUME_DOWN,
  SWC_NONE
};

SWC_EVENT swc2Events[] = {
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
}

void loop() {
  int swc1Value = analogRead(swc1Pin);
  int swc2Value = analogRead(swc2Pin);

  if (baseMeasurement == 0) {
    baseMeasurement = (swc1Value + swc2Value) / 2;
    swcEdges[4] = baseMeasurement / 100 * 100; // round to 100s
  }
  else {
    processEvent(swc1Value, swc2Value);
  }
}

void processEvent(int swc1, int swc2) {

  SWC_EVENT currentEvent = SWC_NONE;
  SWC_EVENT event1 = SWC_NONE;
  SWC_EVENT event2 = SWC_NONE;
  
  for (int i=0; i<5; i++) {
    if (swc1 >= swcEdges[i]) {
      event1 = swc1Events[i];
    }
    
    if (swc2 >= swcEdges[i]) {
      event2 = swc2Events[i];
    }
  }
  
  // these are mutually exclusive but you never really press more than one
  // (you're using your thumb on one side of the wheel)
  if (event1 != SWC_NONE) {
    currentEvent = event1;
  }
  else if (event2 != SWC_NONE) {
    currentEvent = event2;
  }
  
  if (currentEvent != SWC_NONE) {
    //Keyboard.print(eventMap[currentEvent]);
    EVENT_KEY key = eventKeys[currentEvent];
    if (key.page == CONSUMER) {
      Keyboard.set_media(key.code);
      Keyboard.send_now();
      //delay(100);
      Keyboard.releaseAll();
    }
    else {
      Keyboard.set_key1(key.code);
      Keyboard.send_now();
      Keyboard.releaseAll();
    }
    delay(eventDelay[event2]);
  }
  
  delay(eventDelay[currentEvent]);
}
