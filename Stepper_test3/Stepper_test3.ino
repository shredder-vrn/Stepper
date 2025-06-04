/*
 * Управление тремя шаговыми моторами:
 * - Мотор 1: отвечает за нажатие
 * - Мотор 2: отвечает за подъём
 * - Мотор 3: отвечает за вращение
 */

// Конфигурация пинов для управления моторами
#define MOTOR1_STEP     2
#define MOTOR1_DIR      3
#define MOTOR1_ENABLE   4

#define MOTOR2_STEP     5
#define MOTOR2_DIR      6
#define MOTOR2_ENABLE   7

#define MOTOR3_STEP     8
#define MOTOR3_DIR      9
#define MOTOR3_ENABLE   10

// Состояние системы
enum SystemState {
  IDLE,
  RUNNING,
  EMERGENCY_STOP
};

struct MotorControl {
  bool isRunning = false;
  unsigned long targetSteps = 0;
  unsigned long completedSteps = 0;
  bool direction = HIGH;
  uint8_t activeMotor = 0;  // 0 - нет активного мотора
};

SystemState currentState = IDLE;
MotorControl motor;

void setup() {
  Serial.begin(9600);
  
  // Инициализация пинов моторов
  initMotorPins();
  
  // Отключение всех моторов при старте
  disableAllMotors();
  
  displayMainMenu();
}

void loop() {
  switch(currentState) {
    case IDLE:
      handleUserInput();
      break;
      
    case RUNNING:
      runMotorOperation();
      break;
      
    case EMERGENCY_STOP:
      // Обработка аварийного останова
      break;
  }
}

// Инициализация пинов моторов
void initMotorPins() {
  const uint8_t motorPins[3][3] = {
    {MOTOR1_STEP, MOTOR1_DIR, MOTOR1_ENABLE},
    {MOTOR2_STEP, MOTOR2_DIR, MOTOR2_ENABLE},
    {MOTOR3_STEP, MOTOR3_DIR, MOTOR3_ENABLE}
  };
  
  for (int i = 0; i < 3; i++) {
    pinMode(motorPins[i][0], OUTPUT);
    pinMode(motorPins[i][1], OUTPUT);
    pinMode(motorPins[i][2], OUTPUT);
  }
}

// Обработка пользовательского ввода
void handleUserInput() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input == "1") {
      selectMotorMenu();
    }
    else if (input == "0") {
      emergencyStopHandler();
    }
    else if (input == "help") {
      displayMainMenu();
    }
    else {
      Serial.println("Ошибка: неизвестная команда. Введите 'help' для помощи.");
    }
  }
}

// Меню выбора мотора
void selectMotorMenu() {
  Serial.println("Выберите мотор (1-3):\n1 - Нажатие\n2 - Подъём\n3 - Вращение");
  
  while (!Serial.available()) {}
  String input = Serial.readStringUntil('\n');
  input.trim();
  
  if (input.length() == 1 && input[0] >= '1' && input[0] <= '3') {
    motor.activeMotor = input.toInt();
    setupMotorParameters();
  } else {
    Serial.println("Ошибка: введите номер мотора от 1 до 3");
    displayMainMenu();
  }
}

// Настройка параметров мотора
void setupMotorParameters() {
  // Получение количества шагов
  Serial.println("Введите количество шагов (1-200000):");
  motor.targetSteps = readNumericalInput(1, 200000);
  
  // Получение направления вращения
  Serial.println("Выберите направление:\ncw - по часовой\nccw - против часовой");
  String dir = readStringInput();
  motor.direction = (dir == "ccw") ? LOW : HIGH;
  
  // Установка направления
  setMotorDirection(motor.activeMotor, motor.direction);
  
  // Подтверждение параметров
  Serial.print("Мотор ");
  Serial.print(motor.activeMotor);
  Serial.print(" | Шагов: ");
  Serial.print(motor.targetSteps);
  Serial.print(" | Направление: ");
  Serial.println(motor.direction == HIGH ? "cw" : "ccw");
  
  currentState = RUNNING;
  startMotorOperation();
}

// Запуск операции мотора
void startMotorOperation() {
  enableMotor(motor.activeMotor);
  motor.completedSteps = 0;
  motor.isRunning = true;
  
  Serial.println("Запуск операции...");
  Serial.println("Введите '0' для аварийной остановки");
}

// Основной цикл работы мотора
void runMotorOperation() {
  if (motor.completedSteps < motor.targetSteps) {
    if (checkEmergencyStop()) return;
    
    makeMotorStep(motor.activeMotor);
    motor.completedSteps++;
    
    // Вывод прогресса каждые 100 шагов
    if (motor.completedSteps % 100 == 0) {
      printOperationProgress();
    }
    
    delayMicroseconds(500);  // Пауза между шагами
  } else {
    completeOperation();
  }
}

// Завершение операции
void completeOperation() {
  disableMotor(motor.activeMotor);
  
  Serial.print("\nОперация завершена! Выполнено ");
  Serial.print(motor.completedSteps);
  Serial.println(" шагов.");
  
  resetMotorState();
  currentState = IDLE;
  displayMainMenu();
}

// Аварийная остановка
void emergencyStopHandler() {
  if (currentState == RUNNING) {
    Serial.println("!!! АВАРИЙНАЯ ОСТАНОВКА !!!");
  }
  
  disableAllMotors();
  resetMotorState();
  currentState = IDLE;
  displayMainMenu();
}

// Вспомогательные функции
void setMotorDirection(uint8_t motorNum, bool dir) {
  switch(motorNum) {
    case 1: digitalWrite(MOTOR1_DIR, dir); break;
    case 2: digitalWrite(MOTOR2_DIR, dir); break;
    case 3: digitalWrite(MOTOR3_DIR, dir); break;
  }
}

void makeMotorStep(uint8_t motorNum) {
  uint8_t stepPin;
  switch(motorNum) {
    case 1: stepPin = MOTOR1_STEP; break;
    case 2: stepPin = MOTOR2_STEP; break;
    case 3: stepPin = MOTOR3_STEP; break;
  }
  
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(500);
  digitalWrite(stepPin, LOW);
}

void enableMotor(uint8_t motorNum) {
  switch(motorNum) {
    case 1: digitalWrite(MOTOR1_ENABLE, LOW); break;
    case 2: digitalWrite(MOTOR2_ENABLE, LOW); break;
    case 3: digitalWrite(MOTOR3_ENABLE, LOW); break;
  }
}

void disableMotor(uint8_t motorNum) {
  switch(motorNum) {
    case 1: digitalWrite(MOTOR1_ENABLE, HIGH); break;
    case 2: digitalWrite(MOTOR2_ENABLE, HIGH); break;
    case 3: digitalWrite(MOTOR3_ENABLE, HIGH); break;
  }
}

void disableAllMotors() {
  digitalWrite(MOTOR1_ENABLE, HIGH);
  digitalWrite(MOTOR2_ENABLE, HIGH);
  digitalWrite(MOTOR3_ENABLE, HIGH);
}

void resetMotorState() {
  motor.isRunning = false;
  motor.activeMotor = 0;
  motor.targetSteps = 0;
  motor.completedSteps = 0;
}

bool checkEmergencyStop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    if (input.trim() == "0") {
      emergencyStopHandler();
      return true;
    }
  }
  return false;
}

void printOperationProgress() {
  float progress = (float)motor.completedSteps / motor.targetSteps * 100;
  Serial.print("Прогресс: ");
  Serial.print(motor.completedSteps);
  Serial.print("/");
  Serial.print(motor.targetSteps);
  Serial.print(" (");
  Serial.print(progress, 1);
  Serial.println("%)");
}

unsigned long readNumericalInput(unsigned long minVal, unsigned long maxVal) {
  while (true) {
    while (!Serial.available()) {}
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    unsigned long value = input.toInt();
    if (value >= minVal && value <= maxVal) {
      return value;
    }
    Serial.print("Ошибка: введите число от ");
    Serial.print(minVal);
    Serial.print(" до ");
    Serial.print(maxVal);
    Serial.println(":");
  }
}

String readStringInput() {
  while (!Serial.available()) {}
  String input = Serial.readStringUntil('\n');
  input.trim();
  return input;
}

void displayMainMenu() {
  Serial.println("\n=== УПРАВЛЕНИЕ ШАГОВЫМИ МОТОРАМИ ===");
  Serial.println("1 - Запустить мотор");
  Serial.println("0 - Аварийная остановка");
  Serial.println("help - Показать это меню");
  Serial.println("================================");
  Serial.println("Ожидание команды...");
}
