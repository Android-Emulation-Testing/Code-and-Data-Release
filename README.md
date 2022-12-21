# Code and Data Release

This repo contains the code and data involved in our study.

## Code Release

### Failure Scene Capture

We have provided the source code of the failure scene capture mechanisms mentioned in our paper in the [`capture` folder](https://github.com/Android-Emulation-Testing/code-and-data/tree/main/capture).

### Enhancements

We have provided the source code of the enhancements mentioned in our paper in the [`enhancements` folder](https://github.com/Android-Emulation-Testing/code-and-data/tree/main/enhancements).

## Data Release

We have provided in part the measurement data (with proper anonymization) in the [`data` folder](https://github.com/Android-Emulation-Testing/code-and-data/tree/main/data). 
We will release the full dataset as soon as we obtain official approval of the relevant authorities.

### Data Format

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
