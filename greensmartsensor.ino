#include <SocketIOClient.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define EVENT_WEATHER_SENSOR "event_weather_sensor"
#define EVENT_RAIN_SENSOR "event_rain_sensor"
#define EVENT_WELCOME "event_welcome"

const int rainSensor = 5; // Chân tín hiệu cảm biến mưa ở chân digital 5 (arduino)
const int DHTPIN = 4;       //Đọc dữ liệu từ DHT11 ở chân 2 trên mạch Arduino
const int DHTTYPE = DHT22;  //Khai báo loại cảm biến, có 2 loại là DHT11 và DHT22
const int led = 2;
DHT dht(DHTPIN, DHTTYPE);
SocketIOClient client;
const char* ssid = "Manh";
const char* password = "12345678";

char host[] = "192.168.1.10";
int port = 3000;

extern String RID;
extern String Rname;
extern String Rcontent;

unsigned long previousMillis = 0;
long interval = 2000;
bool light = true;

unsigned long lastreply = 0;
unsigned long lastsend = 0;

StaticJsonBuffer<200> jsonBuffer;
JsonObject& jsonWeather = jsonBuffer.createObject();
JsonObject& jsonRain = jsonBuffer.createObject();

void setup() {
  pinMode(led, OUTPUT);
  pinMode(rainSensor, INPUT); // Đặt chân cảm biến mưa là INPUT, vì tín hiệu sẽ được truyền đến cho Arduino
  Serial.begin(115200);// Khởi động Serial ở baudrate 9600
  dht.begin();
  Serial.print("Ket noi vao mang ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { //Thoát ra khỏi vòng
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  Serial.println(F("Da ket noi WiFi"));
  Serial.println(F("Di chi IP cua ESP8266 (Socket Client ESP8266): "));
  Serial.println(WiFi.localIP());
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
  if (client.connected())
  {
    client.send("connection", "message", "Connected !!!!");
  }
}

void loop() {
  digitalWrite(led, light);
  light = !light;
  int rain = 1 - digitalRead(rainSensor);
  if (millis() - previousMillis > interval) {
    previousMillis = millis();
    float humi = dht.readHumidity();    //Đọc độ ẩm
    float temp = dht.readTemperature(); //Đọc nhiệt độ
    if (isnan(temp) || isnan(humi)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    Serial.println(rain);
    Serial.printf("Nhiet do %s - Do am %s\r\n", String(temp, 1).c_str(), String(humi, 1).c_str());
    jsonWeather["temperature"] = temp;
    jsonWeather["humidity"] = humi;
    jsonWeather["rain"] = rain;
    String objData;
    jsonWeather.printTo(objData);
    client.sendJSON(EVENT_WEATHER_SENSOR, objData);
  }

  if (client.monitor()) {
    lastreply = millis();
    Serial.println(RID);
    if (RID == EVENT_WELCOME && Rname == "time")
    {
      Serial.print("Il est ");
      Serial.println(Rcontent);
    }
  }

  if (!client.connected()) {
    client.reconnect(host, port);
  }
}
