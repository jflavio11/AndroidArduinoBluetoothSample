/**
 * Software for Arduino to controll a robotic arm.
 * 
 * Android app code is on Github: 
 * https://github.com/jflavio1/AndroidArduinoBluetoothSample
 * 
 * @author: Jose Flavio Quispe Irrazábal
 */
// para controlar servos
#include <Servo.h>
#include <ctype.h>

// para operaciones matematicas
#include <math.h>

#define pi (3.1416/180)
#define pii 3.1416
#define arm1 8
#define arm2 7
#define arm3 5

// calibracion
// para primer servo, el que esta en la base
#define Q1_0 22
#define Q1_45 58
#define Q1_90 95

// para segundo servo, primera articulacion tipo codo
#define Q2_0 75
#define Q2_90 180

// tercer servo
#define Q3_90 136
#define Q3_0 41


// primer servo, la base
Servo servo1;
int pinServo1 = 3;

// servo 2
Servo servo2;
int pinServo2 = 5;

// servo 3
Servo servo3;
int pinServo3 = 11;

int count = 0;

// para configurar bluetooth
#include <SoftwareSerial.h>
#define Rx 12
#define Tx 13
#define KEY 12
SoftwareSerial BTSerial(Rx, Tx);

// to save messages from Android app
int angleOrders [3] = { -2, -2, -2};
int c= 0;
char r[2] = {};
boolean complete = false;

void setup() {
  
  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo3.attach(pinServo3);

  digitalWrite(12, HIGH);
  
  BTSerial.begin(9600);
  BTSerial.flush();
  
  delay(500);
  Serial.begin(9600);
  Serial.println("READY");
}

void loop() {
  
  // Esperamos ha recibir datos.
  if (BTSerial.available()){
  
    // La funcion read() devuelve un caracter 
    char command = BTSerial.read();
    BTSerial.flush();
    //Serial.println(command);

    if(command != 'n'){
      r[c] = command;
      c++;
    }else{
      c = 0;
      complete = true;
    }

    if(complete){
      char buffer[2];
      buffer[0] = r[0];
      buffer[1] = r[1];
      
      int angle = atoi(buffer);
      
      if(count <3 ){
        angleOrders[count] = angle;
        int angle = angleOrders[count];
        //Serial.println(angle);
        if(count == 2){
          count = 0;
        } else {
          count++;
        }
      }

      complete = false;
      
    }
    
  }

  // if last is not null, means that we have received all orders
  if(angleOrders[2] != -2){

      int firstAngle = angleOrders[0];
      int secondAngle = angleOrders[1];
      int thirdAngle = angleOrders[2];
    
      if(firstAngle != -2){
        int angle1 = as1(angleOrders[0]);
        servo1.write(angle1);
      }
      
      if(secondAngle != -2){
        int angle2 = as2(angleOrders[1]);
        servo2.write(angle2);
      }
      
      if(thirdAngle != -2){
        int angle3 = as3(angleOrders[2]);
        servo3.write(angle3);
      }

      int x = posX(firstAngle, secondAngle, thirdAngle);
      int y = posY(firstAngle, secondAngle, thirdAngle);
      int z = posZ(firstAngle, secondAngle, thirdAngle);
      
      Serial.println(x);
      Serial.println(y);
      Serial.println(z);

      //moverXYZ(x, y, z);

      angleOrders[0] = -2;
      angleOrders[1] = -2;
      angleOrders[2] = -2;
  }

  //delay(500);
}

/**
 * Transforma el angulo dado a la escala del microservo 1.
 */
int as1(int angle){
  return map(angle, -45, 45, Q1_0, Q1_90);
}

/**
 * Transforma el ángulo dado a la escala del microservo 2.
 */
int as2(int angle){
  return map(angle, 0, 90, Q2_0, Q2_90);  
}

/**
 * Transforma el ángulo dado a la escala del microservo 3. 
 * Por motivos de calculo, 0 sera 90 grados y 90 serán 0 grados.
 */
int as3(int angle){
  return map(angle, 0, 90, Q3_90, Q3_0);
}

// region cinematica directa
/**
 * Retorna la posición en X dado los ángulos dos y tres.
 */
int posX(int q1, int q2, int q3){
  return cos(q1*pi)*( (arm2*cos(q2 * pi)) + (arm3*cos((q3*pi)-(q2*pi))) );
}

int posY(int q1, int q2, int q3){
  return sin(q1*pi) * ( (arm2*cos(q2*pi)) + (arm3*cos((q3*pi)-(q2*pi))));
}

int posZ(int q1, int q2, int q3){
  return arm1 + arm2*sin(q2*pi) - arm3*sin((q3*pi)-(q2*pi));
}

// endregion

// region cinematica inversa

void moverXYZ(double x, double y, double z){
  double h=z-arm1;
  double r=sqrt(pow(x,2)+pow(y,2));

  double cos3 = calc_coseno3(x, y, h, arm2, arm3);
  double sen3 = calc_seno3(cos3);
  
  double angle1 = calc_angle1(x, y);
  double angle2 = calc_angle2(h, r, arm3, sen3, arm2, cos3);
  double angle3 = calc_angle3(sen3, cos3);

  //Serial.println(angle1);
  //Serial.println(angle2);
  //Serial.println(angle3);
}

// CALCULO DE COSENO DE Q3
double calc_coseno3(double x, double y, double h, double l2, double l3){
  double temp1=pow(x,2)+pow(y,2)+pow(h,2)-pow(l2,2)-pow(l3,2);
  double temp2=2*l2*l3;
  int cq3=temp1/temp2;
  return cq3;
}

// CALCULO DE SENO DE Q3
double calc_seno3(double cq3){
  double temp=1-pow(cq3,2);
  int sq3p=sqrt(temp);
  return sq3p;
}

// CALCULO DE ANGULO Q1

double calc_angle1(double x, double y){
  return atan2(y,x)*180/pii;
}

// CALCULO DE ANGULO Q2
double calc_angle2(double h, double r, double l3, double sq3p, double l2, double cq3){
  double q2 = (atan2(h,r)-atan2(l3*sq3p , l2+l3*cq3))*180/pii;
  return q2;
}

// CALCULO DE ANGULO Q3
double calc_angle3(double sq3p, double cq3){
  double q3=atan2(sq3p,cq3)*180/pii;
  return q3;
}

// endregion
