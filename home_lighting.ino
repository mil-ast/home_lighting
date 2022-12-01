#define IS_DEBUG_MODE 0
#define CORRIDOR_LIGHT_ON_DELAY 6000
#define STAIRS_LIGHT_ON_DELAY 6000

enum State {
  STATE_WAIT,
  STATE_UP,
  STATE_DOWN,
};

const int maxLightLevel = 130; // макс значение датчика света для работы (0 темно, 1023 светло)
const int lightPin = A3; // датчик света
const int pinSwitchCorridor = 5; // включатель коридора   PWM 5
const int pinSwitchStairs = 6; // включатель лестницы     PWM 6
const int pinMotionSensorCorridor1 = 7; // датчик движения коридора у лестницы
const int pinMotionSensorCorridor2 = 8; // датчик движения коридора у санузла
const int pinMotionSensorStairs1 = 9; // датчик движения лестницы верх
const int pinMotionSensorStairs2 = 10; // датчик движения лестницы низ

struct StateValues {  
  int lightLevel;
  unsigned long timeOn ;
  State state;
  int sensor1;
  int sensor2;
};

// коридор второго этажа
struct StateValues coridorValues = { 0, 0, STATE_WAIT, 0, 0 };

// лестница на второй этаж
struct StateValues stairsValues = { 0, 0, STATE_WAIT, 0, 0 };

const int lightLevelStep = 3;
unsigned long lastLogTime = 0;

void setup() {
  #if (IS_DEBUG_MODE == 1)
    Serial.begin(9600);
  #endif

  pinMode(pinSwitchCorridor, OUTPUT);
  analogWrite(pinSwitchCorridor, 0);
  pinMode(pinMotionSensorCorridor1, INPUT);
  pinMode(pinMotionSensorCorridor2, INPUT);

  pinMode(pinSwitchStairs, OUTPUT);
  analogWrite(pinSwitchStairs, 0);
  pinMode(pinMotionSensorStairs1, INPUT);
  pinMode(pinMotionSensorStairs2, INPUT);
}

void loop() {
  // если светло и свет отключен, то ожидаем
  const int lightSensorValue = analogRead(lightPin);
  if (lightSensorValue > maxLightLevel) {
    if (coridorValues.state == STATE_WAIT && stairsValues.state == STATE_WAIT && coridorValues.lightLevel == 0 && stairsValues.lightLevel == 0) {
      #if (IS_DEBUG_MODE == 1)
        lastLogTime = millis();
        Serial.print("Sleep, lightSensorValue: ");
        Serial.print(lightSensorValue);
        Serial.print("\r\n");
      #endif
      
      delay(5000);
      return;
    }
  }

  #if (IS_DEBUG_MODE == 1)
    if (duration(lastLogTime) > 1000) {
      lastLogTime = millis();
      Serial.print("LightValue: ");
      Serial.print(lightSensorValue);
      
      Serial.print(" Coridor: ");
      Serial.print(coridorValues.lightLevel);
      Serial.print(", ");      
      Serial.print(coridorValues.sensor1);
      Serial.print(", ");
      Serial.print(coridorValues.sensor2);

      Serial.print(" Stairs: ");
      Serial.print(stairsValues.lightLevel);
      Serial.print(", ");        
      Serial.print(stairsValues.sensor1);
      Serial.print(", ");
      Serial.print(stairsValues.sensor2);
            
      Serial.print("\r\n");
    }
  #endif

  corridorHandler();
  stairsHandler();

  delay(30);
}


void corridorHandler() {
  const int valueMotionSensor1 = digitalRead(pinMotionSensorCorridor1);
  const int valueMotionSensor2 = digitalRead(pinMotionSensorCorridor2);
  const bool isMotionSensor = valueMotionSensor1 == HIGH || valueMotionSensor2 == HIGH;

  coridorValues.sensor1 = valueMotionSensor1;
  coridorValues.sensor2 = valueMotionSensor2;

  if (isMotionSensor) {
    coridorValues.timeOn = millis();
    coridorValues.state = STATE_UP;
  }

  switch (coridorValues.state) {
    case STATE_UP:
      if (coridorValues.lightLevel < 0xFF) {
        coridorValues.lightLevel = lightIncrement(coridorValues.lightLevel);
        analogWrite(pinSwitchCorridor, coridorValues.lightLevel);
      } else {
        coridorValues.state = STATE_WAIT;
      }
    break;
    case STATE_DOWN:
      if (coridorValues.lightLevel > 0) {
        coridorValues.lightLevel = lighDecrement(coridorValues.lightLevel);
        analogWrite(pinSwitchCorridor, coridorValues.lightLevel);
      } else {
        coridorValues.state = STATE_WAIT;  
      }
    break;
    default:
      // если свет включен более чем, начнем отключать его
      if (coridorValues.lightLevel > 0 && duration(coridorValues.timeOn) > CORRIDOR_LIGHT_ON_DELAY) {
        coridorValues.state = STATE_DOWN;
      }
  }
}

void stairsHandler() {
  const int valueMotionSensor1 = digitalRead(pinMotionSensorStairs1);
  const int valueMotionSensor2 = digitalRead(pinMotionSensorStairs2);
  const bool isMotionSensor = valueMotionSensor1 == HIGH || valueMotionSensor2 == HIGH;

  stairsValues.sensor1 = valueMotionSensor1;
  stairsValues.sensor2 = valueMotionSensor2;

  if (isMotionSensor) {
    stairsValues.timeOn = millis();
    stairsValues.state = STATE_UP;
  }

  switch (stairsValues.state) {
    case STATE_UP:
      if (stairsValues.lightLevel < 0xFF) {
        stairsValues.lightLevel = lightIncrement(stairsValues.lightLevel);
        analogWrite(pinSwitchStairs, stairsValues.lightLevel);
      } else {
        stairsValues.state = STATE_WAIT;
      }
    break;
    case STATE_DOWN:
      if (stairsValues.lightLevel > 0) {
        stairsValues.lightLevel = lighDecrement(stairsValues.lightLevel);
        analogWrite(pinSwitchStairs, stairsValues.lightLevel);
      } else {
        stairsValues.state = STATE_WAIT;  
      }
    break;
    default:
      // если свет включен более чем, начнем отключать его
      if (stairsValues.lightLevel > 0 && duration(stairsValues.timeOn) > STAIRS_LIGHT_ON_DELAY) {
        stairsValues.state = STATE_DOWN;
      }
  }
}

int lightIncrement(int currentValue) {
  int newValue = currentValue + lightLevelStep;
  if (newValue >= 0xFF ) {
    return 0xFF;
  }
  return newValue;
}

int lighDecrement(int currentValue) {
  int newValue = currentValue - lightLevelStep;
  if (newValue <= 0) {
    return 0;
  }
  return newValue;
}

unsigned long duration(unsigned long t) {
  unsigned long now = millis();
  if (now < t) {
    return (0xFFFFFFFF - t) + now;
  }

  return now - t;
}
