# Failure Scene Capture

## Introduction

Effective failure scene capturing in our largescale context for testing a production app needs to satisfy three unique requirements as follows.

* Failure scene data should be captured as comprehensively as possible to help debug the functional problems.
* Failure capturing should be lightweight; otherwise, large-scale and comprehensive data collection can easily overload our test devices and data servers.
* The failure capturing operation in itself should be reliable without incurring secondary failures that could corrupt the scene and thus hinder our later analysis.

However, existing tools like Android Vitals and xCrash fail to satisfy the three requirements simultaneously. To fulfill all these requirements, we devise a considerate failure scene capturing method that combines content-aware memory image pruning with failsafe data collection.

## Enhancements

Our method enhance existing tools from the two aspects listed below.
### Memory Image Pruning and Recording

**The Motivation**

In practice, production-level failure scene capturing tools focus on capturing three-fold information upon failures: 
1) Android logcat that contains apps’ log outputs, 
2) system resources like the opened file descriptors,
3) execution contexts like the call stacks and register values.

However, we find that a major problem of them is the lack of in-situ memory data, which contain important debugging information (such as the corrupted memory data) for the analysis of various non-standard failure issues with regard to the proprietary hardware/software components of certain phone vendors.

**Our Solution**

To realize comprehensive in-situ data capturing, we resort to common practices by `coredump`ing the full image of the
process memory.
However, conventional coredump images could easily lead to hundreds of MBs of data for a single failure event. 
To essentially reduce the excessive storage overhead of dumping the entire image of the process memory, we carefully prune the image by removing redundant and non-critical data, including the Java VM memory in the presence of a fully-native failure, static resources such as fonts that can be recovered even after failures, inaccessible private memory segments, and unused sparse space in the thread stack. 
This can achieve 15× to 103× reduction of storage overhead in practice, and thus the size of the dumped memory image becomes smaller than 3 MB after conventional `gzip` compression.

###  Failsafe Data Collection

**The Motivation**

Existing tools often rely on a single failure capturing process for data collection, which is prone to secondary failures as system resources are oftentimes rather limited and the contexts are extremely volatile upon failures.

**Our Solution**

To address this, every time a fatal exception or signal is intercepted, four dedicated native processes are launched within the app to capture four-fold in-situ information (i.e., the threefold information collected by existing tools and the memory images captured by us) respectively. 
This allows effective failure isolation among the processes, i.e., even when one process fails, the other living processes can still extract useful (and usually sufficient) information. We also insert safeguards (i.e., fatal signal catchers) into the four data collection processes, which can further intercept fatal signals to allow best-effort self recovery or event logging upon secondary failures.

## Implemention

We implement our failure scene capturing mechanisms by making enhancements to [xCrash](https://github.com/iqiyi/xCrash), a popular open-source failure capture tool for Android.
The full list of changed files is listed below.
To ease lookup, our modifications are marked with `Android-EMU: start of modification` and `Android-EMU: end of modification`.

| File | Added/Changed Symbols | Purpose | Location in xCrash |
| ---- | ---- | ---- | ---- |
|   [`xcd_maps.c`](xcd_maps.c)   |   `dump_size` (added)   |  Prune the memory image  | `xcrash_lib/src/main/cpp/xcrash_dumper/xcd_maps.c` |
|   [`xcd_maps.c`](xcd_maps.c)   |   `fc_coredump_memory` (added)  |  Dump the memory image  | `xcrash_lib/src/main/cpp/xcrash_dumper/xcd_maps.c` |
|   [`xcd_process.c`](xcd_process.c)   |   `record_signal_handler` (added)  |  Safeguard  | `xcrash_lib/src/main/cpp/xcrash_dumper/xcd_process.c` |
|   [`xcd_process.c`](xcd_process.c)   |   `record_safeguard` (added)  |  Safeguard  | `xcrash_lib/src/main/cpp/xcrash_dumper/xcd_process.c` |
|   [`xcd_process.c`](xcd_process.c)   |   `xcd_process_record` (changed)  |  Capture the four-fold in-situ information  | `xcrash_lib/src/main/cpp/xcrash_dumper/xcd_process.c` |

Note: the source code we provide in this directory is based on commit [`457066c`](https://github.com/iqiyi/xCrash/commit/457066ceb48fb84b993f1f04871d9e634d752792), the most recent commit of `xCrash` on the `master` branch at the time of our implementation. The library `tvideo_utils.h` imported in `xcd_maps.c` (to get the offset of the memory image in the `coredump` file) and `xcd_process.c` (to check whether a failure is fully-native) is a commercial closed source library from T-video. We will release the code after we get authorization.
