MoistureLevel selectMoistureLevel(MoistureLevel a, MoistureLevel b);
MoistureLevel getCapacitiveMoistureLevel(int value);
MoistureLevel getResistiveMoistureLevel(int value);
const String moistureLevelToString(MoistureLevel level);

float getTemperatureFromSensor();
int getCapacitiveSensorValue();
int getResistiveSensorValue();

void turnPumpOn();
void turnPumpOff();
