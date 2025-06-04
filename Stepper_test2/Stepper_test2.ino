#define STEP_PIN 2
#define DIR_PIN 3
#define ENABLE_PIN 4

bool isRunning = false; // Флаг выполнения программы
unsigned long stepsToMake = 0;
bool direction = HIGH; // HIGH - по часовой, LOW - против часовой

void setup() {
  Serial.begin(9600);
  Serial.println("Введите команду:");
  Serial.println("1 - запуск мотора");
  Serial.println("0 - остановка");
  Serial.println("---------------------");

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  
  digitalWrite(ENABLE_PIN, HIGH); // Двигатель выключен
  digitalWrite(DIR_PIN, direction); // Устанавливаем направление
}

void loop() {
  if (!isRunning && Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input == "1") {
      Serial.println("Введите количество шагов:");
      while (!Serial.available()) {} // Ждем ввода шагов
      String stepsInput = Serial.readStringUntil('\n');
      stepsToMake = stepsInput.toInt();
      
      Serial.println("Введите направление (cw/ccw):");
      while (!Serial.available()) {} // Ждем ввода направления
      String dirInput = Serial.readStringUntil('\n');
      dirInput.trim();
      
      if (dirInput == "cw") {
        direction = HIGH;
      } else if (dirInput == "ccw") {
        direction = LOW;
      } else {
        Serial.println("Неверное направление! Используется cw по умолчанию");
        direction = HIGH;
      }
      
      digitalWrite(DIR_PIN, direction);
      isRunning = true;
      Serial.println("Запуск мотора...");
      runMotorSequence();
    }
    else if (input == "0") {
      Serial.println("Мотор уже остановлен");
    }
  }
}

void runMotorSequence() {
  digitalWrite(ENABLE_PIN, LOW); // Включаем мотор
  
  for (unsigned long i = 0; i < stepsToMake; i++) {
    // Проверяем, не пришла ли команда '0' для остановки
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if (input == "0") {
        Serial.println("Досрочная остановка!");
        break; // Выход из цикла
      }
    }
    
    // Делаем шаг
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(500);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(500);
    
    // Вывод прогресса каждые 100 шагов
    if ((i + 1) % 100 == 0) {
      Serial.print("Выполнено шагов: ");
      Serial.print(i + 1);
      Serial.print(" из ");
      Serial.println(stepsToMake);
    }
  }

  // Завершение работы
  digitalWrite(ENABLE_PIN, HIGH); // Выключаем мотор
  isRunning = false;
  Serial.println("Готово! Введите '1' для нового запуска.");
  Serial.println("---------------------");
}
