#include <Servo.h>
#include <AccelStepper.h>

const byte JOY_X = A0;
const byte JOY_Y = A1;
const byte JOY_SW = 2;
const byte SENSOR = 3;
const byte IN1 = 4;
const byte IN2 = 5;
const byte IN3 = 6;
const byte IN4 = 7;
const byte LED = 8;
const byte SERVO_PIN = 9;

Servo servoMotor;
AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);

const int CENTRO = 512;
const int UMBRAL_INICIO = 90;
const int UMBRAL_PARADA = 55;
const float VELOCIDAD_MAX = 700.0;
const bool INVERTIR_X = false;
const bool INVERTIR_Y = false;
const byte SENSOR_ACTIVO = LOW;
const unsigned long REBOTE = 50;

bool bloqueado = false;
bool motorActivo = false;
bool botonAnterior = HIGH;
bool sensorAnterior = false;
int anguloActual = 90;
float velocidadActual = 0;
unsigned long ultimoBoton = 0;
unsigned long ultimoReporte = 0;

void setup() {
  Serial.begin(9600);

  pinMode(JOY_SW, INPUT_PULLUP);
  pinMode(SENSOR, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  servoMotor.attach(SERVO_PIN);
  servoMotor.write(90);

  stepper.setMaxSpeed(VELOCIDAD_MAX);
  stepper.setSpeed(0);

  botonAnterior = digitalRead(JOY_SW);
  sensorAnterior = digitalRead(SENSOR) == SENSOR_ACTIVO;

  Serial.println("LABORATORIO NO. 10");
  Serial.println("SISTEMA INICIADO");
}

void bloquear() {
  bloqueado = true;
  motorActivo = false;
  velocidadActual = 0;

  stepper.setSpeed(0);
  stepper.disableOutputs();
  digitalWrite(LED, HIGH);

  Serial.println("ALERTA: VIBRACION");
  Serial.println("SISTEMA BLOQUEADO");
}

void desbloquear() {
  bloqueado = false;

  stepper.enableOutputs();
  stepper.setSpeed(0);
  digitalWrite(LED, LOW);

  Serial.println("SISTEMA DESBLOQUEADO");
}

void controlarServo(int x) {
  if (INVERTIR_X) {
    x = 1023 - x;
  }

  int angulo = map(x, 0, 1023, 0, 180);
  angulo = constrain(angulo, 0, 180);

  if (abs(angulo - anguloActual) >= 2) {
    anguloActual = angulo;
    servoMotor.write(anguloActual);
  }
}

void controlarStepper(int y) {
  if (INVERTIR_Y) {
    y = 1023 - y;
  }

  int diferencia = y - CENTRO;
  int distancia = abs(diferencia);

  if (!motorActivo) {
    if (distancia >= UMBRAL_INICIO) {
      motorActivo = true;
    }
  } else {
    if (distancia <= UMBRAL_PARADA) {
      motorActivo = false;
    }
  }

  if (!motorActivo) {
    velocidadActual = 0;
    stepper.setSpeed(0);
    return;
  }

  float proporcion = (float)(distancia - UMBRAL_PARADA) / (511.0 - UMBRAL_PARADA);
  proporcion = constrain(proporcion, 0.0, 1.0);

  float velocidad = proporcion * VELOCIDAD_MAX;

  if (diferencia > 0) {
    velocidad = -velocidad;
  }

  velocidadActual = velocidad;
  stepper.setSpeed(velocidadActual);
}

void loop() {
  bool vibracion = digitalRead(SENSOR) == SENSOR_ACTIVO;

  if (vibracion && !sensorAnterior) {
    if (!bloqueado) {
      bloquear();
    }
  }
  sensorAnterior = vibracion;

  bool botonActual = digitalRead(JOY_SW);

  if (botonAnterior == HIGH && botonActual == LOW && millis() - ultimoBoton > REBOTE) {
    ultimoBoton = millis();

    if (bloqueado && !vibracion) {
      desbloquear();
    }
  }
  botonAnterior = botonActual;

  if (bloqueado) {
    return;
  }

  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  controlarServo(x);
  controlarStepper(y);
  stepper.runSpeed();

  if (millis() - ultimoReporte >= 500) {
    ultimoReporte = millis();

    Serial.print("X: ");
    Serial.print(x);
    Serial.print(" | Y: ");
    Serial.print(y);
    Serial.print(" | Servo: ");
    Serial.print(anguloActual);
    Serial.print(" | Velocidad: ");
    Serial.print(velocidadActual);
    Serial.print(" | Bloqueado: ");
    Serial.println(bloqueado);
  }
}
