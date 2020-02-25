#include <stdint.h>
#include <TouchScreen.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin


#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>          // f.k. for Arduino-1.5.2
#include <Adafruit_GFX.h>// Hardware-specific library
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0x7B8E

#define MINPRESSURE 200
#define MAXPRESSURE 1000

// Maximum x and y values tft can recognize input on, used in scaling
#define TS_MINX 84
#define TS_MAXX 947

#define TS_MINY 134
#define TS_MAXY 911

int bond_mode = 0;
float npv = 0;
float *cf_array;
float cf_i = 0;
int cf_flag = 0;
int num_cf=0;
int CF_mode=0;
int cf_count = 0;
int second = 0;
char current[40];
char operand[40];
float buffer;
Adafruit_GFX_Button buttons[50];
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
float operand_val = 0;
float current_val = 0;
//0 for none, 1 for addition, 2 for subtraction, 3 for multiplication, 4 for division
int operation = 0;
float rate=0, fv=0, pv=0, pmt=0, num=0;
float bond_info[7];
int bond_current = 0;
float duration = 0;
float pv0 = 0, pvless=0, pvmore=0;
float coup = 0;
float mod_rate = 0;

void setup() {
  int i;
  for (i=0;i<40;i++){
    current[i] = '\0';
    operand[i] = '\0';
  }
  for (i=0;i<4;i++)
    bond_info[i] = 0;
  Serial.begin(9600);
  lcd.init();
  lcd.init();
  lcd.backlight();
  
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.fillScreen(BLACK);
  tft.setCursor(25,25);
  tft.setRotation(1);
 
  tft.setTextColor(WHITE);
  //tft.setTextSize(3);
  //tft.print("Financial Calculator");
  //(X, Y, W, H, outline, inner color, text color, label, text size)
   
   
  
  buttons[0].initButton(&tft, 300, 250, 40  , 60 , WHITE, BLACK, GREEN, "0", 2);
  buttons[1].initButton(&tft, 350, 250, 40  , 60 , WHITE, BLACK, GREEN, ".", 2);
  buttons[2].initButton(&tft, 400, 250, 40  , 60 , WHITE, BLACK, GREEN, "(-)", 2);
  buttons[3].initButton(&tft, 450, 250, 40  , 60 , WHITE, BLACK, GREEN, "+", 2);
  buttons[4].initButton(&tft, 300, 180, 40  , 60 , WHITE, BLACK, GREEN, "1", 2);
  buttons[5].initButton(&tft, 350, 180, 40  , 60 , WHITE, BLACK, GREEN, "2", 2);
  buttons[6].initButton(&tft, 400, 180, 40  , 60 , WHITE, BLACK, GREEN, "3", 2);
  buttons[7].initButton(&tft, 450, 180, 40  , 60 , WHITE, BLACK, GREEN, "-", 2);
  buttons[8].initButton(&tft, 300, 110, 40  , 60, WHITE, BLACK, GREEN, "4", 2);
  buttons[9].initButton(&tft, 350, 110, 40  , 60, WHITE, BLACK, GREEN, "5", 2);
  buttons[10].initButton(&tft, 400, 110, 40, 60, WHITE, BLACK, GREEN, "6", 2);
  buttons[11].initButton(&tft, 450, 110, 40, 60, WHITE, BLACK, GREEN, "X", 2);
  buttons[12].initButton(&tft, 300, 40, 40, 60, WHITE, BLACK, GREEN, "7", 2);
  buttons[13].initButton(&tft, 350, 40, 40, 60, WHITE, BLACK, GREEN, "8", 2);
  buttons[14].initButton(&tft, 400, 40, 40, 60, WHITE, BLACK, GREEN, "9", 2);
  buttons[15].initButton(&tft, 450, 40, 40, 60, WHITE, BLACK, GREEN, "/", 2);
  buttons[16].initButton(&tft, 225, 180, 90 , 60, WHITE, BLACK, GREEN, "CLEAR", 2);
  buttons[17].initButton(&tft, 225, 250, 90 , 60, WHITE, BLACK, GREEN, "ENTER", 2);
  buttons[18].initButton(&tft, 225, 110, 90 , 60, WHITE, BLACK, GREEN, "CL TVM", 2);
  buttons[19].initButton(&tft, 50, 40, 40, 60, WHITE, BLACK, GREEN, "N", 2);
  buttons[20].initButton(&tft, 100, 40, 40, 60, WHITE, BLACK, GREEN, "i", 2);
  buttons[21].initButton(&tft, 150, 40, 40, 60, WHITE, BLACK, GREEN, "PV", 2);
  buttons[22].initButton(&tft, 200, 40, 40, 60, WHITE, BLACK, GREEN, "PMT", 2);
  buttons[23].initButton(&tft, 250, 40, 40, 60, WHITE, BLACK, GREEN, "FV", 2);
  buttons[24].initButton(&tft, 150, 110, 40, 60, WHITE, BLACK, GREEN, "X^2", 2);
  buttons[25].initButton(&tft, 150, 180, 40, 60, WHITE, BLACK, GREEN, "X^Y", 2);
  buttons[26].initButton(&tft, 150, 250, 40, 60, WHITE, BLACK, GREEN, "1/X", 2);
  buttons[27].initButton(&tft, 100, 110, 40, 60, WHITE, BLACK, GREEN, "SQRT", 1);
  buttons[28].initButton(&tft, 100, 180, 40, 60, WHITE, BLACK, GREEN, "LN", 2);
  buttons[29].initButton(&tft, 100, 250, 40, 60, WHITE, BLACK, GREEN, "e^X", 2);
  buttons[30].initButton(&tft, 50, 110, 40, 60, WHITE, BLACK, GREEN, "SIN", 2);
  buttons[31].initButton(&tft, 50, 180, 40, 60, WHITE, BLACK, GREEN, "COS", 2);
  buttons[32].initButton(&tft, 50, 250, 40, 60, WHITE, BLACK, GREEN, "TAN", 2);
  buttons[33].initButton(&tft, 50, 300, 40, 20, WHITE, BLACK, GREEN, "2nd", 1);
  buttons[34].initButton(&tft, 50, 250, 40, 60, WHITE, BLACK, GREEN, "ARCTAN", 1);
  buttons[35].initButton(&tft, 50, 180, 40, 60, WHITE, BLACK, GREEN, "ARCCOS", 1);
  buttons[36].initButton(&tft, 50, 110, 40, 60, WHITE, BLACK, GREEN, "ARCSIN", 1);
  buttons[37].initButton(&tft, 100, 250, 40, 60, WHITE, BLACK, GREEN, "CF", 2);
  buttons[38].initButton(&tft, 100, 180, 40, 60, WHITE, BLACK, GREEN, "ESC", 2);
  buttons[39].initButton(&tft, 50, 40, 40, 60, WHITE, BLACK, GREEN, "NPV", 2);
  buttons[40].initButton(&tft, 100, 40, 40, 60, WHITE, BLACK, GREEN, "IRR", 2);
  buttons[41].initButton(&tft, 150, 40, 40, 60, WHITE, BLACK, GREEN, "PREV", 1.5);
  buttons[42].initButton(&tft, 200, 40, 40, 60, WHITE, BLACK, GREEN, "NEXT", 1.5);
  buttons[43].initButton(&tft, 225, 110, 90, 60, WHITE, BLACK, GREEN, "CLR CF", 2);
  buttons[44].initButton(&tft, 100, 300, 40, 20, WHITE, BLACK, RED, "Bond", 1);
  buttons[48].initButton(&tft, 150, 40, 40, 60, WHITE, BLACK, RED, "PREV", 1.5);
  buttons[49].initButton(&tft, 200, 40, 40, 60, WHITE, BLACK, RED, "NEXT", 1.5);
  
  
  for (i=0;i<34;i++){
    buttons[i].drawButton(false);
  }
  buttons[44].drawButton(false);
}




void loop() {
  int i;
  int x_val = 20;
  int curr_len = det_length(current);
  TSPoint p = ts.getPoint();
  bool negative = false;
  if (current[0] == '-'){
    negative = true;
  }
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //map tft
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    

  
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE){
    //2nd
    //Test just drawing new buttons without clearing screen????
    if (buttons[33].contains(p.x, p.y)){
      buttons[33].drawButton(true);
      delay(10);
      buttons[33].drawButton(false);
      bond_mode = 0;
      if (second == 0){
        //tft.fillScreen(BLACK);
        //for (i=0;i<30;i++)
        //  buttons[i].drawButton(false);
        for (i=33; i<44;i++)
          buttons[i].drawButton(false);
        
        second = 1;
      }
      else if (second==1){
        //tft.fillScreen(BLACK);
        //for (i=30;i<34;i++)
        //  buttons[i].drawButton(false);
        for (i=18; i<33; i++)
          buttons[i].drawButton(false);
        second = 0;
      }
      lcd.clear();
      delay(100);
    }
    //BOND MODE
    if (bond_mode == 1){
      //NEXT
      if(buttons[49].contains(p.x, p.y)){
        buttons[49].drawButton(true);
        delay(10);
        buttons[49].drawButton(false);
        if (bond_current<6){
          bond_current++;
          lcd.clear();
          for (i=0;i<20;i++)
            current[i] = '\0';
          dtostrf(bond_info[bond_current], 2, 4, current);
        }
        cf_flag = 1;
      }
      //PREV
      if (buttons[48].contains(p.x, p.y)){
        buttons[48].drawButton(true);
        delay(10);
        buttons[48].drawButton(false);
        if (bond_current>0){
          bond_current--;
          lcd.clear();
          for (i=0;i<20;i++)
            current[i]='\0';
          dtostrf(bond_info[bond_current], 2, 4, current);
        }
        cf_flag = 1;
      }
      
    }
    if (second == 1){
      
      //CLEAR CF
      if (buttons[43].contains(p.x, p.y)){
        buttons[43].drawButton(true);
        delay(10);
        buttons[43].drawButton(false);
        CF_mode = 0;
        num_cf=0;
        cf_i=0;
        cf_count=0;
        npv=0;
        free(cf_array);
        lcd.clear();
      }
      //NEXT
      if(buttons[42].contains(p.x, p.y)){
        buttons[42].drawButton(true);
        delay(10);
        buttons[42].drawButton(false);
        if (cf_count != num_cf-1){
          cf_count++;
          lcd.clear();
          for (i=0;i<20;i++)
            current[i] = '\0';
          dtostrf(cf_array[cf_count], 2, 4, current);
        }
        cf_flag = 1;
      }
      //PREV
      if (buttons[41].contains(p.x, p.y)){
        buttons[41].drawButton(true);
        delay(10);
        buttons[41].drawButton(false);
        if (cf_count != 0){
          cf_count--;
          lcd.clear();
          for (i=0;i<20;i++)
            current[i]='\0';
          dtostrf(cf_array[cf_count], 2, 4, current);
        }
        else if(bond_mode == 1){
          
        }
        cf_flag = 1;
      }
      //IRR
      if (buttons[40].contains(p.x, p.y)){
        buttons[40].drawButton(true);
        delay(10);
        buttons[40].drawButton(false);
        //Calculate IRR
      }
      //NPV
      if (buttons[39].contains(p.x, p.y)){
        buttons[39].drawButton(true);
        delay(10);
        buttons[39].drawButton(false);
        //Calculate NPV
        npv = npv + cf_array[0];
        for (i=1;i<num_cf;i++){
          npv = npv + (cf_array[i] / pow(1+cf_i, i));
        }
        for (i=0;i<20;i++){
          current[i]='\0';
        }
        lcd.clear();
        cf_flag=1;
        dtostrf(npv, 2, 4, current);
      }
      //ESC
      if (buttons[38].contains(p.x, p.y)){
        buttons[38].drawButton(true);
        delay(10);
        buttons[38].drawButton(false);
        CF_mode = 0;
        bond_mode = 0;
        lcd.clear();
        }
      //CF Key
      if (buttons[37].contains(p.x, p.y)){
        buttons[37].drawButton(true);
        delay(10);
        buttons[37].drawButton(false);
        CF_mode=1;
        for (i=0;i<20;i++)
          current[i]='\0';
      }
      //ARCSIN
      if (buttons[36].contains(p.x, p.y)){
        buttons[36].drawButton(true);
        delay(10);
        buttons[36].drawButton(false);
        dtostrf(asin(atof(current)), 2, 4, current);
        cf_flag=1;
        lcd.clear();   
      }
      //ARCCOS
      if (buttons[35].contains(p.x, p.y)){
        buttons[35].drawButton(true);
        delay(10);
        buttons[35].drawButton(false);
        dtostrf(acos(atof(current)), 2, 4, current);
        cf_flag=1;
        lcd.clear();
      }
      //ARCTAN
      if (buttons[34].contains(p.x, p.y)){
        buttons[34].drawButton(true);
        delay(10);
        buttons[34].drawButton(false);
        dtostrf(atan(atof(current)), 2, 4, current);
        cf_flag=1;
        lcd.clear();
      }
    }
    if (second == 0){      
      //TAN
      if (buttons[32].contains(p.x, p.y)){
        buttons[32].drawButton(true);
        delay(10);
        buttons[32].drawButton(false);
        dtostrf(tan(atof(current)), 2, 4, current);
        cf_flag=1;
        lcd.clear();
      }
      //COS
      if (buttons[31].contains(p.x, p.y)){
        buttons[31].drawButton(true);
        delay(10);
        buttons[31].drawButton(false);
        dtostrf(cos(atof(current)), 2, 4, current);
        cf_flag=1;
        lcd.clear();
      }

    
      //e^X
      if (buttons[29].contains(p.x, p.y)){
        buttons[29].drawButton(true);
        delay(10);
        buttons[29].drawButton(false);
        dtostrf(exp(atof(current)), 2, 4, current);
        cf_flag=1;
        lcd.clear();
      }
      //LN
      if (buttons[28].contains(p.x, p.y)){
        buttons[28].drawButton(true);
        delay(10);
        buttons[28].drawButton(false);
        dtostrf(log(atof(current)), 2, 4, current);
         
        cf_flag=1;
        lcd.clear();
      }
    }
    //Bond
      if (buttons[44].contains(p.x, p.y)){
        buttons[44].drawButton(true);
        delay(10);
        buttons[44].drawButton(false);
        lcd.clear();
        second = 0;
        CF_mode = 0;
        if(bond_mode == 0){
          bond_mode = 1;
          for (i=48;i<50;i++)
            buttons[i].drawButton(false);
        }
        else if(bond_mode == 1){
            bond_mode = 0;
            for (i=18; i<33; i++)
              buttons[i].drawButton(false);
        }
    }
    //SQRT
    if (buttons[27].contains(p.x, p.y)){
      buttons[27].drawButton(true);
      delay(10);
      buttons[27].drawButton(false);
      dtostrf(sqrt(atof(current)), 2, 4, current);
       
      cf_flag=1;
      lcd.clear();
    }
    //1/x
    if (buttons[26].contains(p.x, p.y)){
      buttons[26].drawButton(true);
      delay(10);
      buttons[26].drawButton(false);
      dtostrf((1/atof(current)), 2, 4, current);
       
      cf_flag=1;
      lcd.clear();
    }
    //X^Y
    if (buttons[25].contains(p.x, p.y)){
      buttons[25].drawButton(true);
      delay(10);
      buttons[25].drawButton(false);
      operand_val = atof(current);
      for (i=0; i<20; i++){
        current[i] = 0;
      }
      cf_flag=1;
       
      lcd.clear();
      operation  = 5;
      
    }
    //X^2
    if (buttons[24].contains(p.x, p.y)){
      buttons[24].drawButton(true);
      delay(10);
      buttons[24].drawButton(false);  
      dtostrf(pow(atof(current), 2), 2, 4, current);
      cf_flag=1;
       
      lcd.clear();
    }
    //FV
    if (buttons[23].contains(p.x, p.y)){
      buttons[23].drawButton(true);
      delay(10);
      buttons[23].drawButton(false);
      if (curr_len == 0 && num != 0){
        if (pmt == 0){
          fv = pv * pow(1+rate, num);
        }
        else {
          fv = (pmt * ((pow(1+rate, num)-1)/rate)) + (pv * pow(1+rate, num));
        }
          /*float pay_arr[(int)num+1];
          pay_arr[0] = pv * pow(1+rate, num);
          for (i=1; i < num; i++){
            pay_arr[i] = pmt * pow(1+rate, num-i);
          }
          fv = 0;
          for (i=0; i<=num;i++){
            fv = fv+pay_arr[i];
          }
        }*/
        dtostrf(fv, 2, 4, current);
      }
      else{
        fv =atof(current);
        for(i=0;i<20;i++){
          current[i] = '\0';
        }  
      }
      
       
       
      lcd.clear();
      delay(150);
    }
    if (second == 0 && bond_mode == 0){
      //SIN
      if (buttons[30].contains(p.x, p.y)){
        buttons[30].drawButton(true);
        delay(10);
        buttons[30].drawButton(false);
        dtostrf(sin(atof(current)), 2, 4, current);
        cf_flag=1;
        lcd.clear();
      }
      //PMT
      if (buttons[22].contains(p.x, p.y)){
        buttons[22].drawButton(true);
        delay(10);
        buttons[22].drawButton(false);
        if (curr_len == 0 && num != 0){
          pmt = ((pv - (fv/pow(1+rate,num)))) /  ((1-((1/(pow(1+rate, num)))))/rate);
          dtostrf(pmt, 2, 4, current);
        }
        else{
          pmt = atof(current);
          for(i=0;i<20;i++){
            current[i] = '\0';
          }
        }
         
         
        lcd.clear();
        delay(150);
      }
      
      //PV
      if (buttons[21].contains(p.x, p.y)){
        buttons[21].drawButton(true);
        delay(10);
        buttons[21].drawButton(false);
        if (curr_len == 0 && num != 0){
          if (pmt == 0){
            pv = fv / pow(1+rate, num);
          }
          else {
            pv = (pmt * ((1-pow(1+rate, num * -1))/rate)) + (fv / pow(1+rate, num));          
            /*pv = 0;
            float pay_arr[(int)floor(num)];
            pay_arr[(int)num-1] = (fv+pmt)/pow(1+rate, num);
            for (i=0;i<num-1;i++){
              pay_arr[i]=pmt/pow(1+rate,i+1);
            }
            for (i=0; i<num; i++){
              pv = pv + pay_arr[i];
            }*/
          }
          dtostrf(pv, 2, 4, current);
        }
        else{
          pv =atof(current);
          for(i=0;i<20;i++){
            current[i] = '\0';
          }
        }
         
         
        lcd.clear();
        delay(150);
      }
      //i
      if (buttons[20].contains(p.x, p.y)){
        buttons[20].drawButton(true);
        delay(10);
        buttons[20].drawButton(false);
        if (curr_len == 0 && num != 0){
          float THRESHOLD = 0.0000001, pv_test=0, last1=0, last2=0, last3=0, last4=0, residual=100, rate_test=0.20, increment=0.01, pv_am=0, pv_oi=0;
          int flag=0;
          while(residual > THRESHOLD || residual <= (THRESHOLD * -1)){
            pv_am = fv / pow(1+rate_test, num);
            pv_oi = pmt * ((1 - pow(1+rate_test, -1*num))/rate_test);
            pv_test = pv_am + pv_oi;
  
            residual = pv_test - pv;
  
            last4 = last3;
            last3= last2;
            last2=last1;
            last1 = rate_test;
  
            if (last3 == last1){
              increment = increment / 10;
            }
            if (last1 == last2 && last2 == last3 && last3 == last4){
              break;
            }
            if (residual > 0){
              rate_test = rate_test + increment;
            }
            else if (residual < 0){
              rate_test = rate_test - increment;
            }
            else if (residual == 0){
              rate = rate_test;
              flag=1;
              break;
            }
          }
          if (flag==0){
            rate = rate_test;
          }
          dtostrf(rate, 2, 4, current);
        }
        else{
          rate = atof(current);
          for(i=0;i<20;i++){
            current[i] = '\0';
          }
        }
         delay(150);
         
        lcd.clear();
      }
      
      //N
      if (buttons[19].contains(p.x, p.y)){
        buttons[19].drawButton(true);
        delay(10);
        buttons[19].drawButton(false);
        num = atof(current);
        for(i=0;i<20;i++){
          current[i] = '\0';
        }
         
        delay(150);
        lcd.clear();
      }
      
      //CLEAR TVM
      if (buttons[18].contains(p.x, p.y)){
        buttons[18].drawButton(true);
        delay(10);
        buttons[18].drawButton(false);
        rate=0; pv=0; fv=0; pmt=0; num=0;
         
        
        lcd.clear();
      }
    }
    //ENTER
    if (buttons[17].contains(p.x, p.y)){
      buttons[17].drawButton(true);
      delay(10);
      buttons[17].drawButton(false);
      if (operation == 1){
        cf_flag=1;
        current_val = atof(current);
        Serial.println(current_val);
        buffer = current_val + operand_val;
        Serial.println(buffer);
        dtostrf(buffer, 3, 4, current);
        Serial.println(current);
      }
      else if (operation == 2){
        current_val = atof(current);
        buffer = operand_val - current_val;
        dtostrf(buffer, 3, 4, current);
        cf_flag = 1;
      }
      else if (operation == 3){
        current_val = atof(current);
        buffer = current_val * operand_val;
        dtostrf(buffer, 3, 4, current);
        cf_flag = 1;
      }
      else if (operation == 4){
        current_val = atof(current);
        buffer = operand_val / current_val;
        dtostrf(buffer, 3, 4, current);
        cf_flag = 1;
      }
      else if (operation == 5){
        current_val = atof(current);
        buffer = pow(operand_val, current_val);
        dtostrf(buffer, 3, 4, current);
        cf_flag = 1;
      }
      else if (bond_mode == 1 && bond_current < 4){
        current_val = atof(current);
        bond_info[bond_current] = current_val;
        for (i=0;i<20;i++){
          current[i]='\0';
        }
        dtostrf(bond_info[bond_current], 2, 4, current);
      }
      else if (bond_mode ==  1 && bond_current == 4){
        current_val = (((1+bond_info[0])/bond_info[0]) - ((1+bond_info[0] + (bond_info[2]*(bond_info[1] - bond_info[0])))/(bond_info[1]*((pow(1+bond_info[0], bond_info[2])-1)) + bond_info[0]))) - bond_info[3];
        bond_info[bond_current] = current_val;
        for (i=0;i<20;i++)
          current[i] = '\0';
        dtostrf(bond_info[bond_current], 2, 4, current);
      }
      else if (bond_mode == 1 && bond_current == 5){
        current_val = bond_info[4] / (1+bond_info[0]);
        bond_info[bond_current] = current_val;
        for (i=0;i<20;i++)
          current[i]='\0';
        dtostrf(bond_info[bond_current], 2, 4, current);
      }
      else if (bond_mode == 1 && bond_current == 6){
        coup = bond_info[1] * 1000;
        pv0 = ((coup * ((1-pow(1+bond_info[0], (bond_info[2]) * -1))/bond_info[0])) + (1000 / pow(1+bond_info[0], (bond_info[2])))) * (pow(1+bond_info[0], bond_info[3]));
        mod_rate = bond_info[0] - 0.008;
        pvless = ((coup * ((1-pow(1+mod_rate, (bond_info[2]) * -1))/mod_rate)) + (1000 / pow(1+mod_rate, (bond_info[2])))) * (pow(1+mod_rate, bond_info[3]));
        mod_rate = bond_info[0] + 0.008;
        pvmore = ((coup * ((1-pow(1+mod_rate, (bond_info[2]) * -1))/mod_rate)) + (1000 / pow(1+mod_rate, (bond_info[2])))) * (pow(1+mod_rate, bond_info[3]));
        Serial.println("PV0: ");
        Serial.print(pv0,6);
        Serial.println("PVLess: ");
        Serial.print(pvless,6);
        Serial.println("PVMore: ");
        Serial.print(pvmore,6);
        bond_info[bond_current] = (pvless+pvmore-(2*pv0)) / (pow(0.008, 2) * pv0);
        Serial.println("Conv: ");
        Serial.print(bond_info[bond_current],6);
        for (i=0;i<20;i++)
          current[i] = '\0';
        dtostrf(bond_info[bond_current], 2, 4, current);
      }
      else if (CF_mode == 1 && num_cf != 0 && cf_i != 0){
        current_val = atof(current);
        cf_array[cf_count] = current_val;
        for (i=0;i<20;i++){
          current[i]='\0';
        }
        dtostrf(cf_array[cf_count], 2, 4, current);
      }
      else if (CF_mode == 1 && num_cf != 0 && cf_i == 0){
        current_val = atof(current);
        cf_i = current_val;
        for (i=0;i<20;i++)
          current[i]='\0';
      }
      else if (CF_mode == 1 && num_cf == 0 && cf_i == 0){
        current_val = atof(current);
        num_cf = (int)current_val;
        cf_array = malloc(sizeof(float)*num_cf);
        for (i=0;i<num_cf;i++)
          cf_array[i] = 0;
        for (i=0;i<20;i++)
          current[i] = '\0';
      }

       
      
      lcd.clear();
      for (i=0;i<20;i++){
        operand[i] = '\0';
      }
      current_val=0;
      operand_val=0;
      buffer=0;
      operation=0;
    }
    //Division
    if (buttons[15].contains(p.x, p.y)){
      buttons[15].drawButton(true);
      delay(10);
      buttons[15].drawButton(false);
      for (i=0; i < curr_len; i++){
        if (current[i] != '\0'){
          operand[i] = current[i];
        }
      }
      operand_val = atof(operand);
      Serial.println(operand_val);
      for(i=0;i<20;i++){
        current[i] = '\0';
      }
      operation = 4;
       
      
      lcd.clear();
    }
    //X
    if (buttons[11].contains(p.x, p.y)){
      buttons[11].drawButton(true);
      delay(10);
      buttons[11].drawButton(false);
      for (i=0; i < curr_len; i++){
        if (current[i] != '\0'){
          operand[i] = current[i];
        }
      }
      operand_val = atof(operand);
      Serial.println(operand_val);
      for(i=0;i<20;i++){
        current[i] = '\0';
      }
      operation = 3;
       
       
      lcd.clear();
    }
    //-
    if (buttons[7].contains(p.x, p.y)){
      buttons[7].drawButton(true);
      delay(10);
      buttons[7].drawButton(false);
      for (i=0; i < curr_len; i++){
        if (current[i] != '\0'){
          operand[i] = current[i];
        }
      }
      operand_val = atof(operand);
      Serial.println(operand_val);
      for(i=0;i<20;i++){
        current[i] = '\0';
      }
      operation = 2;
       
       
      lcd.clear();
    }
    //+
    if (buttons[3].contains(p.x, p.y)){
      buttons[3].drawButton(true);
      delay(10);
      buttons[3].drawButton(false);
      for (i=0; i < curr_len; i++){
        if (current[i] != '\0'){
          operand[i] = current[i];
        }
      }
      operand_val = atof(operand);
      Serial.println(operand_val);
      for(i=0;i<20;i++){
        current[i] = '\0';
      }
      operation = 1;
       
       
      lcd.clear();
    }
    //(-)
    if (buttons[2].contains(p.x, p.y)){
      buttons[2].drawButton(true);
      delay(10);
      buttons[2].drawButton(false);
       
       
      lcd.clear();
      if (curr_len == 0){
        delay(1);
      }
      else if (current[0] == '-'){
        for(i=1;i<curr_len; i++){
          current[i-1] = current[i];
        }
        current[curr_len-1] = '\0';
      }
      else{
        for(i=curr_len; i>=0; i--){
          current[i+1]  = current[i];
        }
        current[0] = '-';
      }
      delay(100);
    }
    //. 
    if (buttons[1].contains(p.x, p.y)){
      buttons[1].drawButton(true);
      delay(10);
      buttons[1].drawButton(false);
      if (cf_flag == 1){
        lcd.clear();
        for(i=0;i<20;i++)
          current[i] = '\0';
        cf_flag = 0;
      }
      curr_len = det_length(current);
      if (find_dec(current) == 0){
        if (curr_len == 0){
          current[0] = '0';
          current[1] = '.';
        }
        else if (curr_len > 0 && curr_len < 18){
          current[curr_len] = '.';
        }
      }
      else {
        delay(1);
      }
      delay(100);
    }
    //0
    if (buttons[0].contains(p.x, p.y)){
      buttons[0].drawButton(true);
      delay(10);
      buttons[0].drawButton(false);
            if(cf_flag==1){
        lcd.clear();
        cf_flag=0;
        for(i=0;i<20;i++){
          current[i]='\0';
        }
      }
      curr_len = det_length(current);
      if (curr_len > 0 && curr_len < 18){
        current[curr_len] = '0';
      }
      else if (curr_len == 18){
        delay(1);
      }
      else {
        current[0] = '0';
      }
      delay(100);
    }

    //1    
    if (buttons[4].contains(p.x, p.y)){
      buttons[4].drawButton(true);
      delay(10);
      buttons[4].drawButton(false);
            if(cf_flag==1){
        lcd.clear();
        cf_flag=0;
        for(i=0;i<20;i++){
          current[i]='\0';
        }
      }
      curr_len = det_length(current);
      if (curr_len > 0 && curr_len < 18){
        current[curr_len] = '1';
      }
      else if (curr_len == 18){
        delay(1);
      }
      else {
        current[0] = '1';
      }
    delay(100);
    }
    //2
    if (buttons[5].contains(p.x, p.y)){
      buttons[5].drawButton(true);
      delay(10);
      buttons[5].drawButton(false);
            if(cf_flag==1){
        lcd.clear();
        cf_flag=0;
        for(i=0;i<20;i++){
          current[i]='\0';
        }
      }
      curr_len = det_length(current);
      if (curr_len > 0 && curr_len < 18){
        current[curr_len] = '2';
      }
      else if (curr_len == 18){
        delay(1);
      }
      else {
        current[0] = '2';
      }
    delay(100);
    }
    //3
    if (buttons[6].contains(p.x, p.y)){
      buttons[6].drawButton(true);
      delay(10);
      buttons[6].drawButton(false);
            if(cf_flag==1){
        lcd.clear();
        cf_flag=0;
        for(i=0;i<20;i++){
          current[i]='\0';
        }
      }
      curr_len = det_length(current);
      if (curr_len > 0 && curr_len < 18){
        current[curr_len] = '3';
      }
      else if (curr_len == 18){
        delay(1);
      }
      else {
        current[0] = '3';
      }
      delay(100);
    }
    //4
    if (buttons[8].contains(p.x, p.y)){
      buttons[8].drawButton(true);
      delay(10);
      buttons[8].drawButton(false);
            if(cf_flag==1){
        lcd.clear();
        cf_flag=0;
        for(i=0;i<20;i++){
          current[i]='\0';
        }
      }
      curr_len = det_length(current);
      if (curr_len > 0 && curr_len < 18){
        current[curr_len] = '4';
      }
      else if (curr_len == 18){
        delay(1);
      }
      else {
        current[0] = '4';
      }
      delay(100);
    }
    //5
    if (buttons[9].contains(p.x, p.y)){
      buttons[9].drawButton(true);
      delay(10);
      buttons[9].drawButton(false);
            if(cf_flag==1){
        lcd.clear();
        cf_flag=0;
        for(i=0;i<20;i++){
          current[i]='\0';
        }
      }
      curr_len = det_length(current);
      if (curr_len > 0 && curr_len < 18){
        current[curr_len] = '5';
      }
      else if (curr_len == 18){
        delay(1);
      }
      else {
        current[0] = '5';
      }
      delay(100);
    }
    //6
    if (buttons[10].contains(p.x, p.y)){
      buttons[10].drawButton(true);
      delay(10);
      buttons[10].drawButton(false);
            if(cf_flag==1){
        lcd.clear();
        cf_flag=0;
        for(i=0;i<20;i++){
          current[i]='\0';
        }
      }
      curr_len = det_length(current);
      if (curr_len > 0 && curr_len < 18){
        current[curr_len] = '6';
      }
      else if (curr_len == 18){
        delay(1);
      }
      else {
        current[0] = '6';
      }
      delay(100);
    }
    //7
    if (buttons[12].contains(p.x, p.y)){
      buttons[12].drawButton(true);
      delay(10);
      buttons[12].drawButton(false);
            if(cf_flag==1){
        lcd.clear();
        cf_flag=0;
        for(i=0;i<20;i++){
          current[i]='\0';
        }
      }
      curr_len = det_length(current);
     if (curr_len > 0 && curr_len < 18){
        current[curr_len] = '7';
      }
      else if (curr_len == 18){
        delay(1);
      }
      else {
        current[0] = '7';
      }
      delay(100);
    }
    //8
    if (buttons[13].contains(p.x, p.y)){
      buttons[13].drawButton(true);
      delay(10);
      buttons[13].drawButton(false);
            if(cf_flag==1){
        lcd.clear();
        cf_flag=0;
        for(i=0;i<20;i++){
          current[i]='\0';
        }
      }
      curr_len = det_length(current);
      if (curr_len > 0 && curr_len < 18){
        current[curr_len] = '8';
      }
      else if (curr_len == 18){
        delay(1);
      }
      else {
        current[0] = '8';
      }
      delay(100);
    }
    //9
    if (buttons[14].contains(p.x, p.y)){
      buttons[14].drawButton(true);
      delay(10);
      buttons[14].drawButton(false);
      if(cf_flag==1){
        lcd.clear();
        cf_flag=0;
        for(i=0;i<20;i++){
          current[i]='\0';
        }
      }
      curr_len = det_length(current);
      if (curr_len > 0 && curr_len < 18){
        current[curr_len] = '9';
      }
      else if (curr_len == 18){
        delay(1);
      }
      else {
        current[0] = '9';
      }
      delay(100);
    }
    //Clear
    if (buttons[16].contains(p.x, p.y)){
      buttons[16].drawButton(true);
      delay(10);
      buttons[16].drawButton(false);
      for(i=0;i<20;i++){
        current[i] = '\0';
        operand[i] = '\0';
      }
       
      npv=0;
      duration=0;
      lcd.clear();
      delay(100);
    }

  }

  
  //for (i=0; i<det_length(current); i++){
    //tft.setTextSize(4);
    //tft.setCursor(x_val, 20);
    //tft.print(current[i]);
    //x_val = x_val+23;
    
  //}
  if (bond_mode == 1){
    if(bond_current == 0){
      lcd.setCursor(0,0);
      lcd.print("IntRate: ");
      lcd.setCursor(0,1);
      lcd.print("As Decimal");
      lcd.setCursor(0,2);
      lcd.print("Per Period");
    }
    else if(bond_current == 1){
      lcd.setCursor(0,0);
      lcd.print("CpnRate: ");
      lcd.setCursor(0,1);
      lcd.print("As Decimal");
      lcd.setCursor(0,2);
      lcd.print("Per Period");
    }
    else if(bond_current == 2){
      lcd.setCursor(0,0);
      lcd.print("N: ");
      lcd.setCursor(0,1);
      lcd.print("Pmts remaining");
    }
    else if(bond_current == 3){
      lcd.setCursor(0,0);
      lcd.print("t\T: ");
      lcd.setCursor(0,1);
      lcd.print("t=days from last pmt");
      lcd.setCursor(0,2);
      lcd.print("T=days in cpn prd");
    }
    else if (bond_current == 4){
      lcd.setCursor(0,0);
      lcd.print("Dur: ");
      lcd.setCursor(0,1);
      lcd.print("ENTER to compute");
    }
    else if (bond_current == 5){
      lcd.setCursor(0,0);
      lcd.print("M.Dur: ");
      lcd.setCursor(0,1);
      lcd.print("ENTER to compute");
    }
    else if (bond_current == 6){
      lcd.setCursor(0,0);
      lcd.print("Conv: ");
      lcd.setCursor(0,1);
      lcd.print("ENTER to compute");
      lcd.setCursor(0,2);
      lcd.print("Val is per period");
      lcd.setCursor(0,3);
      lcd.print("/prdicity^2 for ann");
    }
  }
  if (CF_mode==1 && num_cf!=0 && cf_i != 0 && npv==0){
    lcd.setCursor(0,0);
    lcd.print("CF");
    lcd.print(cf_count);
    lcd.print(": ");
  }
  if (CF_mode == 1 && num_cf == 0 && cf_i == 0 && npv==0){
    lcd.setCursor(0,0);
    lcd.print("# of CF: ");
    lcd.setCursor(0,1);
    lcd.print("Incl. Init Outlay");
  }
  if (CF_mode == 1 && num_cf != 0 && cf_i == 0 && npv==0){
    lcd.setCursor(0,0);
    lcd.print("I%: ");
  }
  if (npv != 0){
    lcd.setCursor(0,0);
    lcd.print("NPV: ");
  }
  
  for (i=0; i<det_length(current); i++){
    if (operation==2 or operation == 4){
      lcd.setCursor(i+2, 1);
      lcd.print(current[i]);
    }
    else if (bond_mode == 1){
      if (bond_current == 0 || bond_current == 1){
        lcd.setCursor(9+i, 0);
        lcd.print(current[i]);
      }
      else if(bond_current==2){
        lcd.setCursor(i+3, 0);
        lcd.print(current[i]);
      }
      else if(bond_current==3){
        lcd.setCursor(i+5, 0);
        lcd.print(current[i]);
      }
      else if (bond_current == 4){
        lcd.setCursor(i+5,0);
        lcd.print(current[i]);
      }
      else if (bond_current == 5){
        lcd.setCursor(i+7, 0);
        lcd.print(current[i]);
      }
      else if (bond_current == 6){
        lcd.setCursor(i+6, 0);
        lcd.print(current[i]);
      }
    }
    else if (CF_mode == 1 && npv != 0){
      lcd.setCursor(i+5, 0);
      lcd.print(current[i]);
    }
    else if (CF_mode == 1 && num_cf != 0 && cf_i != 0 && npv == 0){
      lcd.setCursor(i+5, 0);
      lcd.print(current[i]);
    }
    else if (CF_mode == 1 && num_cf == 0 && cf_i == 0 && npv == 0){
      lcd.setCursor(i + 9, 0);
      lcd.print(current[i]);
    }
    else if (CF_mode == 1 && num_cf != 0 && cf_i == 0 && npv == 0){
      lcd.setCursor(i+5, 0);
      lcd.print(current[i]);
    }
    else {
      lcd.setCursor(i, 0);
      lcd.print(current[i]);
    }
  }
  for (i=0; i<det_length(operand); i++){
    if (operation == 2 or operation==4){
      lcd.setCursor(i, 0);
    }
    else{
      lcd.setCursor(i+2, 1);
    }
    lcd.print(operand[i]);
  }
  if (operation == 1){
    lcd.setCursor(0, 1);
    lcd.print("+");
  }
  else if (operation == 2){
    lcd.setCursor(0,1);
    lcd.print("-");
  }
  else if (operation == 3){
    lcd.setCursor(0,1);
    lcd.print("X");
  }
  else if (operation == 4){
    lcd.setCursor(0,1);
    lcd.print("/");
  }
  
  
  if (num != 0){
    //tft.setCursor(20, 100);
    //tft.setTextSize(1);
    char n_string[20];
    dtostrf(num, 2, 2, n_string); 
    char n_string2[20] = "N=";
    strcat(n_string2, n_string); 
    lcd.setCursor(0,3);
    lcd.print(n_string2);
    //tft.print(n_string2);
  }

  if (rate != 0){
    //tft.setCursor(100,100);
    //tft.setTextSize(1);
    char rate_string[20];
    dtostrf(rate, 2, 4, rate_string);
    char rate_string2[20] = "I=";
    strcat(rate_string2, rate_string);
    lcd.setCursor(9, 3);
    lcd.print(rate_string2);
    //tft.print(rate_string2);
  }
  /*
  if (pmt != 0){
    //tft.setCursor(180,100);
    //tft.setTextSize(1);
    char pmt_string[20];
    dtostrf(pmt, 2, 4, pmt_string);
    char pmt_string2[20] = "PMT = ";
    strcat(pmt_string2, pmt_string);
    lcd.setCursor(12, 2);
    lcd.print(pmt_string2);
    //tft.print(pmt_string2);
  }
  */
  if (pv != 0){
    //tft.setCursor(20,115);
    //tft.setTextSize(1);
    char pv_string[20];
    dtostrf(pv, 2, 4, pv_string);
    char pv_string2[20] = "PV = ";
    strcat(pv_string2, pv_string);
    lcd.setCursor(0, 2);
    lcd.print(pv_string2);
    //tft.print(pv_string2);
  }
  
  if (fv != 0){
    //tft.setCursor(200,115);
    //tft.setTextSize(1);
    char fv_string[20];
    dtostrf(fv, 2, 4, fv_string);
    char fv_string2[20] = "FV = ";
    strcat(fv_string2, fv_string);
    lcd.setCursor(0, 1);
    lcd.print(fv_string2);
    //tft.print(fv_string2);
  }
  delay(10);
  //Serial.print(current);
  //Serial.print("\t");
  //Serial.print(curr_len);
  //Serial.print("\n");
}




int det_length(char current[20]){
  int i, curr_len=0;
  
  for (i=0;i<strlen(current);i++){
    if (current[i] != '\0'){
      curr_len = curr_len +1;
    }
    else {
      return curr_len;
    }
  }
}

int find_dec(char current[20])
{
  int i;

  for (i=0; i<strlen(current);i++){
    if (current[i] == '.'){
      return 1;
    }
  }
  return 0;
}

