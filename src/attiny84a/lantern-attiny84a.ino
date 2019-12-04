/* ATtiny84A Servo lantern
 * -----------------------
 *
 * Eugene Yip
 * 6 November 2019
 *
 */

// #define DEBUG_SERVO
#define DEBUG_SYNC

typedef enum {
  ONBOARD_LED = 3
} DebugPins;

// Define pins connected to the servo ports
typedef enum {
  SERVO_PIN_AUX1   = 0, // Servo AUX1 port
  SERVO_PIN_AUX2   = 1  // Servo AUX2 port
} ServoPins;

// Define pins connected to the lantern LED
typedef enum {
  LANTERN_PWM = 8   // Lantern brightness using PWM
} LanternPins;

// Define pin connected to the OneControl synchronisation pulse
typedef enum {
  SYNC_PIN = 9,
  SYNC_INT = PCINT9,
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
  LANTERN_OFF = 255 - 255,
  LANTERN_DIM = 255 - 230,
  LANTERN_MID = 255 - 127,
  LANTERN_ON  = 255 - 0
} LanternBrightness;

typedef struct {
  int pwmValue;
  int pwmDirection;
  bool isGlowCycleComplete;
} LanternState;

volatile LanternState lanternState = {
  .pwmValue = LANTERN_DIM + 1,
  .pwmDirection = 1,
  .isGlowCycleComplete = false
};

void setup(void) {
  // Servo feedback pins (Active low)
  pinMode(SERVO_PIN_AUX1, INPUT);
  pinMode(SERVO_PIN_AUX2, INPUT);

  // Lantern LED
  pinMode(LANTERN_PWM, OUTPUT);
  analogWrite(LANTERN_PWM, LANTERN_OFF);
  lanternGlowReset();

  // OneControl pulse synchronisation
  pinMode(SYNC_PIN, INPUT);

  // Debug LED on the microcontroller board
  pinMode(ONBOARD_LED, OUTPUT);


  /* * * * */
  cli();      // Disable global interrupts

  // Trigger the lantern glowing effect on the rising edge of the synchronisation pulse
  GIMSK = (1 << PCIE1); // Pin Change Interrupt Enable 1
  GIFR = (1 << PCIF1); // Pin Change Interrupt Flag 1
  PCMSK1 = (1 << SYNC_INT); //Pin Change Enable Mask 9

  // Timer/Counter 1 at 600Hz
  TCCR1A = 0; // Clear register TCCR0A
  TCCR1B = 0; // Clear register TCCR0B
  TCNT1  = 0; // Reset counter value
  // Set compare match register for 600Hz increments
  OCR1A = 40;  // = 8MHz/(3kHz*64 prescaler) - 1, (OCR0A value must be less than 65536)
  TCCR1B |= (1 << WGM12); // Enable Clear Timer on Capture mode
  TCCR1B |= (1 << CS11) | (1 << CS10);  // Prescaler of 64
  TIMSK1 |= (1 << OCIE1A);  // Enable timer compare interrupt

  /* * * * */
  sei();  // Enable global interrupts
}

// Interrupt service routine: Timer/Counter 1 at 600Hz
// Updates the lantern's next state of its glowing cycle and
// tracks the synchronisation pulse
ISR(TIM1_COMPA_vect) {
  // Lantern glowing state
  if (!lanternState.isGlowCycleComplete) {
    if ((lanternState.pwmValue == LANTERN_DIM) || (lanternState.pwmValue == LANTERN_ON)) {
      lanternState.isGlowCycleComplete = (lanternState.pwmDirection == -1);
      lanternState.pwmDirection *= -1;
    }
    lanternState.pwmValue = lanternNextPwmValue(lanternState);
  }
}

// Interrupt service routine: Synchronisation pulse on rising edge
ISR(PCINT1_vect) {
  updateSyncState();
  if (syncState.isRisingEdge) {
    lanternGlowReset();
  }
}

// Resets the lantern glowing cycle
void lanternGlowReset(void) {
  syncStatePrintOut();

  lanternState.pwmValue = LANTERN_DIM + 1;
  lanternState.pwmDirection = 1;
  lanternState.isGlowCycleComplete = false;
}

// Stateless function to define the lantern's glowing waveform
int lanternNextPwmValue(volatile LanternState& lanternState) {
  return lanternState.pwmValue + lanternState.pwmDirection;
}

void updateServoState(void) {
  int aux1State = digitalRead(SERVO_PIN_AUX1);
  int aux2State = digitalRead(SERVO_PIN_AUX2);

  if ((aux1State == HIGH) && (aux2State == LOW)) {
    servoState.current = IS_RIGHT;
  } else if ((aux1State == LOW) && (aux2State == HIGH)) {
    servoState.current = IS_LEFT;
  } else {
    servoState.current = IS_NEITHER;
  }
}

void updateSyncState(void) {
  syncState.isRisingEdge = (digitalRead(SYNC_PIN) == true);
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
