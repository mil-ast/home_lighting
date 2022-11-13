#define IS_DEBUG_MODE 0
#define CORRIDOR_LIGHT_ON_DELAY 8000

enum State {
  STATE_UP,
  STATE_DOWN,
  STATE_WAIT,
};

const int maxLightLevel = 300; // макс значение датчика света для работы (0 темно, 1023 светло)
const int lightPin = A3; // датчик света
const int pinSwitchCorridor = 5; // включатель коридора   PWM 5
const int pinSwitchStairs = 6; // включатель лестницы     PWM 6
const int pinMotionSensorCorridor1 = 7; // датчик движения коридора у лестницы
const int pinMotionSensorCorridor2 = 8; // датчик движения коридора у санузла
const int pinMotionSensorStairs1 = 9; // датчик движения лестницы верх
const int pinMotionSensorStairs2 = 10; // датчик движения лестницы низ

int lightLevelCorridor = 0; // текущий уровень освещения коридора
unsigned long corridorTimeOn = 0; // время включения света, чтобы отключить позжеё
State corridorState;

unsigned long lastLogTime = 0;

void setup() {
  #if (IS_DEBUG_MODE == 1)
    Serial.begin(9600);
  #endif

  corridorState = STATE_WAIT;

  //pinMode(pinSwitchCorridor, OUTPUT);
  //digitalWrite(pinSwitchCorridor, LOW);
  pinMode(pinSwitchCorridor, OUTPUT);
  analogWrite(pinSwitchCorridor, 0);

  pinMode(pinMotionSensorCorridor1, INPUT);
  pinMode(pinMotionSensorCorridor2, INPUT);
}

void loop() {
  // если светло и свет отключен, то ожидаем
  const int lightSensorValue = analogRead(lightPin);
  if (lightSensorValue > maxLightLevel) {
    if (corridorState == STATE_WAIT && lightLevelCorridor == 0) {
      #if (IS_DEBUG_MODE == 1)
        Serial.print("Sleep, lightSensorValue: ");
        Serial.print(lightSensorValue);
        Serial.print("\r\n");
      #endif
      
      delay(1000);
      return;
    }
  }

  const int valueMotionSensorCorridor1 = digitalRead(pinMotionSensorCorridor1);
  const int valueMotionSensorCorridor2 = digitalRead(pinMotionSensorCorridor2);
  const bool isMotionSensorCorridor = valueMotionSensorCorridor1 == HIGH || valueMotionSensorCorridor2 == HIGH;

  if (isMotionSensorCorridor) {
    corridorTimeOn = millis();
    corridorState = STATE_UP;
  }

  #if (IS_DEBUG_MODE == 1)
    if (duration(lastLogTime) > 1000) {
      lastLogTime = millis();
      
      Serial.print("motionSensorCorridor1: ");
      Serial.print(valueMotionSensorCorridor1);
      Serial.print(", motionSensorCorridor2: ");
      Serial.print(valueMotionSensorCorridor2);
      Serial.print(", corridorState: ");
      Serial.print(corridorState);
      Serial.print(", lightLevelCorridor: ");
      Serial.print(lightLevelCorridor);
      Serial.print(", lightSensorValue: ");
      Serial.print(lightSensorValue);
      Serial.print("\r\n");
    }
  #endif

  switch (corridorState) {
    case STATE_UP:
      if (lightLevelCorridor < 0xFF) {
        lightLevelCorridor++;
        analogWrite(pinSwitchCorridor, lightLevelCorridor);
      } else {
        corridorState = STATE_WAIT;
      }
    break;
    case STATE_DOWN:
      if (lightLevelCorridor > 0) {
        lightLevelCorridor--;
        analogWrite(pinSwitchCorridor, lightLevelCorridor);
      } else {
        corridorState = STATE_WAIT;  
      }
    break;
    default:
      // если свет включен более чем, начнем отключать его
      if (lightLevelCorridor > 0 && duration(corridorTimeOn) > CORRIDOR_LIGHT_ON_DELAY) {
        corridorState = STATE_DOWN;
      }
  }

  delay(10);
}

unsigned long duration(unsigned long t) {
  unsigned long now = millis();
  if (now < t) {
    return (0xFFFFFFFF - t) + now;
  }

  return now - t;
}
