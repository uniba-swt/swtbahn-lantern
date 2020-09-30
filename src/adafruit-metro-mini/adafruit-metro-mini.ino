/* Adafruit Metro Mini Servo lantern
 * ---------------------------------
 * 
 * Eugene Yip
 * 30 September 2020
 * 
 */

// #define DEBUG_SERVO
#define DEBUG_SYNC

typedef enum {
  ONBOARD_LED = 13
} DebugPins;

// Define pins connected to the servo ports
typedef enum {
  SERVO_PIN_AUX1   = 10, // Servo AUX1 port
  SERVO_PIN_COMMON = 11, // Common pole for servo feedback
  SERVO_PIN_AUX2   = 12  // Servo AUX2 port
} ServoPins;

// Define pins connected to the lantern LED
typedef enum {
  LANTERN_FLIP_REF = 6,  // Reference signal to flip the lantern state
  LANTERN_FLIP     = 7,  // Detects signal to flip the lantern state
  LANTERN_VCC      = 8,  // VCC at 5V
  LANTERN_PWM      = 9   // Lantern brightness using PWM
} LanternPins;

// Define pin connected to the OneControl synchronisation pulse
typedef enum {
  SYNC_PIN = 2
} SyncPin;

typedef struct {
  bool last;
  bool current;
  bool isRisingEdge;
  int pulseThreshold;
} SyncState;

volatile SyncState syncState = {
  .last = false, 
  .current = false, 
  .isRisingEdge = false, 
  .pulseThreshold = 900
};

// Define the possible servo states
typedef enum {
  IS_NEITHER  = 0,
  IS_LEFT     = 1,
  IS_RIGHT    = 2
} ServoValue;

typedef struct {
  ServoValue current;
} ServoState;

volatile ServoState servoState = {
  .current = IS_NEITHER
};

// Define the lantern brightness levels
typedef enum {
  LANTERN_OFF = 255,
  LANTERN_DIM = 230,
  LANTERN_ON  = 100
} LanternBrightness;

typedef struct {
  int pwmValue;
  int pwmDirection;
  bool isGlowCycleComplete;
  bool isFlipped;
} LanternState;

volatile LanternState lanternState = {
  .pwmValue = LANTERN_DIM - 1,
  .pwmDirection = -1,
  .isGlowCycleComplete = false,
  .isFlipped = false
};

void setup(void) {
  // Start serial session
  Serial.begin(9600);
  Serial.println("Servo lantern: 30 September 2020");

  // Servo feedback pins (Active low)
  pinMode(SERVO_PIN_AUX1, INPUT_PULLUP);
  pinMode(SERVO_PIN_COMMON, OUTPUT);
  pinMode(SERVO_PIN_AUX2, INPUT_PULLUP);
  digitalWrite(SERVO_PIN_COMMON, LOW);

  // Lantern LED
  pinMode(LANTERN_VCC, OUTPUT);
  pinMode(LANTERN_PWM, OUTPUT);
  digitalWrite(LANTERN_VCC, HIGH); 
  analogWrite(LANTERN_PWM, LANTERN_OFF);
  lanternGlowReset();

  // Lantern state flip
  pinMode(LANTERN_FLIP_REF, OUTPUT);
  pinMode(LANTERN_FLIP, INPUT_PULLUP);
  digitalWrite(LANTERN_FLIP_REF, LOW);

  // OneControl pulse synchronisation
  pinMode(SYNC_PIN, INPUT_PULLUP);

  // Debug LED on the microcontroller board
  pinMode(ONBOARD_LED, OUTPUT);

  /* * * * */
  cli();      // Disable global interrupts

  // Trigger the lantern glowing effect on the rising edge of the synchronisation pulse
  attachInterrupt(digitalPinToInterrupt(SYNC_PIN), lanternGlowReset, RISING);

  // Timer/Counter 0 at 96Hz
  TCCR0A = 0; // Clear register TCCR0A
  TCCR0B = 0; // Clear register TCCR0B
  TCNT0  = 0; // Reset counter value
  // Set compare match register for 96Hz increments
  OCR0A = 80;  // = 16MHz/(96Hz*2*1024 prescaler) - 1, (OCR0A value must be less than 256)
  TCCR0A |= (1 << WGM01); // Enable Clear Timer on Capture Match mode
  TCCR0B |= (1 << CS02) | (1 << CS00);  // Prescaler of 1024
  TIMSK0 |= (1 << OCIE0A);  // Enable timer compare interrupt

  /* * * * */
  sei();  // Enable global interrupts
}

// Interrupt service routine: Timer/Counter 0 at 96Hz
// Updates the lantern's next state of its glowing cycle and
// tracks the synchronisation pulse
ISR(TIMER0_COMPA_vect){
  // Lantern glowing state
  if (!lanternState.isGlowCycleComplete) {
    if ((lanternState.pwmValue == LANTERN_DIM) || (lanternState.pwmValue == LANTERN_ON)) {
      lanternState.isGlowCycleComplete = (lanternState.pwmDirection == 1);
      lanternState.pwmDirection *= -1;
    }
    lanternState.pwmValue = lanternNextPwmValue(lanternState);  
  }

  lanternState.isFlipped = (digitalRead(LANTERN_FLIP) == 0);
}

// Resets the lantern glowing cycle
void lanternGlowReset(void) {
  syncStatePrintOut();
  
  lanternState.pwmValue = LANTERN_DIM - 1;
  lanternState.pwmDirection = -1;
  lanternState.isGlowCycleComplete = false;
}

// Stateless function to define the lantern's glowing waveform
int lanternNextPwmValue(volatile LanternState& lanternState) {
  return lanternState.pwmValue + lanternState.pwmDirection;
}

void updateServoState(void) {  
  int aux1State = digitalRead(SERVO_PIN_AUX1);
  int aux2State = digitalRead(SERVO_PIN_AUX2);

  if ((aux1State == LOW) && (aux2State == HIGH)) {
    servoState.current = IS_RIGHT;
  } else if ((aux1State == HIGH) && (aux2State == HIGH)) {
    servoState.current = IS_LEFT;
  } else {
    servoState.current = IS_NEITHER;
  }

  if (lanternState.isFlipped) {
    switch (servoState.current) {
      case (IS_RIGHT): servoState.current = IS_LEFT;  break;
      case(IS_LEFT):   servoState.current = IS_RIGHT; break;
      default:         break;
    }
  }
}

void updateSyncState(void) {
  syncState.last = syncState.current;
  syncState.current = (analogRead(SYNC_PIN) > syncState.pulseThreshold);
  syncState.isRisingEdge = (!syncState.last && syncState.current);
}

void syncStatePrintOut(void) {
  #ifdef DEBUG_SYNC
    digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));
  #endif
}

void servoStatePrintOut(void) {
  #ifdef DEBUG_SERVO
    Serial.print("servoState.current: ");

    switch (servoState.current) {
      case (IS_RIGHT):   Serial.println("right");   break;
      case (IS_LEFT):    Serial.println("left");    break;
      case (IS_NEITHER): 
      default:           Serial.println("neither"); break;
    }
  #endif
}

void loop(void) {
  updateServoState();
  servoStatePrintOut();

  switch (servoState.current) {
    case (IS_RIGHT):   analogWrite(LANTERN_PWM, LANTERN_ON);
                       break;
    
    case (IS_LEFT):    analogWrite(LANTERN_PWM, lanternState.pwmValue);
                       break;
 
    case (IS_NEITHER): 
    default:           analogWrite(LANTERN_PWM, LANTERN_OFF);
                       break;
  }
}
