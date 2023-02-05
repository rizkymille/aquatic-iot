#include "AutoPID.h"

#include "DS18B20.h"
#include "OneWireHub.h"
#include "DFRobot_PH.h"
#include "DTH_Turbidity.h"

#include "TaskScheduler.h"

#include "Servo.h"

#define PIN_PH_SENS -1
#define PIN_TEMP_SENS -1
#define PIN_WATER_LVL_SENS A1
#define PIN_TURBIDITY_SENS A0

#define PIN_ACTUATOR_PUMP -1
#define PIN_ACTUATOR_FEEDER -1

#define ADC_BIT 10
#define ADC_VOLTAGE 5

#define SERVO_INIT_POS -180

struct {
  double ph = 7;
  double water_level = 50;
} setpoint;

struct {
  float ph = 5;
  float water_level = 10;
} start_threshold;

constexpr int ADC_BYTE = pow(2, ADC_BIT);

// === Constructors == //
DTH_Turbidity turb_sens(PIN_TURBIDITY_SENS);
DFRobot_PH ph_sens;
OneWireHub temp_wire(PIN_TEMP_SENS);
DS18B20 temp_sens(DS18B20::family_code, 0x00, 0x00, 0xB2, 0x18, 0xDA, 0x00);

Servo feeder_servo;

Scheduler scheduler;
Task *controlTask;
Task *readSensorTask;

String sendbuffer;

struct {
  double temp;
  double ph;
  double water_level;
  double turb;
} sens_data;

double pump_mv;
double feeder_mv;

double PIDgain_pump[3] = { 0.1, 0, 0 };  // kp, ki, kd
double PIDrange_pump[2] = { 0, 256 };    // min, max

double PIDgain_feeder[3] = { 0.1, 0, 0 };
double PIDrange_feeder[2] = { -180, 180 };

unsigned long PIDtimestep = 50;  // in milliseconds

AutoPID pump_controller(&sens_data.water_level, &setpoint.water_level, &pump_mv,
                        PIDrange_pump[0], PIDrange_pump[1], PIDgain_pump[0], PIDgain_pump[1], PIDgain_pump[2]);
AutoPID feeder_controller(&sens_data.ph, &setpoint.ph, &feeder_mv,
                          PIDrange_feeder[0], PIDrange_feeder[1], PIDgain_feeder[0], PIDgain_feeder[1], PIDgain_feeder[2]);

String parseMsg(String &msg) {
  size_t space = msg.indexOf(" ");
  String text = msg.substring(0, space);
  msg = msg.substring(space + 1, msg.length());
  return text;
}

void tuneOption(String &msg) {
  String command = parseMsg(msg);
  String command2 = parseMsg(msg);
  String type = parseMsg(msg);
  if (command.equals("SET")) {
    AutoPID *pidptr;
    double *PIDgainptr[3];
    double *PIDrangeptr[2];

    if (command2.equals("PUMP")) {
      for (int i = 0; i < 3; i++) PIDgainptr[i] = &PIDgain_pump[i];
      for (int i = 0; i < 2; i++) PIDrangeptr[i] = &PIDrange_pump[i];
      pidptr = &pump_controller;
    }
    else if (command2.equals("FEEDER")) {
      for (int i = 0; i < 3; i++) PIDgainptr[i] = &PIDgain_feeder[i];
      for (int i = 0; i < 2; i++) PIDrangeptr[i] = &PIDrange_feeder[i];
      pidptr = &feeder_controller;
    }
    else if (command2.equals("TIMESTEP")) {
      PIDtimestep = parseMsg(msg).toInt();
      pidptr->setTimeStep(PIDtimestep);
      return;
    }

    if (type.equals("GAINS")) {
      for (int i = 0; i < 3; i++) *PIDgainptr[i] = parseMsg(msg).toDouble();
      pidptr->setGains(*PIDgainptr[0], *PIDgainptr[1], *PIDgainptr[2]);
    } 
    else if (type.equals("RANGE")) {
      for (int i = 0; i < 2; i++) *PIDrangeptr[i] = parseMsg(msg).toDouble();
      pidptr->setOutputRange(*PIDrangeptr[0], *PIDrangeptr[1]);
    } 
    else return;
  } 
  else if (command.equals("GET")) {
    double *PIDgainptr[3];
    double *PIDrangeptr[2];
    if (command2.equals("PUMP")) {
      for (int i = 0; i < 3; i++) PIDgainptr[i] = &PIDgain_pump[i];
      for (int i = 0; i < 2; i++) PIDrangeptr[i] = &PIDrange_pump[i];
    } 
    else if (command2.equals("FEEDER")) {
      for (int i = 0; i < 3; i++) PIDgainptr[i] = &PIDgain_feeder[i];
      for (int i = 0; i < 2; i++) PIDrangeptr[i] = &PIDrange_feeder[i];
    } 
    else if (command2.equals("TIMESTEP")) {
      sendbuffer = "TIMESTEP/DT: " + String(PIDtimestep);
      Serial.println(sendbuffer);
      return;
    } 
    else return;

    if (type.equals("GAINS")) {
      sendbuffer = "KP: " + String(*PIDgainptr[0]) + " | KI : " + String(*PIDgainptr[1]) + " | KD: " + String(*PIDgainptr[2]);
    } else if (type.equals("RANGE")) {
      sendbuffer = "MIN: " + String(*PIDrangeptr[0]) + " | MAX : " + String(*PIDrangeptr[1]);
    } else return;
    Serial.println(sendbuffer);
  }
}

void readSensor() {
  // readsensor task
  sens_data.temp = temp_sens.getTemperature();  // get aquatic temperature
  sens_data.ph = ph_sens.readPH(analogRead(PIN_PH_SENS) * ADC_VOLTAGE / ADC_BYTE, sens_data.temp);
  sens_data.water_level = (0.0184*analogRead(PIN_WATER_LVL_SENS) - 10.37)*100/3.3;
  //Serial.println(sens_data.water_level);
  sens_data.turb = turb_sens.readTurbidity();
  sendbuffer = String(sens_data.temp) + "/" + String(sens_data.ph) +"/" + String(sens_data.water_level) + "/" + String(sens_data.turb);
  Serial3.println(sendbuffer);
}


void control() {
  // control switching
  if (sens_data.water_level < start_threshold.water_level) {
    pump_controller.run();
    analogWrite(PIN_ACTUATOR_PUMP, pump_mv);
  } else if (pump_controller.atSetPoint(0.2)) {
    pump_controller.stop();
  }

  if (sens_data.ph < start_threshold.ph) {
    feeder_controller.run();
    feeder_servo.write(feeder_mv);
  } else if (feeder_controller.atSetPoint(0.5)) {
    feeder_controller.stop();
  }
}

void setup() {
  Serial.begin(115200);
  Serial3.begin(115200);

  temp_wire.attach(temp_sens);
  ph_sens.begin();
  pinMode(PIN_WATER_LVL_SENS, INPUT);

  pinMode(PIN_ACTUATOR_PUMP, OUTPUT);

  feeder_servo.attach(PIN_ACTUATOR_FEEDER);
  feeder_servo.write(SERVO_INIT_POS);

  // controller init
  pump_controller.setTimeStep(PIDtimestep);
  feeder_controller.setTimeStep(PIDtimestep);

  scheduler.init();

  controlTask = new Task(PIDtimestep * TASK_MILLISECOND, TASK_FOREVER, &control);
  readSensorTask = new Task(5 * TASK_SECOND, TASK_FOREVER, &readSensor);
  /*
  should've add delete pointers but arduino has no exit condition
  delete controlTask
  delete readSensorTask
  */

  scheduler.addTask(*controlTask);
  scheduler.addTask(*readSensorTask);

  controlTask->enable();
  readSensorTask->enable();
}

void loop() {
  scheduler.execute();
}

// read serial task
void serialEvent() {
  if (Serial.available() > 0) {
    /*
    interactive: GET GAINS PUMP 
    or SET GAINS PUMP 0.7 0.1 0.5
    */
    String msg = Serial.readString();
    msg.trim();
    
    tuneOption(msg);
  }
}
