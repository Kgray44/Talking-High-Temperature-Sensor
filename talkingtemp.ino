#include <DFRobot_RGBLCD1602.h>
#include <DFRobot_MAX31855.h>
#include <DFRobot_DF1101S.h>
#include <SoftwareSerial.h>

float temp=0;
float lasttemp=0;
float fpm=0;

unsigned long lastmillis=0;
unsigned long lastamillis=0;

bool active = true;
bool extraactive = false;
bool off = false;
bool announced = false;

DFRobot_RGBLCD1602 lcd(/*RGBAddr*/0x2D,/*lcdCols*/16,/*lcdRows*/2);  //16 characters and 2 lines of show
DFRobot_MAX31855 max31855;
SoftwareSerial df1101sSerial(9, 10);  //RX  TX
DFRobot_DF1101S df1101s;

void setup() {
  Serial.begin(115200);
  df1101sSerial.begin(115200);

  lcd.init();

  if (!df1101s.begin(df1101sSerial)){
    Serial.println("Init failed, please check the wire connection!");
    delay(1000);
  }
  
  /*Set volume to 20*/
  df1101s.setVol(80);
  Serial.print("VOL:");
  /*Get volume*/
  Serial.println(df1101s.getVol());
  /*Enter music mode*/
  df1101s.switchFunction(df1101s.MUSIC);
  /*Wait for the end of prompt tone*/
  delay(2000);
  /*Set playback mode to "repeat all"*/
  df1101s.setPlayMode(df1101s.ALLCYCLE);
  Serial.print("PlayMode:");
  /*Get playback mode*/
  Serial.println(df1101s.getPlayMode());

  df1101s.setPrompt(false);

  lasttemp = temperature(true);
}

void loop() {
  display(temperature(true));

  if (millis() - lastmillis > 60000){//check temperature announcements once per 60 seconds
    Serial.println("Checking announcements...");
    announcements(temperature(true));
  }

  if (active){//if temperature is fluctuating a lot, announce quicker (10 minutes)
    if (millis() - lastamillis > 60000*12){//10 minutes
      if (!off){
        announced = false;
        Serial.println("Reset 'announced'");
        lastamillis = millis();
      }
    }
  }
  else if (extraactive){
    if (millis() - lastamillis > 60000*4){//4 minutes
      if (!off){
        announced = false;
        Serial.println("Reset 'announced'");
        lastamillis = millis();
      }
    }
  }
  else {//if not, announce slower (45 minutes)
    if (millis() - lastamillis > 60000*50){//45 minutes
      if (!off){
        announced = false;
        Serial.println("Reset 'announced'");
        lastamillis = millis();
      }
    }
  }

}

float temperature(bool F){
  float t=0;

  /*Read Celsius and F*/
  if (!F){
    t = max31855.readCelsius();
  }
  else {
    t = (max31855.readCelsius()*1.8)+32;
  }
  Serial.print("Temperature:");
  Serial.println(t);
  return t;
}

void display(float t){
  lcd.setCursor(0,0);
  lcd.print("Temp (F): ");
  lcd.print(t,1);
  lcd.setCursor(0,1);
  if (t <= 350){
    lcd.setRGB(255,255,0);//yellow
    lcd.print("Creosote");
  }
  else if (t > 350 && t <= 650){
    lcd.setRGB(0,255,0);
    lcd.print("Optimal");
  }
  else if (t > 650){
    lcd.setRGB(255,0,0);
    lcd.print("Overfire");
  }
  lcd.setCursor(9,1);
  lcd.print("FPM:");
  lcd.print(fpm);
}

void announcements(float temp){
  if (abs(temp - lasttemp) >= 2 && abs(temp - lasttemp) < 5){//if the temperature is moving quite fast, set active value to true (for shorter delay between announcements)
    active = true;
  }
  else {active = false;}
  if (abs(temp - lasttemp) >= 5){
    extraactive = true;
  }
  else {extraactive = false;}
  Serial.println(temp - lasttemp);
  
  if (temp < 170){
    if (!announced){
      //need more wood
      df1101s.playSpecFile(11);//6
      delay(4000);
      announced = true;
      active = false;
      extraactive = false;
    }
  }

  if ((temp - lasttemp <= -0.9) && (temp - lasttemp > -1.7)){
    if (temp < 350){
      if (!announced){
        //going out
        df1101s.playSpecFile(13);//7
        delay(4000);
        announced = true;
      }
    }
  }
  else if ((temp - lasttemp <= -1.7)){
    if (temp < 350){
      if (!announced){
        //getting cold fast
        df1101s.playSpecFile(12);//6
        delay(4000);
        announced = true;
      }
    }
  }
  else if ((temp - lasttemp >= 2) && (temp - lasttemp < 4)){
    if (temp > 450){
      if (!announced){
        //warming up
        df1101s.playSpecFile(14);//8
        delay(4000);
        announced = true;
      }
    }
  }
  else if (temp - lasttemp >= 4){
    if (temp > 550){
      if (!announced){
        //getting hot fast
        df1101s.playSpecFile(15);//9
        delay(4000);
        announced = true;
      }
    }
    if (temp > 600 && temp <= 675){
      if (!announced){
        //getting hot too fast
        df1101s.playSpecFile(16);//10
        delay(4000);
        announced = true;
      }
    }
    if (temp > 675){
      if (!announced){
        //getting hot too fast
        df1101s.playSpecFile(18);//10
        delay(4000);
        announced = true;
      }
    }
  }
  if (temp <= 90){
    if (temp - lasttemp <= 0.3){
      off = true;
    }
    else {
      off = false;
    }
  }
  Serial.print("Last temp:");
  Serial.println(lasttemp);
  fpm = temp - lasttemp;
  Serial.print("F per minute:");
  Serial.println(fpm);
  lasttemp = temp;
  lastmillis = millis();
  
}
