//#define TEST

/* pin configuration */
#define PUMP_A     5
#define PUMP_B     6
#define PUMP_SPEED 3
#define HYGROMETER A0
#define ERROR_LED  13

/* hygrometer values */
#define WATERING_THRESHOLD 1000

/* timing values */
#define CHECK_EVERY 3600000  // 1 hour
#define WATER_FOR   300000   // 5 minutes

/* how many samples to collect */
#define SAMPLES 120

/* for pump helper function */
#define FORWARDS  0
#define BACKWARDS 1
#define STOP      3

/* states */
#define NORMAL  0
#define WATERED 1
#define ERRORED 2
byte state = ERRORED;

int water_plant(void);
int check_for_error_after_watering(void);
void set_pump(char, char);
int delay_ms(unsigned long);
void led_blink(unsigned long);

#ifdef TEST
void test_systems(void);
#endif

void setup() {
  Serial.begin(9600);
  pinMode(PUMP_A,     OUTPUT);
  pinMode(PUMP_B,     OUTPUT);
  pinMode(PUMP_SPEED, OUTPUT);
  pinMode(ERROR_LED,  OUTPUT);
  set_pump(STOP, 0);
}

#ifndef TEST
void loop() {
  int reading;

  if (state == NORMAL) {
    if (delay_ms(60000)) {
      Serial.print("Checking hygrometer... ");
      reading = sample_data();
      Serial.println(reading);
  
      if (reading > WATERING_THRESHOLD) {
        Serial.print("Watering... ");
        if (water_plant() > 0) {
          Serial.println("ERROR: plant is still dry! Shutting down until reset.");
          state = ERRORED;
        } else
          Serial.println("done!");
      }
    }
  } else if (state == ERRORED) {
    /* signal SOS */
    led_blink(100);   /* S */
    led_blink(100);
    led_blink(100);
    delay(100);
    
    led_blink(300);   /* O */
    led_blink(300);
    led_blink(300);
    delay(100);

    led_blink(100);   /* S */
    led_blink(100);
    led_blink(100);
    delay(100);

    delay(1000);
  }
}
#else
void loop() {
  test_systems();
  delay(1000);
}
#endif

/*
 * Collect several samples of data to confirm that the plant is in
 * need of watering (data can tend to spike).
 */
int sample_data() {
  int i;
  double avg = 0.0;
  
  for (i = 0; i < SAMPLES; i++) {
    avg += float(analogRead(HYGROMETER)) / (float)SAMPLES;
    delay(50);
  }

  return (int)avg;
}

/*
 * Runs the pump!
 * Returns 1 on error on 0 on success.
 */
int water_plant() {
  set_pump(FORWARDS, 128);
  delay(WATER_FOR);
  set_pump(STOP, 0);
  return check_for_error_after_watering();
}

int check_for_error_after_watering() {
  /* if the hygrometer is STILL reading over the threshold after watering,
   * then there's probably something wrong. */
  if (sample_data() > WATERING_THRESHOLD)
    return 1;
  else
    return 0;
}

/* 
 *  pump helper function
 *  
 *  designed to work with L289N
 *  (see https://tronixlabs.com.au/news/tutorial-l298n-dual-motor-controller-module-2a-and-arduino/)
 *  
 *  arguments:
 *    - direction: FORWARDS, BACKWARDS, or STOP (as defined above)
 *    - speed: 0-255, the speed at which the pump will run 
 */
void set_pump(char direction, char speed) {

  switch (direction) {
    case FORWARDS:
      digitalWrite(PUMP_A, HIGH);
      digitalWrite(PUMP_B, LOW);
      digitalWrite(PUMP_SPEED, speed);
      break;
      
    case BACKWARDS:
      digitalWrite(PUMP_A, LOW);
      digitalWrite(PUMP_B, HIGH);
      digitalWrite(PUMP_SPEED, speed);
      break;

    case STOP:
    default:
      digitalWrite(PUMP_A, LOW);
      digitalWrite(PUMP_B, LOW);
      digitalWrite(PUMP_SPEED, speed);
      break;
  }
}

/*
 * delay helper
 * because delay isn't all that reliable over long periods of time
 * 
 * arguments:
 *  - milliseconds: nummber of milliseconds to delay
 * 
 * returns:
 *  - 0 if the timer is still running
 *  - 1 when the requested delay has been reached
 */

#define STOPPED 0
#define STARTED 1

unsigned long time_started = 0;
byte timer_state = STOPPED;

int delay_ms(unsigned long milliseconds) {
  unsigned long time_taken;

  if (timer_state == STOPPED) {
    /* reset timer */
    time_started = millis();
    timer_state = STARTED;
  } else if (millis() < time_started) {
    /* clock has rolled over */
    time_taken = millis() + 2 - (4294967295 - time_started);
  } else {
    time_taken = millis() + 1 - time_started;
  }

  if (time_taken >= milliseconds) {
    timer_state = STOPPED;
    return 1;
  } else
    return 0;
}

void led_blink(unsigned long milliseconds) {
  digitalWrite(ERROR_LED, HIGH);
  delay(milliseconds);
  digitalWrite(ERROR_LED, LOW);
  delay(milliseconds);
}

#ifdef TEST
void test_systems() {
  int i;

  Serial.println("Sample readings...");
  for (i = 0; i < 5; i++) {
    Serial.println(sample_data());
  }

  delay(500);

  Serial.print("Running forwards at full speed... ");
  set_pump(FORWARDS, 255);
  delay(1000);
  set_pump(STOP, 0);
  Serial.println("done!");

  delay(500);
  
  Serial.print("Running backwards at full speed... ");
  set_pump(FORWARDS, 255);
  delay(1000);
  set_pump(STOP, 0);
  Serial.println("done!");

  delay(500);

  Serial.print("Running forwards at half speed... ");
  set_pump(FORWARDS, 128);
  delay(1000);
  set_pump(STOP, 0);
  Serial.println("done!");
}
#endif

