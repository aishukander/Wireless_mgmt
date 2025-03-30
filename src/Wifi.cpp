#include "Wireless_mgmt.h"
#include "Arduino.h"
#include <WiFi.h>

// ==========================================
// WiFi Client Mode
// ==========================================

/**
 * 連接到指定的 WiFi 網路，可選擇使用靜態IP
 * @param ssid WiFi 網路名稱
 * @param password WiFi 密碼
 * @param silentMode 是否靜默連接
 * @param timeoutSeconds WiFi連接的最長等待時間(秒)
 * @param staticIP 靜態IP地址 (可選，格式如 "192.168.1.100")
 * @param gateway 閘道地址 (可選，使用靜態IP時必須提供)
 * @param subnet 子網掩碼 (可選，使用靜態IP時必須提供)
 * @param dns1 主要DNS伺服器 (可選)
 * @param dns2 次要DNS伺服器 (可選)
 * @return 連接結果 (true: 成功, false: 失敗)
 */
bool Wifi_connect(
    const char* ssid,
    const char* password,
    int timeoutSeconds,
    bool silentMode,
    const char* staticIP,
    const char* gateway,
    const char* subnet,
    const char* dns1,
    const char* dns2
){
    if (!silentMode) {
        Serial.println("正在掃描WiFi網路...");
    }
    
    // 掃描可用的WiFi網路
    int networkCount = WiFi.scanNetworks();
    
    if (networkCount == 0) {
        if (!silentMode) {
            Serial.println();
            Serial.println("- 未找到WiFi網路 -");
        }
        return false;
    }
    
    // 查找匹配的SSID
    bool foundNetwork = false;
    for (int i = 0; i < networkCount; i++) {
        if (strcmp(WiFi.SSID(i).c_str(), ssid) == 0) {
            foundNetwork = true;
            if (!silentMode) {
                Serial.print("找到 ");
                Serial.print(ssid);
                Serial.print(" (信號強度: ");
                Serial.print(WiFi.RSSI(i));
                Serial.println(" dBm)");
            }
            break;
        }
    }
    
    // 清理掃描結果
    WiFi.scanDelete();
    
    // 如果沒有找到匹配的SSID，返回false
    if (!foundNetwork) {
        if (!silentMode) {
            Serial.println();
            Serial.println("找不到 " + String(ssid));
        }
        return false;
    }
    
    // 確定是否使用靜態IP
    bool useStaticIP = (staticIP != NULL && strlen(staticIP) > 0);
    
    if (!silentMode) {
        Serial.print("正在連接到: " + String(ssid));
        if (useStaticIP) {
            Serial.println(" (使用靜態IP)");
        } else {
            Serial.println(" (使用DHCP)");
        }
    }
    
    // 如果提供了靜態IP，就配置靜態IP
    if (useStaticIP) {
        // 檢查必須的參數是否都提供了
        if (gateway == NULL || strlen(gateway) == 0 || subnet == NULL || strlen(subnet) == 0) {
            if (!silentMode) {
                Serial.println("使用靜態IP時，必須提供閘道和子網掩碼!");
            }
            return false;
        }
        
        // 將字串轉換為IPAddress對象
        IPAddress ip, gw, sn, dns1IP, dns2IP;
        if (!ip.fromString(staticIP)) {
            if (!silentMode) {
                Serial.println("靜態IP格式無效!");
            }
            return false;
        }
        
        if (!gw.fromString(gateway)) {
            if (!silentMode) {
                Serial.println("閘道地址格式無效!");
            }
            return false;
        }
        
        if (!sn.fromString(subnet)) {
            if (!silentMode) {
                Serial.println("子網掩碼格式無效!");
            }
            return false;
        }
        
        // 如果提供了DNS，則轉換它們
        if (dns1 != NULL && strlen(dns1) > 0) {
            if (!dns1IP.fromString(dns1)) {
                if (!silentMode) {
                    Serial.println("DNS1格式無效!");
                }
                return false;
            }
        }
        
        if (dns2 != NULL && strlen(dns2) > 0) {
            if (!dns2IP.fromString(dns2)) {
                if (!silentMode) {
                    Serial.println("DNS2格式無效!");
                }
                return false;
            }
        }
        
        // 配置靜態IP
        if (!WiFi.config(ip, gw, sn, dns1IP, dns2IP)) {
            if (!silentMode) {
                Serial.println("靜態IP配置失敗!");
            }
            return false;
        }
    }
    
    // 開始連接 WiFi
    WiFi.begin(ssid, password);
    
    // 等待連接，設置超時
    int attempts = 0;
    const int delayMs = 500; // 每次延遲的毫秒數
    const int maxAttempts = timeoutSeconds * 1000 / delayMs; // 將秒轉換為嘗試次數
    
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(delayMs);
        attempts++;
        if (!silentMode) {
            Serial.print(".");
            if (attempts % 10 == 0) {
                Serial.println();
            }
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        if (!silentMode) {
            // 格式化要顯示的資訊
            String ipStr = "- IP: " + WiFi.localIP().toString();
            String macStr = "- MAC: " + WiFi.macAddress();
            String rssiStr = "- RSSI: " + String(WiFi.RSSI()) + " dBm";
            String titleStr = "- " + WiFi.SSID() + " 連接成功！";
            
            // 使用固定長度分隔線，不再動態生成
            const char* separatorLine = "--------------------------------";
            
            // 輸出資訊
            Serial.println();
            Serial.println(separatorLine);
            Serial.println(titleStr);
            Serial.println(ipStr);
            Serial.println(macStr);
            Serial.println(rssiStr);
            Serial.println(separatorLine);
        }
        return true;
    } else {
        if (!silentMode) {
            Serial.println();
            Serial.println("- WiFi 連接失敗 -");
        }
        return false;
    }
}

/**
 * 檢查WiFi連線狀態並顯示連線資訊
 * @param silentMode 是否靜默模式 (不顯示連線資訊)
 * @return WiFi狀態代碼 (WL_CONNECTED=3表示已連接) 
 */
byte Wifi_checkStatus(bool silentMode) {
    // 取得目前WiFi狀態
    byte status = WiFi.status();
    
    // 如果是靜默模式，直接返回狀態碼
    if (silentMode) {
        return status;
    }
    
    // 非靜默模式，顯示狀態資訊
    Serial.println();
    Serial.println("----------- WiFi 狀態 -----------");
    
    // 根據狀態碼顯示對應的文字說明
    String statusText = "";
    switch (status) {
        case WL_CONNECTED:
            statusText = "已連接";
            break;
        case WL_NO_SHIELD:
            statusText = "未偵測到WiFi模組";
            break;
        case WL_IDLE_STATUS:
            statusText = "閒置中";
            break;
        case WL_NO_SSID_AVAIL:
            statusText = "找不到目標網路";
            break;
        case WL_SCAN_COMPLETED:
            statusText = "掃描完成";
            break;
        case WL_CONNECT_FAILED:
            statusText = "連接失敗";
            break;
        case WL_CONNECTION_LOST:
            statusText = "連接中斷";
            break;
        case WL_DISCONNECTED:
            statusText = "未連接";
            break;
        default:
            statusText = "未知狀態 (" + String(status) + ")";
    }
    
    Serial.println("- 狀態: " + statusText);
    
    // 如果已連接，則顯示詳細資訊
    if (status == WL_CONNECTED) {
        // 基本連接資訊
        Serial.println("- SSID: " + WiFi.SSID());
        Serial.println("- 信號強度 (RSSI): " + String(WiFi.RSSI()) + " dBm");
        Serial.println("- MAC 地址: " + WiFi.macAddress());
        Serial.println("- IP 地址: " + WiFi.localIP().toString());
        Serial.println("- 子網掩碼: " + WiFi.subnetMask().toString());
        Serial.println("- 閘道: " + WiFi.gatewayIP().toString());
        Serial.println("- DNS: " + WiFi.dnsIP().toString());
        Serial.println("- WiFi 主機名稱: " + String(WiFi.getHostname()));
    }
    
    Serial.println("--------------------------------");
    Serial.println();
    
    return status;
}

// ==========================================
// WiFi Access Point Mode
// ==========================================

/**
 * 設置並啟動WiFi AP模式
 * @param ssid AP的SSID名稱
 * @param password AP的密碼 (至少8位，若為空則創建開放網絡)
 * @param channel WiFi頻道 (1-13)，預設為1
 * @param hidden 是否隱藏SSID，預設為false
 * @param maxConnection 最大連接數，預設為4
 * @return 是否成功開啟AP
 */
bool Wifi_AP_start(const char* ssid, const char* password, int channel, bool hidden, int maxConnection) {
  Serial.println("啟動WiFi AP模式... ");
  
  // 配置AP
  WiFi.mode(WIFI_AP);
  
  // 啟動AP
  bool success;
  if(password != NULL && strlen(password) >= 8) {
    success = WiFi.softAP(ssid, password, channel, hidden, maxConnection);
  } else {
    success = WiFi.softAP(ssid, NULL, channel, hidden, maxConnection);
    if(password != NULL) {
      Serial.println("警告: 密碼少於8位或無效，創建開放網絡");
    }
  }

  const char* separatorLine = "--------------------------------";
  Serial.println(separatorLine);
  if(success) {
    Serial.println("- 成功!");
    Serial.println("- AP SSID: " + String(ssid));
    Serial.println("- AP IP: " + WiFi.softAPIP().toString());
    Serial.println("- Password: " + String(password));
  } else {
    Serial.println("失敗!");
  }
  Serial.println(separatorLine);
  
  return success;
}

/**
 * 檢查WiFi AP狀態並顯示連線資訊
 * @param silentMode 是否靜默模式 (不顯示連線資訊)
 * @return 連接到AP的設備數量
 */
int Wifi_AP_checkStatus(bool silentMode) {
  int stationCount = WiFi.softAPgetStationNum();
  
  if(!silentMode) {
    Serial.println("----------- AP 狀態 -----------");
    Serial.println("- AP IP: " + WiFi.softAPIP().toString());
    Serial.println("- 連接數量: " + String(stationCount));
    Serial.println("- AP MAC: " + WiFi.softAPmacAddress());
    Serial.println("--------------------------------");
  }
  
  return stationCount;
}

/**
 * 停止WiFi AP模式
 * @return 是否成功停止AP
 */
bool Wifi_AP_stop() {
  Serial.println("--------------------------------");
  Serial.print("停止WiFi AP模式... ");
  
  bool success = WiFi.softAPdisconnect(true);
  
  if(success) {
    Serial.println("成功!");
  } else {
    Serial.println("失敗!");
  }
  Serial.println("--------------------------------");
  
  return success;
}