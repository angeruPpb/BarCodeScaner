#include <Arduino.h>
#include "Utils.hpp"
#include <vector>
#include <ArduinoJson.h>
#include <HTTPClient.h>

Utils *ut = new Utils();

// ESP32AP::Config apConfig;

/************** TASK *******
 *   SETTING CONFIG
 *   @brief: Set the pins of the LEDs as OUTPUT
 *   @param: pool.ntp.org
 *   @return:
 **************************** */
char manualAttendanceType[10] = "entrance";
char dniData[20];
bool manualOverride = false;
bool attendanceToggle = true;
const char *prevAttendanceType = "";

// String wifi = "CASA2 2.4G"; String password = "isaias25++";
// String wifi = "CLARO_B253"; String password = "5wEs6DQpcp";
String wifi = "Pixel_7";
String password = "hehehe123";

void scanTask(void *pvParameters)
{
  Utils *ut = static_cast<Utils *>(pvParameters);
  while (1)
  {
    if (Serial2.available())
    {
      ut->onBuzzer(); 
      ut->onGreenLed();
      String dni = Serial2.readStringUntil('\n');
      dni = ut->cesarCipherDecode(dni, 3);
      if (dni.length() > 0)
      {
        //ut->onBuzzer();
        ut->addToQueueIfUnique(dni, manualAttendanceType);
        // apagar led verde
        digitalWrite(GREENLED, LOW);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void wifiReconnectTask(void *pvParameters) {
    Utils *ut = static_cast<Utils *>(pvParameters);
    bool wasConnected = false;
    while (1) {
        if (WiFi.status() != WL_CONNECTED) {
            //ut->offLeds();
            ut->connecToWifi(wifi.c_str(), password.c_str());
            wasConnected = false;
        } else {
            if (!wasConnected) {
                ut->offLeds();
                digitalWrite(YELLOWLED, HIGH); // Amarillo fijo
                wasConnected = true;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Revisa cada segundo
    }
}
void attendanceTypeTask(void *pvParameters) {
    Utils *ut = static_cast<Utils *>(pvParameters);
    while (1) {
        if (WiFi.status() == WL_CONNECTED) {
            digitalWrite(REDLED, LOW); // Apaga rojo
            digitalWrite(BLUELED, LOW);   // Apaga azul
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                if (timeinfo.tm_hour < 12) {
                    strcpy(manualAttendanceType, "entrance");
                    ut->onBlueLed();
                } else {
                    strcpy(manualAttendanceType, "exit");
                    ut->onRedLed();
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000)); // 30 minutos
    }
}

void setup()
{
  ut->onBuzzer();
  delay(2000);
  ut->onBuzzer();
  // Serial.begin(9600);
  // Serial2.begin(9600, SERIAL_8N1, 13, 14);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 13, 14);

  ut->setLeds(); // Iniciar Leds
  // Conexión WiFi inicial
  
  //ut->connecToWifi(wifi.c_str(), password.c_str());

  // Crea las tareas en paralelo
  xTaskCreate(wifiReconnectTask, "WifiReconnectTask", 4096, ut, 1, NULL);
  xTaskCreate(scanTask, "ScanTask", 4096, ut, 1, NULL);
  xTaskCreate(Utils::sendDataToServer, "SendTask", 8192, ut, 1, NULL);
  xTaskCreate(attendanceTypeTask, "AttendanceTypeTask", 4096, ut, 1, NULL);

  Serial.println("******** INIT SYSTEM ********");
  /*
  // if (ut->connecToWifi("CLARO_B253", "5wEs6DQpcp"))
  // if (ut->connecToWifi("YVONNE_EXT_5G", "PRESIOSARAV77"))
  if (ut->connecToWifi(wifi.c_str(), password.c_str()))
  {
    Serial.println("Conectado a la red WiFi");
  }*/
}

void loop()
{

}
/*
void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    ut->offLeds();
    if (!ut->connecToWifi(wifi.c_str(), password.c_str()))
    {
      ut->onRedLed();
    }
  }
  else
  {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Failed to obtain time");
      delay(1000);
      return;
    }

    const char *attendanceType;

    if (manualOverride)
    {
      attendanceType = attendanceToggle ? "entrance" : "exit";
    }
    else
    {
      attendanceType = (timeinfo.tm_hour < 12) ? "entrance" : "exit";
    }

    if (manualOverride && attendanceType != prevAttendanceType)
    {
      Serial.println("Modo manual activado");
      // ut->clearProcessedDnis();
      prevAttendanceType = attendanceType;
    }

    if (!manualOverride)
    {
      if (timeinfo.tm_hour < 12)
      {
        ut->lightsTomorrow();
      }
      else
      {
        ut->lightsAfternoon();
      }
    }

    if (digitalRead(BUTTON_PIN) == HIGH)
    {
      attendanceToggle = !attendanceToggle;
      manualOverride = true;

      if (attendanceToggle)
      {
        Serial.println("Manual override: entrance");
        ut->lightsTomorrow();
      }
      else
      {
        Serial.println("Manual override: exit");
        ut->lightsAfternoon();
      }
      // ut->clearProcessedDnis();
      delay(500);
    }

    if (Serial2.available())
    {
      ut->onBuzzer();
      // ut->greenFlagLedBlink();
      String dni = Serial2.readStringUntil('\n');

      Serial.println("BEFORE DNI: ");
      Serial.println(dni);
      dni = ut->cesarCipherDecode(dni, 3);

      Serial.println("AFTER DNI: ");
      Serial.println(dni);

      if (dni.length() > 0)
      {
        if (attendanceType == "entrance")
        {
          ut->onBlueLed();
        }
        else
        {
          ut->onRedLed();
        }
        ut->greenLedBlink();
        ut->addToQueueIfUnique(dni, attendanceType);
      }
    }
  }
}
*/