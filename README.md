LILYGO T-Display-S3 KY-001 Temperature Sensor Module Project

This code reads the digital signal from the KY-001 module and displays the temperature on the built-in screen of the LilyGO T-Display-S3 using the TFT_eSPI library. The screen is only updated if there is a change in the sensor reading.

Pin Connections:
 - KY-001 VCC  -> 3.3V
 - KY-001 GND  -> GND
 - KY-001 S    -> GPIO1

DS18B20 Specifications:
 - Communicates over one-wire bus communication
 - Power supply range: 3.0V to 5.5V
 - Resolution: 9-bit to 12-bit
 - Operating temperature range: -55ºC to +125ºC
 - Accuracy +/-0.5 ºC (between the range -10ºC to 85ºC)
