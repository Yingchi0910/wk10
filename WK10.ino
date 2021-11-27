#include <WiFi.h>
#include <ArduinoJson.h>
const char* ssid = "212-1";  // insert your SSID
const char* password = "ntuedorm";  // insert your password
const char* host1 = "maker.ifttt.com"; //IFTTT server網址
const char* event = "chichi";  //IFTTT事件名稱
const char* apiKey = "oz3QrvFtkRiVWeNcVxnm_a2EjaUmfAG3cjfBH8PbZt_";  //IFTTT Applet key
const char* host2 = "api.openweathermap.org";
const char* resource = "/data/2.5/weather?id=1665148&appid=bec364f733bca9408d91277a7982cb81"; 
char jsonRead[600]; //讀取response後儲存JSON資料的變數，必須是全域變數

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
}

void loop() {
    char* json = GetWeatherData();
 
    Serial.println(json);
    
    //將字串轉換為JSON，儲存在doc中
    StaticJsonDocument<600> doc;
    deserializeJson(doc, json);
    
    const char* location = doc["name"]; 
    String value1 = doc["weather"][0]["description"];
    String weatherfix = "";

    for(int i=0;i<value1.length();i++){
      if(value1[i]!=' ') weatherfix+=value1[i];
      else if(value1[i]==' ')weatherfix+="%20";
      }
    
    double value2 = doc["main"]["temp"];
    double value3 = doc["main"]["humidity"];
    Serial.print("*** ");
    Serial.print(location);
    Serial.println(" ***");
    Serial.print("Type: ");
    Serial.println(value1);
    Serial.print("Temp: ");
    Serial.print(value2 - 273);
    Serial.println("C");
    Serial.print("Humidity: ");
    Serial.print(value3);
    Serial.println("%");
    Serial.println("--------------------"); 
    
    delay(3000); // the OWM free plan API does NOT allow more then 60 calls per minute
    
    SendLineNotify(String(weatherfix), String(value2-273), String(value3));  //Client傳送資料
  
  delay(30000);
}  

void SendLineNotify(String weatherfix, String value2 , String value3 ){
  
  WiFiClient client;  //建立Client物件
  const int httpPort = 80;  //預設通訊阜80
  String url = "";  //API url
  String JsonString = "";  //此範例不會用到

  //Client連結Server
  if (client.connect(host1, httpPort)) {
    
    //Webhook API
    url += "/trigger/" + String(event) + "/with/key/" + String(apiKey);
    //Query String
    url += "?value1=" + weatherfix + "&value2=" + value2 + "&value3=" + value3;

    //Client傳送
    client.println(String("POST ") + url + " HTTP/1.1");
    client.println(String("Host: ") + host1); 
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(JsonString.length());
    client.println();
    client.println(JsonString);

    //等待5秒，每0.1秒偵測一次是否有接收到response資料  
    int timeout = 0;
    while(!client.available() && (timeout++ <= 50)){
      delay(100);
    }
  
    //如果無回應
    if(!client.available()) {
      Serial.println("No response...");
    }
    //用while迴圈一字一字讀取Response
    while(client.available()){
      Serial.write(client.read());
    }

    //停止Client
    client.stop(); 
  } 
}



char* GetWeatherData(){
  
  WiFiClient client;  //建立Client物件
  const int httpPort = 80;  //預設通訊阜80
  String JsonString = "";  //此範例不會用到

  //Client連結Server
  if (client.connect(host2, httpPort)) {

    //Client傳送
    client.println(String("POST ") + resource + " HTTP/1.1");
    client.println(String("Host: ") + host2); 
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(JsonString.length());
    client.println();
    client.println(JsonString);

    //等待5秒，每0.1秒偵測一次是否有接收到response資料  
    int timeout = 0;
    while(!client.available() && (timeout++ <= 50)){
      delay(100);
    }
  
    //如果無回應
    if(!client.available()) {
      Serial.println("No response...");
    }

    //Checking for the end of HTTP stream
    while(!client.find("\r\n\r\n")){
      // wait for finishing header stream reading ...
    }

    //讀取資料並儲存在jsonRead中
    client.readBytes(jsonRead, 600);

    //停止Client
    client.stop(); 

    //回傳
    return jsonRead;
  } 
}
