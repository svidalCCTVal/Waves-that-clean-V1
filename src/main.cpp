#include <Arduino.h>
#include "Servo.h"

//linea de prueba de git-repo desde computador escritorio de Seba V. 
//línea de prueba de git-repo desde notebook de Seba V. 

// Pines para motores de desplazamiento
const int ENA = 3;   // Enable motor izquierdo
const int IN1 = 4;   // Dirección motor izquierdo (hacia adelante)
const int IN2 = 2;   // Dirección motor izquierdo (hacia atrás)
const int IN3 = 13;  // Dirección motor derecho (hacia adelante)
const int IN4 = A4;  // Dirección motor derecho (hacia atrás)
const int ENB = 5;   // Enable motor derecho

int prueba = 10;
float prueba2 = 20;
float prueba = 3.3; 
// Pines para motores brushless
const uint8_t VescOutputPinA = 11; // Motor brushless de abajo
const uint8_t VescOutputPinB = 6;  // Motor brushless de arriba

// Pines para sensores ultrasónicos
const int trigPin = 7; // Trigger compartido
const int echoA1 = 8;
const int echoA2 = 9;
const int echoB1 = A3;
const int echoB2 = A2;

// Umbral de detección en cm
const int umbralDeteccion = 11;

// Variables para distancia
float distanciaA1 = -1;
float distanciaA2 = -1;
float distanciaB1 = -1;
float distanciaB2 = -1;

// Velocidad del motor (PWM: 0-255)
const int velocidadMotorA = 205; // Velocidad del motor A
const int velocidadMotorB = 205; // Velocidad del motor B

// Variables para control de brushless
int speedsDerecha[3] = {1700, 1600, 1760}; 
int speedsIzquierda[3] = {1300, 1400, 1240};
int mode = 0;
unsigned long previousMillisBrushless = 0;
const unsigned long intervalBrushless = 500;

// Estados del robot y motores
bool desplazamientoActivo = false;
bool brushlessActivo = false;
bool estadoMotor = true;

// Variables para medición de sensores ultrasónicos
bool medirDistanciaActiva = false;
int sensorSeleccionado = -1; // 0: A1, 1: A2, 2: B1, 3: B2
const int sensoresEcho[] = {echoA1, echoA2, echoB1, echoB2};
const char* nombresSensores[] = {"A1", "A2", "B1", "B2"};

// Objetos Servo para los ESC
Servo escA;
Servo escB;

// Utility Functions
void moverMotorA(bool haciaDerecha);
void moverMotorB(bool haciaDerecha);
void detenerMotorA();
void detenerMotorB();
void controlarBrushless(bool haciaDerecha);
void detenerBrushless();
float medirDistancia(int trigPin, int echoPin);
void controlarDesplazamiento();


//------------------------------------------SETUP------------------------------------------//
void setup(){
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  escA.attach(VescOutputPinA);
  escB.attach(VescOutputPinB);
  escA.writeMicroseconds(1500);
  escB.writeMicroseconds(1500);

  pinMode(trigPin, OUTPUT);
  pinMode(echoA1, INPUT);
  pinMode(echoA2, INPUT);
  pinMode(echoB1, INPUT);
  pinMode(echoB2, INPUT);

  Serial.begin(9600);
  Serial.println("Comandos disponibles:");
  Serial.println("START DESP: Inicia desplazamiento.");
  Serial.println("START BLDC: Inicia brushless.");
  Serial.println("START ALL: Inicia desplazamiento y brushless.");
  Serial.println("STOP: Detiene todo.");
  Serial.println("MEASURE: Inicia medición de sensores ultrasónicos.");
  Serial.println("STOP MEASURE: Detiene medición de sensores.");
  Serial.println("SELECT <A1, A2, B1, B2>: Selecciona un sensor para medir distancia.");
}

//------------------------------------------LOOP-------------------------------------------//
void loop() {
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando.equalsIgnoreCase("START DESP")) {
      desplazamientoActivo = true;
      brushlessActivo = false;
    } else if (comando.equalsIgnoreCase("START BLDC")) {
      brushlessActivo = true;
      desplazamientoActivo = false;
    } else if (comando.equalsIgnoreCase("START ALL")) {
      desplazamientoActivo = true;
      brushlessActivo = true;
    } else if (comando.equalsIgnoreCase("STOP")) {
      desplazamientoActivo = false;
      brushlessActivo = false;
      detenerMotorA();
      detenerMotorB();
      detenerBrushless();
    } else if (comando.equalsIgnoreCase("MEASURE")) {
      medirDistanciaActiva = true;
      Serial.println("Medición activada.");
    } else if (comando.equalsIgnoreCase("STOP MEASURE")) {
      medirDistanciaActiva = false;
      Serial.println("Medición desactivada.");
    } else if (comando.startsWith("SELECT ")) {
      String sensorStr = comando.substring(7);
      sensorSeleccionado = -1;
      for (int i = 0; i < 4; i++) {
        if (sensorStr.equalsIgnoreCase(nombresSensores[i])) {
          sensorSeleccionado = i;
          Serial.print("Sensor seleccionado: ");
          Serial.println(nombresSensores[i]);
          break;
        }
      }
      if (sensorSeleccionado == -1) {
        Serial.println("Sensor no válido. Use SELECT A1, A2, B1 o B2.");
      }
    }
  }

  if (desplazamientoActivo) {
    controlarDesplazamiento();
  }

  if (brushlessActivo) {
    controlarBrushless(estadoMotor);
  }

  if (medirDistanciaActiva && sensorSeleccionado != -1) {
    float distancia = medirDistancia(trigPin, sensoresEcho[sensorSeleccionado]);
    Serial.print("Distancia (");
    Serial.print(nombresSensores[sensorSeleccionado]);
    Serial.print("): ");
    Serial.print(distancia);
    Serial.println(" cm");
    delay(500);
  }
}

// Función para controlar el desplazamiento         1 es a la izquierda 2 a la derecha, B es el lado donde esta la placa, A el lado opuesto
void controlarDesplazamiento() {
  distanciaA1 = medirDistancia(trigPin, echoA1);
  delay(15); // Pausa para evitar interferencias
  distanciaA2 = medirDistancia(trigPin, echoA2);
  delay(15); // Pausa para evitar interferencias
  distanciaB1 = medirDistancia(trigPin, echoB1);
  delay(15); // Pausa para evitar interferencias
  distanciaB2 = medirDistancia(trigPin, echoB2);
  delay(15); // Pausa para evitar interferencias

  // Control de movimiento basado en detección de fin de carrera
  if (((distanciaA1 > umbralDeteccion) && (distanciaA2 > umbralDeteccion)) || ((distanciaB1 > umbralDeteccion) && (distanciaB2 > umbralDeteccion))) {
    detenerMotorA();
    detenerMotorB();
  } else if (distanciaA1 >= umbralDeteccion || distanciaB1 >= umbralDeteccion) {
    estadoMotor = true;
    moverMotorA(estadoMotor);
    moverMotorB(estadoMotor);
  }
  if (distanciaA2 >= umbralDeteccion || distanciaB2 >= umbralDeteccion) {
    estadoMotor = false;
    moverMotorA(estadoMotor);
    moverMotorB(estadoMotor);
  } else {
    moverMotorA(estadoMotor);
    moverMotorB(estadoMotor);
  }
}

// Función para medir distancia
float medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracion = pulseIn(echoPin, HIGH, 30000);
  if (duracion == 0) {
    return -1;
  }
  return duracion * 0.034 / 2;
}

// Función para mover motores
void moverMotorA(bool haciaDerecha) {
  analogWrite(ENA, velocidadMotorA);
  if (haciaDerecha) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }
}

void moverMotorB(bool haciaDerecha) {
  analogWrite(ENB, velocidadMotorB);
  if (haciaDerecha) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
}

// Función para detener motores
void detenerMotorA() {
  analogWrite(ENA, 0);
}
void detenerMotorB() {
  analogWrite(ENB, 0);
}

// Función para controlar los brushless
void controlarBrushless(bool haciaDerecha) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisBrushless >= intervalBrushless) {
    previousMillisBrushless = currentMillis;
    mode = (mode + 1) % 3;
    escA.writeMicroseconds(haciaDerecha ? speedsDerecha[mode] : speedsIzquierda[mode]);
    escB.writeMicroseconds(haciaDerecha ? speedsDerecha[mode] : speedsIzquierda[mode]);
  }
}

// Función para detener brushless
void detenerBrushless() {
  escA.writeMicroseconds(1500);
  escB.writeMicroseconds(1500);
}