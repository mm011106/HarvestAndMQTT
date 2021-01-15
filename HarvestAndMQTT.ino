

// **************************************
//  Soracom HarvestとMQTT AWS-IoTのテストのための
//    Environmental Sensor BME-280
// デバイスのテスト動作用スクリプト
//
// 2021/1/12  V0.1  miyamoto
//    NTPによるRTCのイニシャライズあり
// **************************************


#include <WioLTEforArduino.h>
#include <WioLTEClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <timer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <stdio.h>
#include <Arduino_JSON.h>

#define INTERVAL        (60000)
#define RECEIVE_TIMEOUT (10000)

constexpr boolean NETWORK_ON = true;

constexpr char ID_HEADER[] = "ET_EI001_";

#define MQTT_SERVER_HOST  "beam.soracom.io"
#define MQTT_SERVER_PORT  (1883)

#define OUT_TOPIC         "wio/test/cloud"
#define IN_TOPIC          "$aws/things/ET_EI001_440103197786190_866522040084085/shadow/update/delta"

String aws_ID = ID_HEADER;

char imsi[16]="";
char imei[16]="";

typedef struct bme280_reading {
  float_t temperature;
  float_t humidity;
  float_t pressure;

} bme280_reading_t;

typedef struct net_host {
  char name[64];
  uint16_t port;
  char user[32];
  char pass[32];
} net_host_t;

enum net_err_code_t {
  E_OK = 0,
  E_OPEN_CLOSE,
  E_SEND_RECEIVE,
  E_CONNECTION,
  E_TIMEOUT,
  E_PING
};

WioLTE Wio;

WioLTEClient WioClient(&Wio);
PubSubClient MqttClient;

HardwareTimer Timer1(TIM2);

Adafruit_BME280 bme; // I2C

net_host_t host_soracom_air = {"soracom.io", 0, "sora", "sora"};
net_host_t host_aws_mqtt = {"beam.soracom.io", 1883, "", ""};
net_host_t host_soracom_harvest = {"harvest.soracom.io", 8514, "", ""};
net_host_t host_ping_check = {"8.8.8.8",0,"",""};

uint32_t tickCount;
boolean f_tick;
bme280_reading_t sensor_readings;

uint16_t bin_net_err[(net_err_code_t)E_PING + 1]={};

void setup() 
{
  // wait the srial monitor on the host to be ready. これを有効にするとスクリプトがシリアルポートを待つので、
  //  スタンドアロンの時にここで永遠に待つことになる。
  //while(!SerialUSB) ;
  
  delay(3000);

  SerialUSB.println("");
  SerialUSB.println("--- START ");

  Wio.Init();
  
  Wio.PowerSupplyGrove(true);
  delay(500);
  
  Wio.PowerSupplyLTE(true);
  delay(500);

  unsigned status;

  SerialUSB.println("---  INIT: setup BME280");
  // BME280 default settings
//  status = bme.begin();  
  // You can also pass in a Wire library object like &Wire2
  status = bme.begin(0x76, &Wire);
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");
      while (1) delay(10);
  }

  SerialUSB.println("---  INIT: setup Timer");
  Timer1.setOverflow(1 * 1000 * 1000, MICROSEC_FORMAT); //1 s :30 s Max
  Timer1.attachInterrupt(&tickTack);
  // SerialUSB.println(Timer1.getTimerClkFreq());
  // SerialUSB.println(Timer1.getPrescaleFactor());
  // SerialUSB.println(Timer1.getCount());
  // SerialUSB.println(Timer1.getOverflow());
  // SerialUSB.println(Timer1.getCaptureCompare(1));


    if(NETWORK_ON){

      uint16_t restart_count =0;
      while (!connect_net(host_soracom_air, host_ping_check.name)){
        SerialUSB.print(restart_count);
        SerialUSB.println("Network init fail. retry in 5min...");
        delay(5*60*1000); // wait for 5 min
        restart_count++;
        if (restart_count > 10){
          restart_count=0;
          SerialUSB.println("Restart the system.");
          delay(1000);
          Wio.SystemReset();
          delay(5000);
        }
      }

      SerialUSB.println("### Sync time.");
      if (!Wio.SyncTime("ntp.nict.jp")) {
        SerialUSB.println("### ERROR! ###");
        catch_net_error(E_CONNECTION, bin_net_err);
      }
      delay(500);      

      uint16_t len_imsi=Wio.GetIMSI(imsi,sizeof(imsi));
      SerialUSB.print("IMSI=");
      SerialUSB.println(imsi);
      delay(500); 
      aws_ID+=imsi;
      aws_ID+="_";

      uint16_t len_imei=Wio.GetIMEI(imei,sizeof(imei));
      SerialUSB.print("IMEI=");
      SerialUSB.println(imei);
      aws_ID+=imei;
      
      char ID[128]="";
      aws_ID.toCharArray(ID,sizeof(ID));
      SerialUSB.println(ID);

      SerialUSB.print("### Connecting to MQTT server :");
      SerialUSB.println(host_aws_mqtt.name);

      MqttClient.setServer(host_aws_mqtt.name, host_aws_mqtt.port);
      MqttClient.setCallback(callback_MQTT_subscribe);
      MqttClient.setClient(WioClient);
      
      if (!MqttClient.connect(ID)) {
        SerialUSB.println("### ERROR! ###");
        catch_net_error(E_CONNECTION, bin_net_err);
      }

      MqttClient.subscribe(IN_TOPIC);
    }

    SerialUSB.println("---  INIT:   finished... ");
    SerialUSB.println("");
    
    Timer1.resume();
}
 
void loop() 
{
  MqttClient.loop();

  if (f_tick){
    // 毎分の実行
    if ((tickCount % 60)==0){

      struct tm now;
      if (!Wio.GetTime(&now)) {
        SerialUSB.println("### ERR:GetTime()");
      }
      SerialUSB.print("UTC:");
      SerialUSB.print(asctime(&now));

      bme280_reading_t readings = readBME280();
      SerialUSB.print("Temperature = ");
      SerialUSB.println(readings.temperature);

      sendHarvest(readings);
    }

    //  ５分毎の実行
    if ((tickCount % 300)==0){
      struct tm now;
      if (!Wio.GetTime(&now)) {
        SerialUSB.println("### ERR:GetTime()");
      }
      SerialUSB.print("UTC:");
      SerialUSB.print(asctime(&now));

      uint16_t net_err_total=0;
      for (int i=0; i<(net_err_code_t)E_PING+1; i++){
        net_err_total+=bin_net_err[i];
      }
      SerialUSB.print("Network err in total:");
      SerialUSB.println(net_err_total);
    }

    f_tick = false;
  }

  delay(100);
}

void tickTack(HardwareTimer *HT) {
  tickCount++;
  //  24時間でクリア
  if (tickCount == 24*60*60){
    tickCount=0;
  }

  f_tick = true;
  // SerialUSB.print(tickCount);
  // SerialUSB.print("+");
}

/*! 
    @brief  Harvestにデータを送る
    @param value JSONフォマットのデータ
    @return True:正常終了   False:送信エラー
*/
boolean sendHarvest(bme280_reading_t value){

  if(NETWORK_ON){

    char data[64];
    sprintf(data,"{\"temp\":%.1f}", value.temperature);
    
    uint16_t connectId;
    connectId = Wio.SocketOpen(host_soracom_harvest.name , host_soracom_harvest.port , WIOLTE_UDP);
    if (connectId < 0) {
      SerialUSB.println("### Socket Connection ERROR! ###");
      catch_net_error(E_OPEN_CLOSE, bin_net_err);
      return false;
    }

    // SerialUSB.println("### Send.");
    SerialUSB.print(">");
    SerialUSB.print(data);
    SerialUSB.println("");

    if (Wio.SocketSend(connectId, data)) {
      //  送信が成功したらアクノリッジを読み込む
      // SerialUSB.println("### Receive.");
      int length=-1;
      length = Wio.SocketReceive(connectId, data, sizeof (data), RECEIVE_TIMEOUT);
      
      if (length > 0) {
        SerialUSB.print("<");
        SerialUSB.print(data);
        SerialUSB.println("");
      }  else if (length == 0) {
        SerialUSB.println("### RECEIVE TIMEOUT! ###");
        catch_net_error(E_TIMEOUT, bin_net_err);
        return false;
      } else {
        SerialUSB.println("### Receive ERROR! ###");
        catch_net_error(E_SEND_RECEIVE, bin_net_err);
        return false;
      }

    } else { 
      // 送信失敗の場合
      SerialUSB.println("### Send ERROR! ###");
      catch_net_error(E_SEND_RECEIVE, bin_net_err);
      return false;
    }

    // SerialUSB.println("### Close.");
    if (!Wio.SocketClose(connectId)) {
      SerialUSB.println("### Socket Close ERROR! ###");
      catch_net_error(E_OPEN_CLOSE, bin_net_err);
      return false;
    }

  } else {
    SerialUSB.println("Network interface is closed.");
  }

  return true;
}

bme280_reading_t readBME280(void) { 
  
    bme280_reading_t readings={};
    readings.temperature = bme.readTemperature();
    readings.pressure = bme.readPressure() / 100.0F;
    readings.humidity = bme.readHumidity();

    return readings;
}


void callback_MQTT_subscribe(char* topic, byte* payload, unsigned int length) {

  String topic_s = topic;

  uint16_t cast_mode = 0;

  SerialUSB.print("Subscribe:");
  SerialUSB.println(topic_s);

  if(topic_s.indexOf(imsi) > 0){
    SerialUSB.print("unicast ");
    cast_mode = 1;
  }
  if(topic_s.indexOf("broadcast") > 0){
    SerialUSB.print("broadcast ");
    cast_mode = 2;
  }
  
  if (cast_mode > 0){
    SerialUSB.print(":");
    // for (int i = 0; i < length; i++) SerialUSB.print((char)payload[i]);
    // for (int i = 0; i < length; i++) payload_s+=(char)payload[i];
    payload[length]='\0';
    String payload_s = String((char*)payload);
    SerialUSB.println(payload_s);

    JSONVar payload_json = JSON.parse((char*)payload);
    SerialUSB.println(payload_json);
    SerialUSB.println(payload_json.keys());
    JSONVar layer2=payload_json[payload_json.keys()[0]];
    SerialUSB.println(layer2.keys());
    SerialUSB.println(sizeof(layer2.keys()));

    if (payload_json["state"]["desired"].hasOwnProperty("command")) {
      SerialUSB.print("command = ");
      SerialUSB.println((const char*) payload_json["state"]["desired"]["command"]);
    }

    MQTT_publish();
  }


}

void MQTT_publish(void){
  char data[100];
  sprintf(data, "{\"ans\":\"I've got a message.\"}");
  SerialUSB.print("Publish:");
  SerialUSB.print(data);
  SerialUSB.println("");
  if(!MqttClient.publish(OUT_TOPIC, data)){
    SerialUSB.println("### MQTT publish err");
    catch_net_error(E_SEND_RECEIVE, bin_net_err);
  };
  
}

bool connect_net(const net_host_t& host, const char* ping_host){

  SerialUSB.println("---  INIT: Network ");
  SerialUSB.println("### Modem turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return false;
  }
  delay(1000);

  SerialUSB.print("### Connecting to ");
  SerialUSB.println(host.name);

  if (!Wio.Activate(host.name, host.user, host.pass)) {
    SerialUSB.println("### ERROR! ###");
    return false;
  }
  delay(500);

  SerialUSB.println("### ping check. ");
  if (!Wio.Ping(ping_host)){
    SerialUSB.println("### ERROR! ###");
    return false;
  }

  return true;
}

void catch_net_error(const net_err_code_t code, uint16_t* bin){
  bin[code]++;
  // for (int i=0; i<(net_err_code_t)E_PING+1; i++){
  //   SerialUSB.println(bin[i]);
  // }
  return;
}