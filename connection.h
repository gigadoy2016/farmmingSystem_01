void connectionWIFI(char* WLAN_SSID,char* WLAN_PASS){
    Serial.print("Wifi Connecting to SSD:");
    Serial.println(WLAN_SSID);
    WiFi.begin(WLAN_SSID, WLAN_PASS);                                                   //เชื่อมต่อกับ AP
    Serial.print("MAC adress:");
    Serial.println(WiFi.macAddress());
    int count =0; 
    
    while (WiFi.status() != WL_CONNECTED)                                     //รอการเชื่อมต่อ
    {
          delay(500);
          Serial.print(".");
          ++count;
          if(count >5000){
            ESP.reset();
            return;
          }
    }
    Serial.println("");
    Serial.print("WiFi connected IP:");                                         //แสดงข้อความเชื่อมต่อสำเร็จ  
    Serial.println(WiFi.localIP());                                           // แสดงหมายเลข IP ของ Server    
    Serial.println("<<  <<< << Network READY >>>>> >>>  >>");
}

void thingsSpeak(float temperature,float humitdity) {
  
    static const char*      HOST_CLOUD   = "api.thingspeak.com";
    static const char*      apiKEY_CLOUD = "OJDJI20NWYIRFWWB";
    WiFiClient client_1;                                  // Use WiFiClient class to create TCP connections

    if (!client_1.connect(HOST_CLOUD, 80)) {
         Serial.print(HOST_CLOUD);
         Serial.println("connection failed");
        return;
    }
    String url = "/update/";                            // We now create a URI for the request
    url += "?key=";                                     //  url += streamId;
    url += apiKEY_CLOUD;
    url += "&field1=";
    url += temperature;
    url += "&field2=";
    url += humitdity;              
    client_1.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + HOST_CLOUD + "\r\n" +
                 "Connection: close\r\n\r\n");
    Serial.println("thingSpeak.com =>Send!! success");
}

void thingsSpeakData(float data[]) {
  
    static const char*      HOST_CLOUD   = "api.thingspeak.com";
    static const char*      apiKEY_CLOUD = "OJDJI20NWYIRFWWB";
    WiFiClient client_1;                                  // Use WiFiClient class to create TCP connections

    if (!client_1.connect(HOST_CLOUD, 80)) {
         Serial.print(HOST_CLOUD);
         Serial.println("connection failed");
        return;
    }
    String url = "/update/";                            // We now create a URI for the request
    url += "?key=";                                     //  url += streamId;
    url += apiKEY_CLOUD;
    for(int i=0; i < sizeof(data)+1; i++){
      url += "&field";
      url +=(i+1);
      url +="=";
      url += data[i]; 
    }
    client_1.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + HOST_CLOUD + "\r\n" +
                 "Connection: close\r\n\r\n");
    //Serial.println(url);
    Serial.println("thingSpeak.com =>Send!! success");
}
