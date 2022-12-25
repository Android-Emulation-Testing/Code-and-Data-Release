# Graphics Resource Format Extension

## Background

When testing the 10 global-scale apps mentioned in our paper, a few false positive failures only manifest on virtualized devices. 
Our analysis reveals that they are closely related to the graphics subsystem of virtualized devices. 
As a consequence, the vast majority (87.8%) of such failure events occur on the video streaming components of Android apps, which heavily depend on the graphics subsystem.

## The Problem

In detail, the root cause of these graphics-related failures lies in the implicit differences between the graphics resources used by Android and the underlying emulators.
To accelerate graphics rendering, our deployed Cuttlefish GAEs send graphics resources produced within Android to the host (ARM server) side so that the host GPU can be multiplexed for rendering them.
However, the host-side graphics subsystem’s definition of certain resource formats (e.g., the YV12 format for video frames) are different from those defined in Android in some implicit aspects.

Take YV12 as an example,
discrepencies exist in how the AOSP guest OS and the `minigbm` host-side graphics library handle certain graphics resources (i.e., YV12 buffer height alignment).
In [Android specifications](https://developer.android.com/reference/android/graphics/ImageFormat#YV12), YV12 buffer heights are unaligned, but in the underlying minigbm driver, buffer heights are aligned to 8 bytes (cf. [virtio_dumb_bo_create()](https://cs.android.com/android/platform/superproject/+/android-10.0.0_r47:external/minigbm/virtio_gpu.c;drc=abe44f62208cfaf1b329703d9043b1004baffb44;l=67) and [drv_bo_from_format()](https://cs.android.com/android/platform/superproject/+/android-10.0.0_r47:external/minigbm/helpers.c;drc=6bd7885bcfc2bb64fd2c532e1a83fd5d38fd981b;l=239). 
Some other backends (e.g., `msm`, `tegra`) use various alignment sizes as well. This creates inconsistencies that will ultimately lead to overruns and invalid accesses when these buffers are used in the Android framework.

## Our Solution

To address this issue, we extend the resource format within the host-side graphics library (used by emulators) by adding Android-specific formats
to the host-side graphics library (i.e., `minigbm`) based on the resource definitions of Android.
We have provided our modifications to the `minigbm` library in this folder.
Key changes to the library involve the use of unaligned heights in [`helpers.c`](helpers.c) and [`gbm.c`](gbm.c), and a synchronization of host's graphics capabilities with the guest in [`virtio_gpu.c`](virtio_gpu.c).


| File | Changed Symbols | Purpose | Location in AOSP |
| ---- | ---- | ---- | ---- |
|   [`helpers.c`](helpers.c)   |   `drv_dumb_bo_create`   |   Use unaligned heights in YV12 buffer creation  | `external/minigbm/helpers.c` |
|   [`gbm.c`](gbm.c)   |   `gbm_bo_create`   |   Use unaligned heights in YV12 buffer creation  | `external/minigbm/gbm.c` |
|   [`virtio_gpu.c`](virtio_gpu.c)   |   `virtio_gpu_priv, translate_format, virtio_gpu_init, virtio_gpu_supports_format, virtio_gpu_add_combination, virtio_gpu_add_combinations, virtio_gpu_get_caps`   |  Sync the host's graphics capabilities with the guest  | `external/minigbm/virtio_gpu.c` |
|   [`virgl_hw.h`](virgl_hw.h)   |   `VIRGL_FORMAT_YV12, VIRGL_FORMAT_YV16, VIRGL_FORMAT_IYUV, VIRGL_FORMAT_NV12, VIRGL_FORMAT_NV21`   |   YV12-related constant declarations  | `external/minigbm/virgl_hw.h` |

Note: the above enhancement is based on the `android10-release` branch, since, at the time of our study, Android 10 is the most prevalent version among the users of T-video.
The related logic in other Android versions stay largely unchanged, and therefore you can port the fix to any recent Android version that you desire. 

We have also [reported the issues and the fixes](https://issuetracker.google.com/issues/262255458) to the development team of GAE.