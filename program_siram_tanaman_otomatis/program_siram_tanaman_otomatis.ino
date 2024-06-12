#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// Define the I2C address of the LCD. It can be 0x27 or 0x3F. Adjust if necessary.
LiquidCrystal_I2C lcd(0x27, 16, 2);

int SensorPin = 36; // deklarasi pin analog yg dipakai
int soilMoistureValue; // menyimpan nilai analog dari sensor ke esp32
int soilmoisturepercent; // nilai yg diperoleh dalam bentuk persen setelah dimaping

#define RedLed  13 // PIN LED Merah
#define YellowLed  12 // PIN LED Kuning
#define GreenLed  14 // PIN LED Hijau

// Wi-Fi credentials
const char* ssid = "Alvian Production @ office"; // replace with your Wi-Fi SSID
const char* password = "Banyuwangi1"; // replace with your Wi-Fi password

// Firebase Realtime Database
const char* firebaseHost = "https://programsiramtanamotomatis-default-rtdb.asia-southeast1.firebasedatabase.app";
const char* firebaseAuth = "0x4v3eZ0qGzf4o22saDNpt9pP0e5efNXmHJwlzkO";

FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth auth;

unsigned long lastFailedSend = 0; // Waktu terakhir gagal kirim data ke Firebase
const unsigned long reconnectInterval = 120000; // Interval untuk mencoba reconnect (1 menit)

void setup() {
  Serial.begin(115200); // Baudrate komunikasi dengan serial monitor
  pinMode(RedLed, OUTPUT);
  pinMode(YellowLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  //Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  // Jeda untuk mengurangi gangguan dari Wi-Fi
  delay(1000);

  // Initialize Firebase
  firebaseConfig.host = firebaseHost;
  firebaseConfig.signer.tokens.legacy_token = firebaseAuth;
  Firebase.begin(&firebaseConfig, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  soilMoistureValue = analogRead(SensorPin);
  Serial.print("Nilai analog = ");
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, 4095, 0, 0, 100);

  Serial.print("Presentase kelembaban tanah= ");
  Serial.print(soilmoisturepercent);
  Serial.println("% ");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kelembaban: ");
  lcd.print(soilmoisturepercent);
  lcd.print("%");

  if (soilmoisturepercent > 60 && soilmoisturepercent <= 100) {
    Serial.println("Tanah basah");
    lcd.setCursor(0, 1);
    lcd.print("Status: Basah");
    digitalWrite(GreenLed, HIGH);
    digitalWrite(RedLed, LOW);
    digitalWrite(YellowLed, LOW);
  } else if (soilmoisturepercent > 30 && soilmoisturepercent <= 60) {
    Serial.println("Tanah kondisi normal");
    lcd.setCursor(0, 1);
    lcd.print("Status: Normal");
    digitalWrite(YellowLed, HIGH);
    digitalWrite(RedLed, LOW);
    digitalWrite(GreenLed, LOW);
  } else if (soilmoisturepercent >= 0 && soilmoisturepercent <= 30) {
    Serial.println("Tanah Kering");
    lcd.setCursor(0, 1);
    lcd.print("Status: Kering");
    digitalWrite(RedLed, HIGH);
    digitalWrite(YellowLed, LOW);
    digitalWrite(GreenLed, LOW);
  }

  // Upload to Firebase
  String path = "/soilMoisture"; // Your desired path in Firebase
  if (Firebase.setInt(firebaseData, path + "/value", soilmoisturepercent)) {
    Serial.println("Data sent to Firebase successfully");
  } else {
    Serial.print("Failed to send data to Firebase: ");
    Serial.println(firebaseData.errorReason());
    
    if (lastFailedSend == 0) {
      lastFailedSend = millis(); // Set waktu gagal kirim pertama kali
    } else if (millis() - lastFailedSend >= reconnectInterval) {
      // Mencoba menyambung ulang ke Wi-Fi dan Firebase setelah 1 menit gagal kirim data
      Serial.println("Menyambung ulang ke Wi-Fi...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
      }
      Serial.println("Terhubung ulang ke Wi-Fi");

      // Konfigurasi ulang Firebase
      firebaseConfig.host = firebaseHost;
      firebaseConfig.signer.tokens.legacy_token = firebaseAuth;
      Firebase.begin(&firebaseConfig, &auth);

      lastFailedSend = millis(); // Reset waktu gagal kirim setelah reconnect
    }

  }
  delay(3000);
  
}


// int SensorPin = 4; // Pin analog untuk sensor kelembaban tanah
// int soilMoistureValue; // Menyimpan nilai analog dari sensor

// void setup() {
//   Serial.begin(115200); // Memulai komunikasi serial dengan baud rate 115200
// }

// void loop() {
//   soilMoistureValue = analogRead(SensorPin); // Membaca nilai analog dari sensor
//   Serial.print("Nilai analog sensor kelembaban tanah: ");
//   Serial.println(soilMoistureValue); // Mencetak nilai analog ke Serial Monitor
//   delay(1000); // Menunggu 1 detik sebelum pembacaan berikutnya
// }
