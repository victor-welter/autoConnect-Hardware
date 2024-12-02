#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Callback para conexão e desconexão
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("Cliente conectado");
  }

  void onDisconnect(BLEServer* pServer) {
    Serial.println("Cliente desconectado");
  }
};

const int sensorPin = 34;
const float maxInputVoltage = 12.0;  // Voltagem máxima do divisor de tensão
const float referenceVoltage = 3.3;  // Voltagem de referência do ESP32

// Quantidade de leituras
const int numReadings = 20;

void setup() {
  Serial.begin(115200);

  BLEDevice::init("ESP32-BLE");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->setValue("0");
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
}

void loop() {
  int readings[numReadings];  // Array para armazenar as leituras

  // Realiza várias leituras
  for (int i = 0; i < numReadings; i++) {
    readings[i] = analogRead(sensorPin);
    delay(10);
  }

  // Calcula a moda
  int modeValue = findMode(readings, numReadings);

  // Converte a moda para milivolts
  float voltage = (modeValue / 4095.0) * referenceVoltage;  // Em volts
  float realVoltage = voltage * (maxInputVoltage / referenceVoltage);  // Ajuste com divisor de tensão
  float realMillivolts = realVoltage * 1000;  // Converte para milivolts

  Serial.print("Tensão medida (moda): ");
  Serial.print(realMillivolts);
  Serial.println(" mV");

  // Atualiza o valor da característica BLE
  pCharacteristic->setValue(String(realMillivolts).c_str());
  pCharacteristic->notify();  // Envia uma notificação para o cliente

  delay(1000);
}


// Função para calcular a moda
int findMode(int data[], int size) {
  int maxValue = 0, maxCount = 0;

  for (int i = 0; i < size; ++i) {
    int count = 0;

    for (int j = 0; j < size; ++j) {
      if (data[j] == data[i]) {
        ++count;
      }
    }

    if (count > maxCount) {
      maxCount = count;
      maxValue = data[i];
    }
  }

  return maxValue;
}

