//Added proportional algorithm in motorspeed value
//Added troubleshoot
//Added Wire library
//Replace delay with millis() and micros()
//DAISY CHAINED Max Sonar
//Added maxSonar class


#include <SPI.h>
#include <Pixy.h>
#include <Wire.h>
#include <EEPROM.h>

#define X_CENTER        ((PIXY_MAX_X-PIXY_MIN_X)/2)
#define Y_CENTER        ((PIXY_MAX_Y-PIXY_MIN_Y)/2)

Pixy pixy;

byte I2Caddr;

int MotorSpeed;

//PID calculation
const int Setpoint=15;
int Input,kp;
int32_t Output;

//For troubleshooting delay()
uint32_t pastTime=0;

class ServoLoop
{
public:
        ServoLoop(int32_t pgain, int32_t dgain);

        void update(int32_t error);

        int32_t m_pos;
        int32_t m_prevError;
        int32_t m_pgain;
        int32_t m_dgain;
};

ServoLoop panLoop(450,650);
ServoLoop tiltLoop(650,850);

ServoLoop::ServoLoop(int32_t pgain, int32_t dgain)
{
        m_pos = PIXY_RCS_CENTER_POS;
        m_prevError = 0x80000000L;
        m_pgain = pgain;
        m_dgain = dgain;
}

void ServoLoop::update(int32_t error)
{
        long int vel;
        if(m_prevError!=0x80000000){
                vel = (error * m_pgain + (error - m_prevError) * m_dgain) >> 10;
                m_pos += vel;
                if(m_pos>PIXY_RCS_MAX_POS){
                        m_pos=PIXY_RCS_MAX_POS;
                }
                else if(m_pos<PIXY_RCS_MIN_POS){
                        m_pos=PIXY_RCS_MIN_POS;
                }
        }
        m_prevError = error;
}

class maxSonar
{
public:
        maxSonar();
        void readSonar(const int pwPin);
        uint16_t pwDistance;
        unsigned long currentTime;
        unsigned long previousTime;
};

maxSonar leftSonar,rightSonar;

maxSonar::maxSonar()
{
        //unsigned long previousTime=0;
        previousTime=0;
}

void maxSonar::readSonar(const int pwPin)
{
        const int serialSonarPin=7;
        const int interval=100;
        pinMode(serialSonarPin,OUTPUT);
        digitalWrite(serialSonarPin,HIGH);
        digitalWrite(serialSonarPin,LOW);
        pinMode(serialSonarPin,INPUT);
        if(((currentTime=millis())-previousTime)>=interval){
                pwDistance=analogRead(pwPin)/2;
                previousTime=millis();
        }
}

void setup()
{
        Serial.begin(9600);
        Wire.begin(I2Caddress());
        Wire.onRequest(sendMotorSpeed);
        pixy.init();
}

byte  I2Caddress()
{

        byte n=EEPROM.read(0);
        if(n!=0x55){
                EEPROM.write(0,0x55);
                EEPROM.write(1,0x06);
        }
        I2Caddr=EEPROM.read(1);
        return I2Caddr;
}

void track_object()
{
    uint16_t blocks;
    int32_t panError, tiltError;

    blocks=pixy.getBlocks();

    if (blocks)
    {
        panError=X_CENTER-pixy.blocks[0].x;
        tiltError=pixy.blocks[0].y-Y_CENTER;

        panLoop.update(panError);
        tiltLoop.update(tiltError);

        pixy.setServos(panLoop.m_pos,tiltLoop.m_pos);
    }
}

void sendMotorSpeed()
{
        byte buffer[4];
        buffer[0]=MotorSpeed>>8;
        buffer[1]=MotorSpeed&0xFF;
        buffer[2]=Input>>8;
        buffer[3]=Input&0xFF;
        Wire.write(buffer,4);
}

void sonarPidCompute()
{
//      Input=analogRead(A0);
//      Input=map(Input,0,1024,0,50);
//      Input=constrain(Input,0,50);
        Input=(leftSonar.pwDistance);

        double error=Input-Setpoint;
        Output=kp*error;
        MotorSpeed=Output>>3;
	MotorSpeed=map(MotorSpeed,-188,425,0,255);
	MotorSpeed=constrain(MotorSpeed,0,255);

}

void sonarSetTuning(double Kp)
{
        kp=Kp;
}

void loop()
{
        track_object();

        leftSonar.readSonar(A2);
        rightSonar.readSonar(A1);

        sonarSetTuning(100);
        sonarPidCompute();
	
        troubleShoot();
}

void troubleShoot()
{
        uint32_t currentTime;
        const int interval=500;
        if((currentTime=millis()-pastTime)>=interval){
        //      Serial.print(leftSonar.pwDistance);
        //      Serial.print(potentiometer);
        //      Serial.print("\t");
        //      Serial.print(leftSonarError);
        //      Serial.print("\t");
        //      Serial.print(proportional);
        //      Serial.print("\t");
        //      Serial.print(integral);
        //      Serial.print("\t");
        //      Serial.println(MotorSpeed);
        //      Serial.print(leftSonar.pwDistance);
        //      Serial.print("\t");
        //      Serial.println(rightSonar.pwDistance);
                Serial.print(Input);
                Serial.print("\t");
                Serial.println(MotorSpeed);
                pastTime=millis();
        }
}
