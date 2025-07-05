#include <Arduino.h>
#include <DHT.h>

#define DHT_PIN 4
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

void setup()
{
  Serial.begin(115200);
  Serial.println("DUT Firmware Started. Waiting for sensor data...");
  dht.begin();
}

void loop()
{
  delay(1000);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Error: Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temp: 1");
  Serial.print(temperature, 1);
  Serial.print(" C, Hum: ");
  Serial.print(humidity, 1);
  Serial.println(" %");
}