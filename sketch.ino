#include <ESP32Servo.h>

#define SERVO_PIN      15
#define LDR_IZQ_PIN    36
#define LDR_CEN_PIN    39
#define LDR_DER_PIN    34
#define FC37_PIN       35
#define T1592_PIN       4

#define POS_GALERA     0
#define POS_NEUTRO     90
#define POS_DERECHA    180
#define POS_IZQUIERDA  90

Servo servoTendedero;
int anguloActual = -1;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=================================================");
  Serial.println("  TENDEDERO INTELIGENTE IoT - ESP32 DevKit V1  ");
  Serial.println("=================================================");
  Serial.println("[OK] Puerto Serial iniciado a 115200 baudios.");

  pinMode(T1592_PIN, INPUT_PULLDOWN);
  pinMode(FC37_PIN, INPUT);
  pinMode(LDR_IZQ_PIN, INPUT);
  pinMode(LDR_CEN_PIN, INPUT);
  pinMode(LDR_DER_PIN, INPUT);
  Serial.println("[OK] Pines configurados.");

  ESP32PWM::allocateTimer(0);
  servoTendedero.setPeriodHertz(50);
  int canal = servoTendedero.attach(SERVO_PIN, 500, 2500);
  Serial.print("[OK] Servo attach, canal: "); Serial.println(canal);
  servoTendedero.write(POS_GALERA);
  anguloActual = POS_GALERA;

  Serial.println("Iniciando ciclo de monitoreo continuo...\n");
}

void loop() {
  int t1592_lluvia = digitalRead(T1592_PIN);
  int fc37_raw = analogRead(FC37_PIN);
  int fc37_humedad = map(fc37_raw, 0, 4095, 0, 100);

  int ldr_izq = analogRead(LDR_IZQ_PIN);
  int ldr_cen = analogRead(LDR_CEN_PIN);
  int ldr_der = analogRead(LDR_DER_PIN);

  int dLDR_IZQ = ldr_izq - ldr_cen;
  int dLDR_DER = ldr_der - ldr_cen;

  int anguloObjetivo = POS_GALERA;
  String estado = "";

  if (t1592_lluvia == HIGH || fc37_humedad > 70) {
    anguloObjetivo = POS_GALERA;
    estado = "ALERTA: Agua o humedad critica -> Resguardo en Galera (0 deg)";
  }
  else if (fc37_humedad > 40) {
    if (ldr_der > ldr_izq + 200) {
      anguloObjetivo = POS_DERECHA;
      estado = "Humedad moderada -> Sector Derecho (180 deg)";
    } else if (ldr_izq > ldr_der + 200) {
      anguloObjetivo = POS_IZQUIERDA;
      estado = "Humedad moderada -> Sector Izquierdo (90 deg)";
    } else {
      anguloObjetivo = POS_GALERA;
      estado = "Humedad moderada sin sol -> Resguardo (0 deg)";
    }
  }
  else {
    if (ldr_der > ldr_izq + 150) {
      anguloObjetivo = POS_DERECHA;
      estado = "Estado Seco -> Mayor luz DERECHA -> Giro a 180 deg";
    } else if (ldr_izq > ldr_der + 150) {
      anguloObjetivo = POS_IZQUIERDA;
      estado = "Estado Seco -> Mayor luz IZQUIERDA -> Giro a 90 deg";
    } else {
      anguloObjetivo = POS_NEUTRO;
      estado = "Estado Seco -> Luz equilibrada -> Sector Neutro (90 deg)";
    }
  }

  if (anguloObjetivo != anguloActual) {
    servoTendedero.write(anguloObjetivo);
    anguloActual = anguloObjetivo;
  }

  Serial.println("--------------------------------------------------");
  Serial.print("Lluvia T1592: "); Serial.print(t1592_lluvia == HIGH ? "DETECTADA (1)" : "NINGUNA (0)");
  Serial.print(" | Humedad FC-37: "); Serial.print(fc37_humedad); Serial.println("%");

  Serial.print("LDRs (ADC) -> Izq: "); Serial.print(ldr_izq);
  Serial.print(" | Cen: "); Serial.print(ldr_cen);
  Serial.print(" | Der: "); Serial.println(ldr_der);

  Serial.print("Diferenciales -> dIZQ: "); Serial.print(dLDR_IZQ);
  Serial.print(" | dDER: "); Serial.println(dLDR_DER);

  Serial.print("Angulo Servo: "); Serial.print(anguloActual); Serial.println(" deg");
  Serial.print("Estado: "); Serial.println(estado);

  delay(1000);
}