// Конфигурация пинов для трех моторов
#define PRESS_MOTOR_STEP 2    // Мотор нажимания
#define PRESS_MOTOR_DIR 3
#define PRESS_MOTOR_ENABLE 4

#define LIFT_MOTOR_STEP 5     // Мотор подъёма
#define LIFT_MOTOR_DIR 6
#define LIFT_MOTOR_ENABLE 7

#define ROTATE_MOTOR_STEP 8   // Мотор вращения
#define ROTATE_MOTOR_DIR 9
#define ROTATE_MOTOR_ENABLE 10

// Глобальные переменные
bool isRunning = false;
unsigned long stepsToMake = 0;
bool direction = HIGH;
int activeMotor = 0; // 0 - нет, 1-3 - номера моторов

void setup() {
  Serial.begin(9600);
  
  // Настройка пинов для всех моторов
  pinMode(PRESS_MOTOR_STEP, OUTPUT);
  pinMode(PRESS_MOTOR_DIR, OUTPUT);
  pinMode(PRESS_MOTOR_ENABLE, OUTPUT);
  digitalWrite(PRESS_MOTOR_ENABLE, HIGH);
  
  pinMode(LIFT_MOTOR_STEP, OUTPUT);
  pinMode(LIFT_MOTOR_DIR, OUTPUT);
  pinMode(LIFT_MOTOR_ENABLE, OUTPUT);
  digitalWrite(LIFT_MOTOR_ENABLE, HIGH);

  pinMode(ROTATE_MOTOR_STEP, OUTPUT);
  pinMode(ROTATE_MOTOR_DIR, OUTPUT);
  pinMode(ROTATE_MOTOR_ENABLE, OUTPUT);
  digitalWrite(ROTATE_MOTOR_ENABLE, HIGH);

  printMenu();
}

void loop() {
  if (!isRunning) {
    if (Serial.available() > 0) {
      handleUserInput();
    }
  }
}

void handleUserInput() {
  String input = Serial.readStringUntil('\n');
  input.trim();
  
  if (input == "1") {
    selectMotor();
  }
  else if (input == "0") {
    emergencyStop();
  }
  else if (input == "help") {
    printMenu();
  }
  else {
    Serial.println("Неизвестная команда. Введите 'help' для списка команд.");
  }
}

void selectMotor() {
  Serial.println("Выберите мотор:");
  Serial.println("1 - Мотор нажимания");
  Serial.println("2 - Мотор подъёма");
  Serial.println("3 - Мотор вращения");
  
  while (!Serial.available()) {}
  String motorInput = Serial.readStringUntil('\n');
  motorInput.trim();
  
  if (motorInput == "1" || motorInput == "2" || motorInput == "3") {
    activeMotor = motorInput.toInt();
    setupMotorParameters();
  } else {
    Serial.println("Ошибка: введите 1, 2 или 3");
    printMenu();
  }
}

void setupMotorParameters() {
  // Запрос количества шагов
  Serial.println("Введите количество шагов:");
  while (!Serial.available()) {}
  String stepsInput = Serial.readStringUntil('\n');
  stepsToMake = stepsInput.toInt();
  
  // Запрос направления
  Serial.println("Введите направление:");
  Serial.println("cw - по часовой стрелке");
  Serial.println("ccw - против часовой стрелки");
  while (!Serial.available()) {}
  String dirInput = Serial.readStringUntil('\n');
  dirInput.trim();
  
  direction = (dirInput == "ccw") ? LOW : HIGH;
  
  // Установка направления для выбранного мотора
  switch(activeMotor) {
    case 1: 
      digitalWrite(PRESS_MOTOR_DIR, direction); 
      Serial.println("Мотор нажимания настроен");
      break;
    case 2: 
      digitalWrite(LIFT_MOTOR_DIR, direction);
      Serial.println("Мотор подъёма настроен");
      break;
    case 3: 
      digitalWrite(ROTATE_MOTOR_DIR, direction);
      Serial.println("Мотор вращения настроен");
      break;
  }
  
  startMotor();
}

void startMotor() {
  isRunning = true;
  Serial.print("Запуск мотора: ");
  switch(activeMotor) {
    case 1: Serial.println("нажимание"); break;
    case 2: Serial.println("подъём"); break;
    case 3: Serial.println("вращение"); break;
  }
  runMotorSequence();
}

void runMotorSequence() {
  // Активация выбранного мотора
  switch(activeMotor) {
    case 1: digitalWrite(PRESS_MOTOR_ENABLE, LOW); break;
    case 2: digitalWrite(LIFT_MOTOR_ENABLE, LOW); break;
    case 3: digitalWrite(ROTATE_MOTOR_ENABLE, LOW); break;
  }
  
  for (unsigned long i = 0; i < stepsToMake; i++) {
    if (checkForEmergencyStop()) {
      break;
    }
    
    // Генерация шага
    makeStep();
    
    // Вывод прогресса
    if ((i + 1) % 100 == 0) {
      printProgress(i + 1);
    }
  }

  completeOperation();
}

bool checkForEmergencyStop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input == "0") {
      Serial.println("Досрочная остановка!");
      return true;
    }
  }
  return false;
}

void makeStep() {
  switch(activeMotor) {
    case 1: 
      digitalWrite(PRESS_MOTOR_STEP, HIGH);
      delayMicroseconds(500);
      digitalWrite(PRESS_MOTOR_STEP, LOW);
      break;
    case 2:
      digitalWrite(LIFT_MOTOR_STEP, HIGH);
      delayMicroseconds(500);
      digitalWrite(LIFT_MOTOR_STEP, LOW);
      break;
    case 3:
      digitalWrite(ROTATE_MOTOR_STEP, HIGH);
      delayMicroseconds(500);
      digitalWrite(ROTATE_MOTOR_STEP, LOW);
      break;
  }
  delayMicroseconds(500);
}

void printProgress(unsigned long currentStep) {
  Serial.print("Выполнено: ");
  Serial.print(currentStep);
  Serial.print("/");
  Serial.print(stepsToMake);
  Serial.print(" шагов (");
  Serial.print((currentStep * 100) / stepsToMake);
  Serial.println("%)");
}

void completeOperation() {
  stopAllMotors();
  Serial.println("Операция завершена!");
  resetSystemState();
  printMenu();
}

void emergencyStop() {
  stopAllMotors();
  if (isRunning) {
    Serial.println("Аварийная остановка выполнена!");
  } else {
    Serial.println("Все моторы уже остановлены");
  }
  resetSystemState();
  printMenu();
}

void stopAllMotors() {
  digitalWrite(PRESS_MOTOR_ENABLE, HIGH);
  digitalWrite(LIFT_MOTOR_ENABLE, HIGH);
  digitalWrite(ROTATE_MOTOR_ENABLE, HIGH);
}

void resetSystemState() {
  isRunning = false;
  activeMotor = 0;
  stepsToMake = 0;
}

void printMenu() {
  Serial.println("\n=================================");
  Serial.println("  СИСТЕМА УПРАВЛЕНИЯ МОТОРАМИ  ");
  Serial.println("=================================");
  Serial.println("1 - Запуск мотора");
  Serial.println("  1 - Мотор нажимания");
  Serial.println("  2 - Мотор подъёма");
  Serial.println("  3 - Мотор вращения");
  Serial.println("0 - Аварийная остановка");
  Serial.println("help - Показать это меню");
  Serial.println("=================================");
  Serial.println("Ожидание команды...");
}
