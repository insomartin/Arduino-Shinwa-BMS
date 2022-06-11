//=============================================================
/*
   README
   platform: ESP32.
   debug at software serial, rs-485 must be connected at hardware
   serial port. only use ground and tx as debug connection. no need for rx.
   check if there is any packet
*/
//=============================================================

//#define debug; //remove comment to debug.


#define RXD2 16
#define TXD2 17



byte BMS_test[6] = {0x7E, 0x00, 0x01, 0x00, 0x00, 0x0D};

// NOTE: addresses of the bms must be sequential. eg. 0,1,2,3,4,5....
// skipping a number will result in slower polling rate due to timeout.
byte number_of_bms_connected = 1;
byte number_battery_inSeries = 15; // 15s
int bms_timeout_mS = 1000; // set of timeout per battery pack.

unsigned long previousMillis = 0;
const long polling_interval_mS = 3000;

byte serialbuf[100];
byte rx_counter = 0;
byte packet_len = 90;
bool dataok = false;

void setup() {
  Serial.begin(9600); // bms should be connected at this serial port.
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= polling_interval_mS) {
    previousMillis = currentMillis;

    Serial2.write(BMS_test, 6); // send a test packet every second.

#ifdef debug
    Serial.print("hello world");
#endif
  }

  if (Serial2.available()) {
    byte newChar = Serial2.read();
    serialbuf[rx_counter] = newChar ;
    rx_counter++;
    if (rx_counter == packet_len) {
      rx_counter = 0; // reset counter.
      dataok = true;
    }
  }

  if (dataok) {
    for ( int i = 0; i <= packet_len; i++) {
      Serial.print(serialbuf[i], HEX);
    }
    Serial.println("DATARECIEVED!");

    byte counter_1 = 6;
    for ( int i = 1; i <= number_battery_inSeries; i++) {
      byte data0, data1, cellstatus;
      float batteryVoltage;
      Serial.print("cell:");
      Serial.print(i);
      Serial.print(", ");
      cellstatus = serialbuf[counter_1];
      data0 = serialbuf[counter_1];
      data0 = data0 << 3;
      data0 = data0 >> 3; //discard first 3 bits
      data1 = serialbuf[counter_1 + 1];
      batteryVoltage = (data0 * 0xFF + data1) / 1000.00;
      //Serial.print(serialbuf[counter_1],HEX);
      Serial.print(batteryVoltage);
      Serial.print(" volts, Status:");

      if (bitRead(cellstatus, 7)) {
        Serial.print(" Unbalanced");
      }
      if (bitRead(cellstatus, 6)) {
        Serial.print(" Over voltage");
      }
      if (bitRead(cellstatus, 5)) {
        Serial.print(" Under voltage");
      }

      if (bitRead(cellstatus, 7) || bitRead(cellstatus, 6) || bitRead(cellstatus, 5)) {
        Serial.println(" ");
      }
      else {
        Serial.println(" normal");
      }
      counter_1 = counter_1 + 2;
    }

    //======= read current Amps
    /*
       charging is positive, dischaging is negative
    */
    counter_1 = counter_1 + 2;
    //Serial.println(serialbuf[counter_1],HEX);
    //Serial.println(serialbuf[counter_1 + 1],HEX);
    int current_data0, current_data1;
    float Current;
    current_data0 = serialbuf[counter_1];
    current_data1 = serialbuf[counter_1 + 1];
    Current = (30000 - (current_data0 * 0xFF + current_data1)) / 100.00;
    Serial.print("Current: ");
    Serial.print(Current);
    Serial.print(" Amps Status:");

    if (Current > 0) {
      Serial.println(" Charging");
    }
    else {
      Serial.println(" Discharging");
    }


    //======= read SOC state of charge
    /*
       charging is positive, dischaging is negative
    */
    counter_1 = counter_1 + 4;

    //Serial.println(serialbuf[counter_1], HEX);
    //Serial.println(serialbuf[counter_1 + 1], HEX);

    byte soc_data0 = serialbuf[counter_1];
    byte soc_data1 = serialbuf[counter_1 + 1];
    float battery_SOC;
    battery_SOC = (soc_data0 * 0xFF + soc_data1) / 100.00;
    Serial.print("SOC: ");
    Serial.print(battery_SOC);
    Serial.println("%");


    //======= Capacity
    /*
       Ah
    */
    counter_1 = counter_1 + 4;
    //Serial.println(serialbuf[counter_1], HEX);
    //Serial.println(serialbuf[counter_1 + 1], HEX);
    float battery_capacity;
    byte bat_data0, bat_data1;
    bat_data0 = serialbuf[counter_1];
    bat_data1 = serialbuf[counter_1 + 1];

    battery_capacity = (bat_data0 * 0xFF + bat_data1) / 100.00;
    Serial.print("Capacity: ");
    Serial.print(battery_capacity);
    Serial.println("Ah ");

    //======= Temperature
    /*
       5 sensors
    */
    counter_1 = counter_1 + 4;
    for ( int i = 1; i <= 5; i++) {
      byte data0, data1;
      float temperature;

      data0 = serialbuf[counter_1];
      data1 = serialbuf[counter_1 + 1];
      temperature = ((data0 * 0xFF + data1) / 1.00) - 50;
      Serial.print("temp");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(temperature);
      Serial.println("c");
      counter_1 = counter_1 + 2;
    }

    //======= error codes
    /*
       5 sensors
    */
    counter_1 = counter_1 + 4;

    byte error_data0 = serialbuf[counter_1];
    byte error_data1 = serialbuf[counter_1 + 1];
    byte error_data2 = serialbuf[counter_1 + 2];
    byte error_data3 = serialbuf[counter_1 + 3];

    Serial.print("data0 flag: ");

    if (bitRead(error_data0, 7)) {
      Serial.print("Voltage_Module_Error");
    }
    if (bitRead(error_data0, 6)) {
      Serial.print("Discharge_MOS_Error");
    }
    if (bitRead(error_data0, 5)) {
      Serial.print("Charge_MOS_Error");
    }

    if (bitRead(error_data0, 7) || bitRead(error_data0, 6) || bitRead(error_data0, 5)) {
      Serial.println("");
    }
    else {
      Serial.println("No Error");
    }

    Serial.print("data1 flag: ");

    if (bitRead(error_data1, 0)) {
      Serial.print("NTC_Line_Disconnected");
    }

    if (bitRead(error_data1, 1)) {
      Serial.print("Current_Module_Error");
    }

    if (bitRead(error_data1, 2)) {
      Serial.print("Charge_Source_Reversed");
    }

    if (bitRead(error_data1, 0) || bitRead(error_data1, 1) || bitRead(error_data1, 2)) {
      Serial.println("");
    }
    else {
      Serial.println("No Error");
    }

    Serial.print("data2 flag: ");



    if (bitRead(error_data2, 0)) {
      Serial.print("Discharge_OT_Protect");
    }

    if (bitRead(error_data2, 1)) {
      Serial.print("Discharge_UT_Protect");
    }

    if (bitRead(error_data2, 0) || bitRead(error_data2, 1)) {
      Serial.println("");
    }
    else {
      Serial.println("No Error");
    }

    Serial.print("data3 flag: ");

    if (bitRead(error_data3, 0)) {
      Serial.print("Charging");
    }

    if (bitRead(error_data3, 1)) {
      Serial.print("Discharging");
    }

    if (bitRead(error_data3, 2)) {
      Serial.print("Short_Current_Protect");
    }

    if (bitRead(error_data3, 3)) {
      Serial.print("Over_Current_Protect");
    }

    if (bitRead(error_data3, 4)) {
      Serial.print("Over_Voltage_Protect");
    }
    if (bitRead(error_data3, 5)) {
      Serial.print("Under_Voltage_Protect");
    }

    if (bitRead(error_data3, 6)) {
      Serial.print("Charge_OT_Protect");
    }
    if (bitRead(error_data3, 7)) {
      Serial.print("Charge_UT_Protect");
    }

    if (error_data3 > 0) {
      Serial.println("");
    }
    else {
      Serial.println("No Error");
    }

    //======= cycle count
    /*
       5 sensors
    */
    counter_1 = counter_1 + 10;
    //Serial.println(serialbuf[counter_1], HEX);
    //Serial.println(serialbuf[counter_1 + 1], HEX);
    byte cycle_count;

    cycle_count = serialbuf[counter_1 + 1];
    Serial.print("Cycle Count:");
    Serial.println(cycle_count);

    //======= pack voltage
    /*

    */
    counter_1 = counter_1 + 4;
    byte packv_data0, packv_data1;
    float packvoltage;
    packv_data0 = serialbuf[counter_1];
    packv_data1 = serialbuf[counter_1 + 1];
    //Serial.println(serialbuf[counter_1], HEX);
    //Serial.println(serialbuf[counter_1 + 1], HEX);

    packvoltage = (packv_data0 * 0xFF + packv_data1) / 100.00;
    Serial.print("Pack Voltage: ");
    Serial.print(packvoltage);
    Serial.println(" Volts");


    //======= State of health


    counter_1 = counter_1 + 4;

    //Serial.println(serialbuf[counter_1], HEX);
    //Serial.println(serialbuf[counter_1 + 1], HEX);

    byte soh_data0 = serialbuf[counter_1];
    byte soh_data1 = serialbuf[counter_1 + 1];
    float battery_SOH;
    battery_SOH = (soh_data0 * 0xFF + soh_data1) / 100.00;
    Serial.print("SOH: ");
    Serial.print(battery_SOH);
    Serial.println("%");



    dataok = false;
  }
}

byte check(byte buf[], byte len) {
  byte i, chk = 0;
  int sum = 0;
  for (i = 0; i < len; i++)
  {
    chk ^= buf[i];
    sum += buf[i];
  }
  return (byte)((chk ^ sum) & 0xFF);
}
