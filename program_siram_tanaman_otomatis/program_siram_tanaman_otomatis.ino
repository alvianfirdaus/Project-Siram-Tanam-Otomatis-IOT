#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h> // Pustaka untuk DHT22
#include <PubSubClient.h>

// Define the I2C address of the LCD. It can be 0x27 or 0x3F. Adjust if necessary.
LiquidCrystal_I2C lcd(0x27, 16, 2);

int SensorPin = 36; // Pin analog untuk sensor kelembaban tanah
int soilMoistureValue; // Menyimpan nilai analog dari sensor ke esp32
int soilmoisturepercent; // Nilai yang diperoleh dalam bentuk persen setelah di-mapping

#define RedLed  13 // PIN LED Merah
#define YellowLed  12 // PIN LED Kuning
#define GreenLed  14 // PIN LED Hijau
#define RelayPin 27 // PIN Relay untuk mengontrol pompa

// Konfigurasi DHT22
#define DHTPIN 4  // Pin untuk sensor DHT22
#define DHTTYPE DHT22 // Definisikan tipe sensor DHT22
DHT dht(DHTPIN, DHTTYPE);

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
const unsigned long reconnectInterval = 120000; // Interval untuk mencoba reconnect (2 menit)

void setup() {
  Serial.begin(115200); // Baudrate komunikasi dengan serial monitor
  pinMode(RedLed, OUTPUT);
  pinMode(YellowLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);
  pinMode(RelayPin, OUTPUT); // Set pin relay sebagai output

  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  // Initialize DHT sensor
  dht.begin();

  // Connect to Wi-Fi
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
  // Membaca data dari sensor kelembaban tanah
  soilMoistureValue = analogRead(SensorPin);
  Serial.print("Nilai analog = ");
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, 4095, 0, 0, 100);

  Serial.print("Presentase kelembaban tanah = ");
  Serial.print(soilmoisturepercent);
  Serial.println("% ");

  // Membaca suhu dan kelembaban udara dari sensor DHT22
  float temperature = dht.readTemperature(); // Membaca suhu (dalam Celsius)
  float humidity = dht.readHumidity(); // Membaca kelembaban udara

  // Membaca nilai mode dan manual control dari Firebase
  int mode = 1; // default ke mode otomatis
  int manualPumpControl = 0; // default ke pompa mati jika manual
  
  if (Firebase.getInt(firebaseData, "/plot1/mode")) {
    mode = firebaseData.intData();
  } else {
    Serial.println("Failed to get mode from Firebase");
  }

  if (Firebase.getInt(firebaseData, "/plot1/manualPumpControl")) {
    manualPumpControl = firebaseData.intData();
  } else {
    Serial.println("Failed to get manual control from Firebase");
  }

  // Tampilkan status tanah di LCD berdasarkan kelembaban tanah
  // lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("Tanah: ");
  // lcd.print(soilmoisturepercent);
  // lcd.print("%");

  if (soilmoisturepercent > 60 && soilmoisturepercent <= 100) {
    Serial.println("Tanah basah");
    lcd.setCursor(0, 0);
    lcd.print("T : ");
    lcd.print(soilmoisturepercent);
    lcd.print("% | ");
    lcd.print("Basah");
  } else if (soilmoisturepercent > 30 && soilmoisturepercent <= 60) {
    Serial.println("Tanah kondisi normal");
    lcd.setCursor(0, 0);
    lcd.print("T : ");
    lcd.print(soilmoisturepercent);
    lcd.print("% | ");

    lcd.print("Normal");
  } else if (soilmoisturepercent >= 0 && soilmoisturepercent <= 30) {
    Serial.println("Tanah Kering");
    lcd.setCursor(0, 0);
    lcd.print("T : ");
    lcd.print(soilmoisturepercent);
    lcd.print("% | ");
    lcd.print("Kering");
  }

  int statusPompa; // variabel untuk menyimpan status pompa
  
  if (mode == 1) { // Mode otomatis
    // Kontrol pompa otomatis berdasarkan kelembaban tanah
    if (soilmoisturepercent > 60 && soilmoisturepercent <= 100) {
      digitalWrite(RelayPin, HIGH); // Matikan pompa
      statusPompa = 0; // Pompa mati
      lcd.setCursor(0, 1);
      lcd.print("M : oto | P : Of");

    } else if (soilmoisturepercent > 30 && soilmoisturepercent <= 60) {
      digitalWrite(RelayPin, HIGH); // Matikan pompa
      statusPompa = 0; // Pompa mati
      lcd.setCursor(0, 1);
      lcd.print("M : oto | P : Of");
    } else if (soilmoisturepercent >= 0 && soilmoisturepercent <= 30) {
      digitalWrite(RelayPin, LOW); // Nyalakan pompa
      statusPompa = 1; // Pompa nyala
      lcd.setCursor(0, 1);
      lcd.print("M : oto | P : On");
    }
  } else { // Mode manual
    // Kontrol pompa manual berdasarkan manualPumpControl
    if (manualPumpControl == 1) {
      Serial.println("Mode manual: Pompa nyala");
      digitalWrite(RelayPin, LOW); // Nyalakan pompa
      statusPompa = 1; // Pompa nyala
      lcd.setCursor(0, 1);
      lcd.print("M : mnl | P : On");
    } else {
      Serial.println("Mode manual: Pompa mati");
      digitalWrite(RelayPin, HIGH); // Matikan pompa
      statusPompa = 0; // Pompa mati
      lcd.setCursor(0, 1);
      lcd.print("M : mnl | P : Of");
    }
  }

  // Upload data ke Firebase
  String path = "/plot1";
  if (Firebase.setInt(firebaseData, path + "/soilMouisture", soilmoisturepercent) &&
      Firebase.setInt(firebaseData, path + "/status", statusPompa)&&
      Firebase.setFloat(firebaseData, path + "/temperature", temperature) && // Upload suhu
      Firebase.setFloat(firebaseData, path + "/airHumidity", humidity)) {
    Serial.println("Data sent to Firebase successfully");
  } else {
    Serial.print("Failed to send data to Firebase: ");
    Serial.println(firebaseData.errorReason());

    if (lastFailedSend == 0) {
      lastFailedSend = millis();
    } else if (millis() - lastFailedSend >= reconnectInterval) {
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