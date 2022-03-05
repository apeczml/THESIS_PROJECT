#include <Adafruit_TCS34725.h>
#include <Adafruit_PWMServoDriver.h>
#include <LCD_I2C.h>
#include <Wire.h>

#define StartSwitch 2
#define ResetSwitch 3

#define PWM_SERVO_ADD 0x40
#define LCD_I2C_ADD 0x3F
#define COLOR_SENSOR_ADD 0x70

#define CHANEL_1_ARM 0
#define CHANEL_1_SLIDER 1

#define CHANEL_2_ARM 2
#define CHANEL_2_SLIDER 3


#define CHANEL_3_ARM 4
#define CHANEL_3_SLIDER 5


Adafruit_TCS34725 ColorSensor = Adafruit_TCS34725();

Adafruit_PWMServoDriver ServoController = Adafruit_PWMServoDriver(0x40, Wire);

float red, green, blue;

boolean machine_started = false;
boolean confirm_reset = false;

int last_reset_log = 0;
boolean error_status = false;

int SmallCounter = 0;
int MediumCounter = 0;
int LargeCounter = 0;
byte I2Cerror;
boolean override_error = true;

enum colorswitch{UD,GC,YGC,YC};

/** INITIALIZE LCD AND TCA9548A MULTIPLEXER **/

LCD_I2C lcd(0X3F, 16, 2);

void TCA9548A(uint8_t bus)  // function of TCA9548A
{
   Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
   Wire.write(1 << bus);          // send byte to select bus
   Wire.endTransmission();
}

/** INITIALIZE LCD AND TCA9548A MULTIPLEXER **/

void setup() {
  
   Serial.begin(9600);
   pinMode(StartSwitch, INPUT_PULLUP);
   pinMode(ResetSwitch, INPUT_PULLUP);

   Wire.begin();

   delay(100);
   lcd.begin();
   lcd.backlight();

   delay(100);
   lcd.setCursor(0, 0);
   lcd.print("STATUS:CHECKING");
   delay(1000);

   lcd.setCursor(0, 1);
    lcd.print("CHECK SERVO CTRL");
    TCA9548A(2);
    ColorSensor.begin();   
    delay(1000);
    
   Wire.beginTransmission(PWM_SERVO_ADD);
   I2Cerror =  Wire.endTransmission();

   if(I2Cerror!=0){
    
      error_status = true;
      lcd.setCursor(0, 1);
      lcd.print("SERVO CTRL N.F");
      TCA9548A(2);
      ColorSensor.begin();   
      delay(1000);   
      
   }else{

      
   ServoController.begin();
   ServoController.setPWMFreq(60);  

    lcd.setCursor(0, 1);
    lcd.print("SERV.CTL RUNNING");
    TCA9548A(2);
    ColorSensor.begin();   
    delay(1000);
    
   
    lcd.setCursor(0, 1);
    lcd.print("CHECK. CLR.SEN. 1");
    TCA9548A(2);
    ColorSensor.begin();   
    delay(1000);
    if(ColorSensor.begin()){
    
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("SENS.1 - RUNNING");

      delay(1000);
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("CHCK. CLR.SENS 2");
      
      TCA9548A(3);
      ColorSensor.begin();
      delay(1000);
       if(ColorSensor.begin()){
            
            lcd.setCursor(0, 1);
            lcd.print("                ");  
            lcd.setCursor(0, 1);
            lcd.print("SENS.2 - RUNNING");

              delay(1000);
              lcd.setCursor(0, 1);
              lcd.print("                ");
              lcd.setCursor(0, 1);
              lcd.print("CHCK. CLR.SENS 3");
              
              TCA9548A(4);
              ColorSensor.begin();
              delay(1000);
               if(ColorSensor.begin()){
                    
                    lcd.setCursor(0, 1);
                    lcd.print("                ");  
                    lcd.setCursor(0, 1);
                    lcd.print("SENS.3 - RUNNING");
        
                     
                 }else{
                     error_status = true;
                    lcd.setCursor(0, 1);
                    lcd.print("                ");
                    lcd.setCursor(0, 1);
                    lcd.print("SENS.3 - N.FOUND");
                }
               
         }else{
             error_status = true;
            lcd.setCursor(0, 1);
            lcd.print("                ");
            lcd.setCursor(0, 1);
            lcd.print("SENS.2 - N.FOUND");
        }
       
   }else{
      error_status = true;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("SENS.1 - N.FOUND");
  }
    
  }

   if(error_status==true){
    
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("STATUS: ERROR");
    
  }else{

     SmallCounter = 0;
     MediumCounter = 0;
     LargeCounter = 0;
  }

   delay(1000);
}

void loop() {
  
   String disp_small = "";
   String disp_medium = "";
   String disp_large = ""; 
   int SensorChannel = 0;
   String ColorRst = "";
   int ReadStartSwitch = !digitalRead(StartSwitch);
   int ReadResetSwitch = !digitalRead(ResetSwitch);

    checkError();

    
    if(!error_status || override_error == true){
      
         if (!ReadStartSwitch) {
            lcd.setCursor(0, 0);
            lcd.print("                ");  
            lcd.setCursor(0, 0);
            lcd.print("STATUS: STANDBY");  
            lcd.setCursor(0, 1);
            lcd.print("                ");
            lcd.setCursor(0, 1);
            lcd.print("SWITCH START BTN");
      
         } else { 


              String Tresult = "";
              uint16_t r, g, b, c, colorTemp, Tlux;
              TCA9548A(2);
              ColorSensor.getRGB(&red, &green, &blue);
              ColorSensor.getRawData(&r, &g, &b, &c);
              colorTemp = ColorSensor.calculateColorTemperature_dn40(r, g, b, c);
              Tlux = ColorSensor.calculateLux(r, g, b);
              ColorSensor.setInterrupt(true);  // turn off LED
              
              Tresult = checkcolor(int(r),int(g),int(b),int(Tlux),int(colorTemp),int(c));
              ColorRst = Tresult;
              SensorChannel = 1;
              
              Serial.println("");
              Serial.print("READING SENSOR 1\t");
              Serial.print("R:\t"); Serial.print(int(red));Serial.print("("); Serial.print(r, DEC);Serial.print(")");
              Serial.print("\tG:\t"); Serial.print(int(green)); Serial.print("("); Serial.print(g, DEC);Serial.print(")");
              Serial.print("\tB:\t"); Serial.print(int(blue));Serial.print("("); Serial.print(b, DEC);Serial.print(")");
              Serial.print("\tCLUX:\t"); Serial.print(Tlux);
              Serial.print("\tCTEMP:\t"); Serial.print(colorTemp);
              Serial.print("\tRAW-C: "); Serial.print(c, DEC); Serial.print(" ");
              Serial.print("\t: RESULT: ");
              Serial.print(Tresult);
              
              CalculateMove(int(SensorChannel), String(ColorRst));
              delay(100);
             
              TCA9548A(3);
              ColorSensor.getRGB(&red, &green, &blue);
              ColorSensor.getRawData(&r, &g, &b, &c);
              colorTemp = ColorSensor.calculateColorTemperature_dn40(r, g, b, c);
              Tlux = ColorSensor.calculateLux(r, g, b);
              ColorSensor.setInterrupt(true);  // turn off LED
              
              Tresult = checkcolor2(int(r),int(g),int(b),int(Tlux),int(colorTemp),int(c));
              ColorRst = Tresult;
              SensorChannel = 2;
              
              Serial.println("");
              Serial.print("READING SENSOR 1\t");
              Serial.print("R:\t"); Serial.print(int(red));Serial.print("("); Serial.print(r, DEC);Serial.print(")");
              Serial.print("\tG:\t"); Serial.print(int(green)); Serial.print("("); Serial.print(g, DEC);Serial.print(")");
              Serial.print("\tB:\t"); Serial.print(int(blue));Serial.print("("); Serial.print(b, DEC);Serial.print(")");
              Serial.print("\tCLUX:\t"); Serial.print(Tlux);
              Serial.print("\tCTEMP:\t"); Serial.print(colorTemp);
              Serial.print("\tRAW-C: "); Serial.print(c, DEC); Serial.print(" ");
              Serial.print("\t: RESULT: ");
              Serial.print(Tresult);
              
              CalculateMove(int(SensorChannel), String(ColorRst));
              delay(100);
              
              TCA9548A(4);
               ColorSensor.getRGB(&red, &green, &blue);
              ColorSensor.getRawData(&r, &g, &b, &c);
              colorTemp = ColorSensor.calculateColorTemperature_dn40(r, g, b, c);
              Tlux = ColorSensor.calculateLux(r, g, b);
              ColorSensor.setInterrupt(true);  // turn off LED
              
              Tresult = checkcolor(int(r),int(g),int(b),int(Tlux),int(colorTemp),int(c));
              ColorRst = Tresult;
              SensorChannel = 3;
              
              Serial.println("");
              Serial.print("READING SENSOR 1\t");
              Serial.print("R:\t"); Serial.print(int(red));Serial.print("("); Serial.print(r, DEC);Serial.print(")");
              Serial.print("\tG:\t"); Serial.print(int(green)); Serial.print("("); Serial.print(g, DEC);Serial.print(")");
              Serial.print("\tB:\t"); Serial.print(int(blue));Serial.print("("); Serial.print(b, DEC);Serial.print(")");
              Serial.print("\tCLUX:\t"); Serial.print(Tlux);
              Serial.print("\tCTEMP:\t"); Serial.print(colorTemp);
              Serial.print("\tRAW-C: "); Serial.print(c, DEC); Serial.print(" ");
              Serial.print("\t: RESULT: ");
              Serial.print(Tresult);
              
              CalculateMove(int(SensorChannel), String(ColorRst));
              delay(100);
              Serial.println("");
              delay(300);
                
              
              lcd.setCursor(0, 0);
              lcd.print("                ");  
              lcd.setCursor(0, 0);
              lcd.print(" SML   MED   LRG");
              
              
              disp_small = disp_small.concat(" ");
              disp_medium = disp_medium.concat("   ");
              disp_large = disp_large.concat("   ");
              disp_small = formatter(SmallCounter);
              disp_medium =formatter(MediumCounter);
              disp_large = formatter(LargeCounter);
              
              lcd.setCursor(0, 1);
              lcd.print("                ");
              lcd.setCursor(0, 1);
              lcd.print(disp_small);
              lcd.setCursor(6, 1);
              lcd.print(disp_medium);
              lcd.setCursor(12, 1);
              lcd.print(disp_large);

         }

    }else{
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("STATUS: ERROR");
   }
delay(300);

}

void checkError(){

   Wire.beginTransmission(PWM_SERVO_ADD);
   I2Cerror =  Wire.endTransmission();
   if(I2Cerror!=0){
    
      error_status = true;
      lcd.setCursor(0, 1);
      lcd.print("SERVO CTRL N.F");
      TCA9548A(2);
      ColorSensor.begin();   
      delay(1000);   
      
   }else{

    TCA9548A(2);
    ColorSensor.begin();   
    if(ColorSensor.begin()){
    
      TCA9548A(3);
      ColorSensor.begin();
     
       if(ColorSensor.begin()){
              
              TCA9548A(4);
              ColorSensor.begin();
             
               if(ColorSensor.begin()){
        
                     error_status = false;
                     
                 }else{
                    error_status = true;
                    error_status = true;
                    lcd.setCursor(0, 1);
                    lcd.print("                ");
                    lcd.setCursor(0, 1);
                    lcd.print("SENS.3 - N.FOUND");
                   
                }
               
         }else{
             error_status = true;
             error_status = true;
              lcd.setCursor(0, 1);
              lcd.print("                ");
              lcd.setCursor(0, 1);
              lcd.print("SENS.2 - N.FOUND");
          
        }
       
   }else{
      error_status = true;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("SENS.1 - N.FOUND");
  }
    
}  
  
}

void CalculateMove(int SensorChannel,String ColorRst){
    
   if(ColorRst!="UD"){
          switch(SensorChannel){
            case 1:
              SmallCounter+=1;
            break;
            case 2:
              MediumCounter+=1;
            break;
            case 3:
              LargeCounter+=1;
            break;
          }
      
          if(ColorRst == "GC"){
               Serial.print((String)"\tMOVING CHANNEL "+SensorChannel+" TO GREEN");
               
          }else if(ColorRst == "YGC"){
      
              Serial.print((String)"\tMOVING CHANNEL "+SensorChannel+" TO YELLOW GREEN");
            
          }else if(ColorRst == "YC"){
              Serial.print((String)"\tMOVING CHANNEL "+SensorChannel+" TO YELLOW");
          }else{
              Serial.print((String)"\tUKNOWN DIRECTION");
          }
    }
}

String checkcolor(int redcolor, int greencolor, int bluecolor, int tlux, int rtemp, int rawc){
    String Result ="";
    
   if(redcolor>= 4 && redcolor<=7 && greencolor>=7 && greencolor<=13 && bluecolor>= 6 && bluecolor<= 11 && tlux>=5 && tlux<=10 && rtemp>=5836 && rtemp<=7487){
         Result = "GC";
         
   }else if(redcolor>= 7 && redcolor<=8 && greencolor>=10 && greencolor<=13 && bluecolor>= 7 && bluecolor<= 9 && tlux>=8 && tlux<=12 && rtemp>=5201 && rtemp<=5745){
         Result = "YGC";
         
   }else if(redcolor>= 11 && redcolor<=17 && greencolor>=15 && greencolor<=19 && bluecolor>= 9 && bluecolor<= 13 && tlux>=11 && tlux<=16 && rtemp>=4010 && rtemp<=4854){
         Result = "YC";
         
    }else{
         Result = "UD";
         
    }
    return Result;
}

String checkcolor2(int redcolor, int greencolor, int bluecolor, int tlux, int rtemp, int rawc){
    String Result ="";
    
   if(redcolor>= 3 && redcolor<=5 && greencolor>=4 && greencolor<=7 && bluecolor>= 4 && bluecolor<= 4 && tlux>=2 && tlux<=6 && rtemp>=4439 && rtemp<=6471){
         Result = "GC";
         
   }else if(redcolor>= 6 && redcolor<=9 && greencolor>=8 && greencolor<=12 && bluecolor>= 4 && bluecolor<= 5 && tlux>=7 && tlux<=12 && rtemp>=3507 && rtemp<=4566){
         Result = "YGC";
         
   }else if(redcolor>= 12 && redcolor<=14 && greencolor>=14 && greencolor<=15 && bluecolor>= 6 && bluecolor<= 7 && tlux>=12  && tlux<=14 && rtemp>=3023 && rtemp<=3442){
         Result = "YC";
         
    }else{
         Result = "UD";
         
    }
    return Result;
}

String formatter(int num){
  
  int l_num = String(num).length();
  String v_num="";
  int zeros = 4 - l_num;
  for(int i = 0;i<zeros;i++){
    v_num.concat("0");
  }  
   v_num.concat(num); 
  return v_num;
}
