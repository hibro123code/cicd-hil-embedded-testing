#include <Arduino.h>

#define SIM_PIN 4

float sim_temp = 25.0;
float sim_hum = 60.0;
bool simulate_fail = false;

void sendDHT11Signal()
{
  uint8_t data[5] = {0, 0, 0, 0, 0};

  int temp_int = (int)sim_temp;
  int hum_int = (int)sim_hum;

  data[0] = hum_int;                               // 8 bit integral hum
  data[1] = 0;                                     // 8 bit decimal hum
  data[2] = temp_int;                              // 8 bit integral temp
  data[3] = 0;                                     // 8 bit decimal temp
  data[4] = data[0] + data[1] + data[2] + data[3]; // 8 bit Checksum = hum + temp

  // --- Bắt đầu gửi tín hiệu ---
  // 1. Tín hiệu khởi động từ Master (DUT) 18ms_LOW -> 30us_HIGH
  // 2. Tín hiệu phản hồi từ DHT 80us_LOW -> 80us_HIGH
  pinMode(SIM_PIN, OUTPUT);
  digitalWrite(SIM_PIN, LOW);
  delayMicroseconds(80);
  digitalWrite(SIM_PIN, HIGH);
  delayMicroseconds(80);

  // 3. Bắt đầu gửi 40 bit dữ liệu
  for (int i = 0; i < 40; i++)
  {
    // Mỗi bit bắt đầu bằng 50us_LOW
    digitalWrite(SIM_PIN, LOW);
    delayMicroseconds(50);

    // Gửi bit '0' hoặc '1'
    if ((data[i / 8] >> (7 - (i % 8))) & 1)
    {
      // Bit '1' là 70us_HIGH
      digitalWrite(SIM_PIN, HIGH);
      delayMicroseconds(70);
    }
    else
    {
      // Bit '0' là 27us_HIGH
      digitalWrite(SIM_PIN, HIGH);
      delayMicroseconds(27);
      digitalWrite(SIM_PIN, LOW);
    }
  }

  // Chuyển chân INPUT
  pinMode(SIM_PIN, INPUT);
}

void setup()
{
  Serial.begin(115200);
  pinMode(SIM_PIN, INPUT); // Ban đầu là input, chờ DUT kéo xuống
  Serial.println("HIL Simulator Started. Waiting for commands...");
}

void loop()
{
  // Kiểm tra xem DUT có kéo xuống để bắt đầu đọc
  if (digitalRead(SIM_PIN) == LOW)
  {
    delay(1);
    if (digitalRead(SIM_PIN) == LOW)
    {
      if (!simulate_fail)
      {
        sendDHT11Signal();
      }
    }
  }

  // Xử lý lệnh từ script Python
  if (Serial.available() > 0)
  {
    String command = Serial.readStringUntil('\n');
    command.trim();
    Serial.print("Received command: ");
    Serial.println(command);

    if (command.startsWith("SIM:"))
    {
      command.remove(0, 4); // Bỏ đi "SIM:"
      int commaIndex = command.indexOf(',');
      if (commaIndex != -1)
      {
        String tempStr = command.substring(0, commaIndex);
        String humStr = command.substring(commaIndex + 1);
        sim_temp = tempStr.toFloat();
        sim_hum = humStr.toFloat();
        simulate_fail = false;
        Serial.println("Set to simulate Temp=" + String(sim_temp) + ", Hum=" + String(sim_hum));
      }
    }
    else if (command.equals("FAIL"))
    {
      simulate_fail = true;
      Serial.println("Set to simulate failure.");
    }
  }
}