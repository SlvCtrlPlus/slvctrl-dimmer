#include <dimmable_light.h>
#include <SerialCommands.h>

const char* DEVICE_TYPE = "dimmer";
const int FW_VERSION = 10000; // 1.00.00
const int PROTOCOL_VERSION = 10000; // 1.00.00

const int OUTPUT_PIN = 1;
const int ZERO_CROSS_PIN = 0;

DimmableLight dimmer(OUTPUT_PIN);

int currentPowerPercentage = 0;

char serial_command_buffer[32];
SerialCommands serialCommands(&Serial, serial_command_buffer, sizeof(serial_command_buffer), "\n", " ");

void setup()
{
  Serial.begin(9600);
  
  DimmableLight::setSyncPin(ZERO_CROSS_PIN);
  DimmableLight::begin();
  dimmer.setBrightness(0);
  
  serialCommands.SetDefaultHandler(commandUnrecognized);
  serialCommands.AddCommand(new SerialCommand("introduce", commandIntroduce));
  serialCommands.AddCommand(new SerialCommand("attributes", commandAttributes));
  serialCommands.AddCommand(new SerialCommand("status", commandStatus));
  serialCommands.AddCommand(new SerialCommand("get-power", commandGetPower));
  serialCommands.AddCommand(new SerialCommand("set-power", commandSetPower));
  serialCommands.AddCommand(new SerialCommand("test", commandTest));
}

void loop()
{
  serialCommands.ReadSerial();
}

void commandUnrecognized(SerialCommands* sender, const char* cmd)
{
  serial_printf(sender->GetSerial(), "Unrecognized command [%s]\n", cmd);
}

void commandIntroduce(SerialCommands* sender)
{
  serial_printf(sender->GetSerial(), "introduce;%s,%d,%d\n", DEVICE_TYPE, FW_VERSION, PROTOCOL_VERSION);
}

void commandAttributes(SerialCommands* sender)
{
  serial_printf(sender->GetSerial(), "attributes;power:rw[0-100]\n");
}

void commandStatus(SerialCommands* sender) {
  serial_printf(sender->GetSerial(), "status;power:%s\n", currentPowerPercentage);
}


void commandGetPower(SerialCommands* sender)
{
  serial_printf(sender->GetSerial(), "get-power;%d;status:successful\n", currentPowerPercentage);
}

void commandSetPower(SerialCommands* sender)
{
  char* percentageStr = sender->Next();

  if (percentageStr == NULL) {
    sender->GetSerial()->println("set-power;;status:failed,reason:percentage_param_missing\n");
    return;
  }

  int percentage = atoi(percentageStr);

  if (percentage < 0 || percentage > 100) {
    serial_printf(sender->GetSerial(), "set-power;%d;status:failed,reason:percentage_out_of_range\n", percentage);
    return;
  }

  currentPowerPercentage = percentage;
  int currentPower = map(percentage, 0, 100, 0, 255);

  dimmer.setBrightness(currentPower);

  serial_printf(sender->GetSerial(), "set-power;%d;status:successful\n", currentPowerPercentage);
}

void commandTest(SerialCommands* sender)
{
  for (int i = 0; i <= 255; i++) {
    dimmer.setBrightness(i);

    serial_printf(sender->GetSerial(), "test;%d;status:successful\n", i);

    delay(50);
  }

  dimmer.setBrightness(0);
}
