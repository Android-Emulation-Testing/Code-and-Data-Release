# Code and Data Release for High-Fidelity Mobile App Testing on Virtualized Devices at Scale

![license](https://img.shields.io/badge/Platform-Android-green "Android")
![license](https://img.shields.io/badge/Version-Beta-yellow "Version")
![license](https://img.shields.io/badge/Licence-Apache%202.0-blue.svg "Apache")

This repository contains the code and data we release to enable high-fidelity mobile app testing on virtualized devices at scale.
They are organized as follows.

```
Code-and-Data-Release
|---- Failure Scene Capture
|---- Enhancements
      |---- Graphics Resource Format Extension
      |---- Background Management Strategy Adaptation
      |---- Dynamic Binary Patching
|---- Measurement Data
```

### Code Release

#### Failure Scene Capture

To effectively capture failure scenes, we propose a considerate method that combines content-aware memory image pruning with failsafe data collection.
We have provided the source code of the failure scene capture mechanisms in the [`Failure Scene Capture` folder](https://github.com/Android-Emulation-Testing/Code-and-Data-Release/tree/main/Failure%20Scene%20Capture).

#### Enhancements

In order to effectively enhance the testing fidelity on virtualized devices, we have devised threefold enhancements that eliminate most of the failure discrepancies in reality:

 * Graphics Resource Format Extension
 * Background Management Strategy Adaptation
 * Dynamic Binary Patching

We have provided the source code of the above enhancements in the [`Enhancements` folder](https://github.com/Android-Emulation-Testing/Code-and-Data-Release/tree/main/Enhancements).

### Data Release

We have provided in part the measurement data (with proper anonymization) in the [`Measurement Data` folder](https://github.com/Android-Emulation-Testing/Code-and-Data-Release/tree/main/Measurement%20Data). 
We will release the full dataset as soon as we obtain official approval of the relevant authorities.

#### Data Format

The data file is organized in `.csv` format. 
Each row corresponds to a failure event.
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
