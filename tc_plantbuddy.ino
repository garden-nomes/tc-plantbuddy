/* pin configuration */
#define PUMP_A     5
#define PUMP_B     6
#define PUMP_SPEED 3
#define HYGROMETER A0

/* hygrometer values */
#define DRY 1000
#define WET 300

/* amount of time pump will run before automatically shutting off
 * (in case hygrometer fails or resevoir is empty) */
#define PUMP_TIMEOUT 60000  // one minute

/* amount of time program will pause after watering */
#define POST_WATERING_DELAY 360000  // one hour

/* for pump helper function */
#define FORWARDS  0
#define BACKWARDS 1
#define STOP      3

void water_plant();
void set_pump(short int, short int);

void setup() {
  pinMode(PUMP_A,     OUTPUT);
  pinMode(PUMP_B,     OUTPUT);
  pinMode(PUMP_SPEED, OUTPUT);
}

void loop() {
  if (analogRead(HYGROMETER) >= DRY)
    water_plant();
  
  delay(1000);
}

void water_plant() {
  unsigned int counter = 0;

  /* turn on pump */
  set_pump(FORWARDS, 128);

  /* wait until the hygrometer says it's wet or we timeout */
  while (analogRead(HYGROMETER) >= WET && counter < PUMP_TIMEOUT) {
    delay(20);
    counter += 20;
  }

  /* pause and run pump in reverse for a second */
  set_pump(STOP, 0);
  delay(500);
  set_pump(BACKWARDS, 128);
  delay(1000);
  set_pump(STOP, 0);

  /* wait before resuming program */
  delay(POST_WATERING_DELAY);
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
void set_pump(short int direction, short int speed) {

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

