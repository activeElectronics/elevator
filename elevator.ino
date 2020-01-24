// Versión V0.3 by Emanuel Aguirre

#include <ButtonDebounce.h>

#define REVERSE_MOVEMENT

#define DEBOUNCE 250
#define DOOR_OPEN_INTERVAL 2000

#define PUSH_BUTTON_PB 2
#define PUSH_BUTTON_1  3
#define PUSH_BUTTON_2  4
#define PUSH_BUTTON_3  5

#define SENSOR_POS_PB  6
#define SENSOR_POS_1  7
#define SENSOR_POS_2  8
#define SENSOR_POS_3  9

#define L298N_IN3 10
#define L298N_IN4 11

int storeButtonState[SENSOR_POS_3 + 2];

// -- GLOBAL CONTROL VARIABLES ----------------------------------------------------------
int pisoActual = -1;
int requests[] = { -1, -1, -1, -1}; // Requests queue R1, R2, R3, R4

bool habilitado = true; // Motor will move only if habilitado == true;

void setup() {
  Serial.begin(115200);
  pinMode(PUSH_BUTTON_PB, INPUT_PULLUP);
  pinMode(PUSH_BUTTON_1, INPUT_PULLUP);
  pinMode(PUSH_BUTTON_2, INPUT_PULLUP);
  pinMode(PUSH_BUTTON_3, INPUT_PULLUP);

  pinMode(SENSOR_POS_PB, INPUT_PULLUP);
  pinMode(SENSOR_POS_1, INPUT_PULLUP);
  pinMode(SENSOR_POS_2, INPUT_PULLUP);
  pinMode(SENSOR_POS_3, INPUT_PULLUP);

  pinMode (L298N_IN3, OUTPUT);
  pinMode (L298N_IN4, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  // -- Inicialización del vector
  for (int i = 0; i < (sizeof(storeButtonState) / sizeof(int)); i++) {
    storeButtonState[i] = 0;
  }

  // -- UBICAR EL CARRO -------------------------------------------------------------------
  while (digitalRead(SENSOR_POS_PB)) {
    Serial.println("[RUNNING]: Carro bajando hasta PB!");
    pisoActual = 0; // Comenzar en el piso 0 (PB)
  }
  motorIdle ();
}

void loop() {
  update_buttons();

  if (is_requests_empty()) {
    print_requests();
    delay(200);
  } else {
    // Hay requests pendientes entonces:
    print_requests();
    if (pisoActual < requests[0]) {
      // Si el piso actual es menor al primer request encontrado, debo subir el carro
      motorUp ();
      if (false/*>pisoActual && < requests[0]*/) {
        //[Pendiente de implementar] Verificar si en la cola, hay pedido algun piso 
        //                           entre el solicitado y el actual
        //Si se ecnuentra dicho piso, cambiar el orden de la cola, para pasar por el piso
        //intermedio primero
      }
    } else if (pisoActual > requests[0]) {
      //Se solicitó un piso por debajo del actual. Bajando
      motorDown ();

    } else if (pisoActual == requests[0]) {
      // El carro arribó al piso solicitado. Detenerlo
      abrir_puerta();
      //requests[0] = -1;
    }
    delay(200);
  }
}
// -- CONTROLES DE MOTOR -------------------------------------------------------------------
void motorUp () {
  if (habilitado) {
    Serial.println("[DEBUG]: Moving UP!");
    digitalWrite(LED_BUILTIN, HIGH);
#ifdef REVERSE_MOVEMENT
    digitalWrite(L298N_IN3, LOW);
    digitalWrite(L298N_IN4, HIGH);
#else
    digitalWrite(L298N_IN3, HIGH);
    digitalWrite(L298N_IN4, LOW);
#endif
  } else {
    motorIdle ();
  }
}

void motorDown () {
  if (habilitado) {
    Serial.println("[DEBUG]: Moving DOWN!");
    digitalWrite(LED_BUILTIN, HIGH);
#ifdef REVERSE_MOVEMENT
    digitalWrite(L298N_IN3, HIGH);
    digitalWrite(L298N_IN4, LOW);
#else
    digitalWrite(L298N_IN3, LOW);
    digitalWrite(L298N_IN4, HIGH);
#endif
  } else {
    motorIdle ();
  }
}

void motorIdle () {
  Serial.println("[DEBUG]: STOP!");
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(L298N_IN3, LOW);
  digitalWrite(L298N_IN4, LOW);
}

// -- CONTROLES DE MOTOR -------------------------------------------------------------------

// -- ABRIR PUERTA -------------------------------------------------------------------------
void abrir_puerta() {
  //Avanzar la fila cuando se llegó al piso seleccionado...
  habilitado = false;
  motorIdle ();
  int i;
  for (i = 0; i < (sizeof(requests) / sizeof(int)) - 1; i++) {
       requests[i] = requests[i + 1];
  }
  // Olvidar el valor de la ultima request en requests[]
  i = (sizeof(requests) / sizeof(int)) - 1;
  requests[i] = -1;

  Serial.println("ABRIENDO PUERTA!!");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);
  digitalWrite(LED_BUILTIN, LOW);
  habilitado = true;
  Serial.println("CERRANDO");

}
// -- ABRIR PUERTA -------------------------------------------------------------------------

// -- CONTROL DE LAS ENTRADAS ---------------------------------------------------------------
void update_buttons() {
  if (button_pressed(PUSH_BUTTON_PB)) {
    Serial.println("Se presionó el PB");
    agregar_request_a_la_cola(PUSH_BUTTON_PB - 2);
  }
  if (button_pressed(PUSH_BUTTON_1)) {
    Serial.println("Se presionó el 1");
    agregar_request_a_la_cola(PUSH_BUTTON_1 - 2);
  }
  if (button_pressed(PUSH_BUTTON_2)) {
    Serial.println("Se presionó el 2");
    agregar_request_a_la_cola(PUSH_BUTTON_2 - 2);
  }
  if (button_pressed(PUSH_BUTTON_3)) {
    Serial.println("Se presionó el 3");
    agregar_request_a_la_cola(PUSH_BUTTON_3 - 2);
  }
  if (button_pressed(SENSOR_POS_PB)) {
    Serial.println("Se presionó el SENSOR_POS_PB");
    pisoActual = 0;
    //eliminar_de_la_cola(pisoActual);
  }
  if (button_pressed(SENSOR_POS_1)) {
    Serial.println("Se presionó el SENSOR_POS_1");
    pisoActual = 1;
  }
  if (button_pressed(SENSOR_POS_2)) {
    Serial.println("Se presionó el SENSOR_POS_2");
    pisoActual = 2;
  }
  if (button_pressed(SENSOR_POS_3)) {
    Serial.println("Se presionó el SENSOR_POS_3");
    pisoActual = 3;
  }
}
// -- CONTROL DE LAS ENTRADAS ---------------------------------------------------------------

// -- MANEJO DE LA COLA ---------------------------------------------------------------------
void agregar_request_a_la_cola(int index) {
  for (int i = 0; i < sizeof(requests) / sizeof(int); i++) {
    if (requests[i] == index) {
      // El piso pedido ya estaba en la cola
      break;
    }
    if (requests[i] == -1 ) {
      // Agregar una request en la primer posición vacía
      requests[i] = index;
      break;
    }
  }
}
// -- MANEJO DE LA COLA ---------------------------------------------------------------------

// -- ESTADO DE LA COLA ---------------------------------------------------------------------
bool is_requests_empty() {
  // Analiza el vector requests[] en busca de '-1' indicando que la cola de tareas está vacía
  // Si encuentra algun valor distinto de '-1' cortó y devuelve false
  bool empty = true;
  for (int i = 0; i < (sizeof(requests) / sizeof(int)); i++) {
    if (requests[i] != -1) {
      // Si hay algun request pendiente
      Serial.println("Hay requests pendientes");
      empty = false;
      break;
    }
  }
  if (empty) Serial.println("No se encontraron requests pendientes");
  return empty;
}
// -- ESTADO DE LA COLA ---------------------------------------------------------------------

// -- DEBUG --
void print_requests() {
  Serial.print("requests(): ");
  for (int i = 0; i < sizeof(requests) / sizeof(int); i++) {
    Serial.print(requests[i]);
    Serial.print("\t| ");
  }
  Serial.println();
}
// -- DEBUG --

// -- DEBOUNCE ------------------------------------------------------------------------------
bool button_pressed (int btn) {
  bool ans = false;
  if (digitalRead(btn) == LOW && storeButtonState[btn] == 0) {
    delay(DEBOUNCE);  // The higher the Delay the less chance of bouncing
    storeButtonState[btn] = 1;
    ans = true;
  } else {
    if (storeButtonState[btn] == 1 && digitalRead(btn) == HIGH) {
      storeButtonState[btn] = 0;
    }
  }
  return ans;
}
// -- DEBOUNCE ------------------------------------------------------------------------------
