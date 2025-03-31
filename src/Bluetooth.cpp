#include "Wireless_mgmt.h"
#include "Arduino.h"
#include <BluetoothSerial.h>

// ==========================================
// Bluetooth Classic
// ==========================================

BluetoothSerial SerialBT; 

// 藍牙回調函式指針
void (*_btCallback)(String) = NULL;

/**
 * 設定藍牙連接參數
 * @param btName 藍牙顯示名稱
 */
void BT_setup(const char* btName, bool silentMode) {
  SerialBT.begin(btName);
  
  if (!silentMode) {
    Serial.println("--------------------------------");
    Serial.println("藍牙設定:");
    Serial.print("- 設備名稱: ");
    Serial.println(btName);
    
    if (_btCallback != NULL) {
      Serial.println("- 回調函式已設定");
    }
    Serial.println("--------------------------------");
  }
}

/**
 * 掃描並連接指定名稱的藍牙設備(一步到位)
 * @param name 要連接的設備名稱
 * @param scanDuration 掃描時間(秒)
 * @param partialMatch 是否允許部分名稱匹配
 * @param maxAttempts 最大嘗試次數
 * @param silentMode 是否安靜模式
 * @return 是否成功連接
 */
bool BT_master_connect(const String &name, uint32_t scanDuration, bool partialMatch, int maxAttempts, bool silentMode) {
  for (int attempt = 1; attempt <= maxAttempts; attempt++) {
    if (!silentMode && maxAttempts > 1) {
      Serial.print("嘗試 ");
      Serial.print(attempt);
      Serial.print("/");
      Serial.print(maxAttempts);
      Serial.println("...");
    }
    
    // 開始掃描
    if (!silentMode) {
      Serial.println("--------------------------------");
      Serial.println("開始藍牙掃描...");
    }
    
    // 清空之前掃描結果
    bool foundTargetDevice = false;
    String targetAddress = "";
    
    // 啟動掃描
    bool scanStarted = SerialBT.discoverAsync([&](BTAdvertisedDevice* device) {
      // 檢查設備名稱是否匹配
      String deviceName = device->getName().c_str();
      bool nameMatches = false;
      
      if (deviceName.length() > 0) {
        if (partialMatch) {
          nameMatches = (deviceName.indexOf(name) >= 0);
        } else {
          nameMatches = deviceName.equals(name);
        }
      }
      
      // 找到匹配設備
      if (nameMatches) {
        foundTargetDevice = true;
        targetAddress = device->getAddress().toString().c_str();
        
        if (!silentMode) {
          Serial.print("找到目標設備: ");
          Serial.print(deviceName);
          Serial.print(" (");
          Serial.print(targetAddress);
          Serial.print("), RSSI: ");
          Serial.println(device->getRSSI());
        }
      } else if (!silentMode) {
        Serial.print("發現設備: ");
        Serial.print(deviceName.length() > 0 ? deviceName : "(無名稱)");
        Serial.print(" (");
        Serial.print(device->getAddress().toString().c_str());
        Serial.println(")");
      }
    });
    
    if (!scanStarted) {
      if (!silentMode) Serial.println("啟動掃描失敗");
      return false;
    }
    
    // 等待掃描結束
    delay(scanDuration * 1000);
    SerialBT.discoverAsyncStop();
    
    if (!silentMode) {
      Serial.println("藍牙掃描已停止");
    }
    
    // 如果找到目標設備，嘗試連接
    if (foundTargetDevice) {
      if (!silentMode) {
        Serial.println("--------------------------------");
        Serial.print("正在連接到設備: ");
        Serial.print(name);
        Serial.print(" (");
        Serial.print(targetAddress);
        Serial.println(")");
      }
      
      bool connected = SerialBT.connect(targetAddress.c_str());
      
      if (connected) {
        if (!silentMode) {
          Serial.println("連接成功!");
          Serial.println("--------------------------------");
        }
        return true;
      } else if (!silentMode) {
        Serial.println("連接失敗");
        Serial.println("--------------------------------");
      }
    } else if (!silentMode) {
      Serial.print("找不到名稱");
      Serial.print(partialMatch ? "包含" : "為");
      Serial.print(" '");
      Serial.print(name);
      Serial.println("' 的設備");
    }
    
    // 如果不是最後一次嘗試，則等待一段時間
    if (attempt < maxAttempts) {
      delay(1000);  // 等待1秒後再試
    }
  }
  
  if (!silentMode && maxAttempts > 1) {
    Serial.print("在 ");
    Serial.print(maxAttempts);
    Serial.print(" 次嘗試後無法連接到設備 '");
    Serial.print(name);
    Serial.println("'");
  }
  
  return false;
}

/**
 * 設定藍牙訊息回調函式
 * @param callback 回調函式指針 void(String)
 */
void BT_setCallback(void (*callback)(String), bool silentMode) {
  _btCallback = callback;
  if (!silentMode) {
    Serial.println("藍牙回調函式已設定");
  }
}

/**
 * 藍牙主迴圈處理
 * 此函式應在主迴圈中定期呼叫
 */
void BT_loop() {
  if (SerialBT.available()) {
    String message = SerialBT.readString();
    message.trim();
    _btCallback(message);
  }
}

/**
 * 透過藍牙發送訊息
 * @param message 要發送的訊息
 * @return 是否成功發送
 */
bool BT_sendMessage(const String &message, bool ln, bool silentMode) {
  if (SerialBT.connected()) {
    if (ln) {
      SerialBT.println(message);
    } else {
      SerialBT.print(message);
    }
    return true;
  } else {
    if (!silentMode) {
      Serial.println("藍牙未連接，無法發送訊息");
    }
    return false;
  }
}

/**
 * 檢查藍牙連接狀態
 * @return 是否已連接
 */
bool BT_checkStatus() {
  return SerialBT.connected();
}

// ==========================================
// Bluetooth Low Energy
// ==========================================

// BLE 伺服器相關變數
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// 訊息回調函式
BLECallbackFunction _bleCallback = NULL;

// 定義回調類別處理連線狀態
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE 用戶已連接");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("BLE 用戶已斷開");
    }
};

// 定義特徵回調處理收到的數據
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      if (_bleCallback != NULL) {
        std::string value = pCharacteristic->getValue();
        String message = String(value.c_str());
        message.trim();
        _bleCallback(message);
      }
    }
};

/**
 * 設定低功耗藍牙連接參數
 * @param bleName 藍牙顯示名稱
 */
void BLE_setup(const char* bleName, bool silentMode) {
  // 創建 BLE 設備
  BLEDevice::init(bleName);
  
  // 創建 BLE 伺服器
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // 創建 BLE 服務
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // 創建 BLE 特徵
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
                    
  // 創建 BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());
  
  // 設置特徵回調
  pCharacteristic->setCallbacks(new MyCallbacks());
  
  // 啟動服務
  pService->start();
  
  // 開始廣播
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // 連線優先級
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  if (!silentMode) {
    Serial.println("--------------------------------");
    Serial.println("低功耗藍牙(BLE)設定:");
    Serial.print("- 設備名稱: ");
    Serial.println(bleName);
    Serial.println("- 等待連線中...");
    
    if (_bleCallback != NULL) {
      Serial.println("- 回調函式已設定");
    }
    Serial.println("--------------------------------");
  }
}

/**
 * 設定低功耗藍牙訊息回調函式
 * @param callback 回調函式指針
 */
void BLE_setCallback(BLECallbackFunction callback, bool silentMode) {
  _bleCallback = callback;
  if (!silentMode) {
    Serial.println("低功耗藍牙回調函式已設定");
  }
}

/**
 * 低功耗藍牙主迴圈處理
 * 此函式應在主迴圈中定期呼叫
 */
void BLE_loop() {
  // 處理重新連線
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // 給藍牙堆疊時間處理
    pServer->startAdvertising(); // 重新開始廣播
    oldDeviceConnected = deviceConnected;
  }
  
  // 已連線
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}

/**
 * 透過低功耗藍牙發送訊息
 * @param message 要發送的訊息
 * @return 是否成功發送
 */
bool BLE_sendMessage(const String &message, bool ln, bool silentMode) {
  if (deviceConnected) {
    String sendStr = message;
    if (ln) {
      sendStr += "\n";
    }
    pCharacteristic->setValue(sendStr.c_str());
    pCharacteristic->notify();
    return true;
  } else {
    if (!silentMode) {
      Serial.println("低功耗藍牙未連接，無法發送訊息");
    }
    return false;
  }
}

/**
 * 檢查低功耗藍牙連接狀態
 * @return 是否已連接
 */
bool BLE_checkStatus() {
  return deviceConnected;
}