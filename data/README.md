## Measurement Data

This directory contains the measurement data of failure events with proper anonymization.

## Data Format

The attributes of each failure event are organized as follows:

|  Attribute   | Description  |
|  ----  | ----  |
| error  | The triggered exception/signal of the failure. |
| scene  | A brief summary of the anonymized failure scene. |
| os_version  | The Android version of the device producing the failure. |
| device_brand  | The brand of the device producing the failure. |
| device_model  | The model of the device producing the failure. |
| device_type  | The type of the device. Specifically, `physical` denotes a physical device and `virtualized` denotes a virtualized device. |
| failure_layer  | The layer in which the failure occurred. Specifically, `java` and `native` denote that the failure occurred in the Java or the native layer, respectively. |