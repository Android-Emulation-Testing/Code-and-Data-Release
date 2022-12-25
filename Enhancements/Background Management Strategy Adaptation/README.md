# Background Management Strategy Adaptation

## Background

In our measurement of the fidelity of virtualized devices with respect to the testing of global-scale apps,
we discover that 1% of the captured failure events occur
much more frequently (up to 1025×) on physical devices
than on virtualized devices.
We find out that such discrepancies root in Chinese vendors’ aggressive strategy for suppressing the background activities of apps.

## The Problem

The customized Android systems of some vendors (including Xiaomi, Huawei, Vivo, OPPO, Honor, and Redmi) aggressively throttle certain background apps by simultaneously killing all the processes in the app’s process group using the `forceStopPackage()` method provided by `ActivityManagerService`, without giving any warning or grace period to the app.
This can easily trigger resource leakage or corruption in the app’s data files or databases, thus leading to more frequent failures on the physical devices than on virtualized devices running AOSP (i.e., creating a vicious circle).

## Our Solution

In our communications with the vendors in question, the vendors are reluctant to modify their strategies in favor of their users’ interests and experiences.
Thus, to practically reduce the frequency discrepancies, we selectively adapt the background management strategy on the associated virtualized devices, by customizing the `ActivityManagerService` (which is the system service that manages all app activities in Android) in their AOSP systems.
The full list of files we modify is listed below. The modified files are available in the current folder as well.

| File | Changed Symbols | Purpose | Location in AOSP |
| ---- | ---- | ---- | ---- |
|   [`ActivityManagerService.java`](ActivityManagerService.java)   |   `appDiedLocked`   |  Atomic Process Group Killing  | `frameworks/base/services/core/java/com/android/server/am/ActivityManagerService.java` |
|   [`processgroup.cpp`](processgroup.cpp)   |   `KillProcessGroup`   |  Atomic Process Group Killing  | `system/core/libprocessgroup/processgroup.cpp` |
|   [`ActivityManagerConstants.java`](ActivityManagerConstants.java)   |   `DEFAULT_BACKGROUND_SETTLE_TIME, DEFAULT_CONTENT_PROVIDER_RETAIN_TIME`   |  Background Management Strategy Tuning  | `frameworks/base/services/core/java/com/android/server/am/ActivityManagerConstants.java` |
|   [`config.xml`](config.xml)   |   `config_lowMemoryKillerMinFreeKbytesAbsolute, config_extraFreeKbytesAbsolute`   |  Background Management Strategy Tuning  | `frameworks/base/core/res/res/values/config.xml` |
|   [`device.mk`](device.mk)   |   `PRODUCT_PROPERTY_OVERRIDES`   |  Background Management Strategy Tuning   | `device/google/cuttlefish/shared/device.mk` |


Note: the above enhancement is based on the `android10-release` branch, since, at the time of our study, Android 10 is the most prevalent version among the users of T-video.
The related logic in other Android versions stay largely unchanged, and therefore you can port the fix to any recent Android version that you desire. 