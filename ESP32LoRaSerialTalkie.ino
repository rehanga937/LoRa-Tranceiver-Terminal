#include <SPI.h>
#include <LoRa.h>

TaskHandle_t RxTask;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Talkie | ESP32 Serial");
  LoRa.setPins(5,4,2);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(41.7E3);
  LoRa.setCodingRate4(6);
  LoRa.setTxPower(2);
  LoRa.enableCrc();
  LoRa.setGain(6);

  xTaskCreatePinnedToCore(rx, "RxTask", 2000, NULL, 0, &RxTask, 0);//1st arg - func name, arg2 - just a name, arg3 - stack size - can print uxTaskGetStackHighWaterMark(NULL) to get remainder of stack size for fine tuning, arg6 - pointer to the TaskHandle_t object, arg7 - core (ESP32 has core 0 and 1, default tasks run on 1)
  //https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/system/freertos.html
  vTaskSuspend(RxTask);

  LoRa.onReceive(onReceive);
  LoRa.receive(); //radio will continue to receive and if packet received, will execute interrupt via DIO0 pin and execute the assigned onReceive (callback) function.
}

void loop() {
  while (Serial.available() == 0);
  String message = Serial.readString(); //CRLF is automatically taken when I press enter
  Serial.print("Me: "); Serial.print(message);
  LoRa.beginPacket();  LoRa.print(message);  LoRa.endPacket();
  LoRa.receive(); //required to make radio module go back to receive mode
}

void onReceive(int packetSize) { vTaskResume(RxTask); } //can't put anything too long and time consuming here or the ESP32 guru watchdog bla bla will panic and crash

void rx(void * pvParameters) {
while(1) { //has to loop fovever - https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/system/freertos.html
  while (LoRa.available()) {
    Serial.print((char)LoRa.read());
  }

  Serial.print("(RSSI "); Serial.print(LoRa.packetRssi());
  Serial.print(", SNR: "); Serial.print(LoRa.packetSnr());
  //Serial.print(", Packet Frequency Error: "); Serial.print(LoRa.packetFrequencyError());
  Serial.println(")");
  //Serial.println(uxTaskGetStackHighWaterMark(NULL));//for fine tuning stack size

  vTaskSuspend(RxTask);
} }
