# Graphics Resource Format Extension

The following source code and fixes are based on the `android-10.0.0_r47` branch.
The related logic in another Android versions stay largely unchanged, and therefore you can port the fix to most Android versions that you desire. 

There exist discrepencies in how AOSP and the `minigbm` host-side graphics library handles certain graphics resources (i.e., YV12 buffer height alignment).
In [Android specifications](https://developer.android.com/reference/android/graphics/ImageFormat#YV12), YV12 buffer heights are unaligned, but in the underlying minigbm driver, buffer heights are aligned to 8 bytes (cf. [virtio_dumb_bo_create()](https://cs.android.com/android/platform/superproject/+/android-10.0.0_r47:external/minigbm/virtio_gpu.c;drc=abe44f62208cfaf1b329703d9043b1004baffb44;l=67) and [drv_bo_from_format()](https://cs.android.com/android/platform/superproject/+/android-10.0.0_r47:external/minigbm/helpers.c;drc=6bd7885bcfc2bb64fd2c532e1a83fd5d38fd981b;l=239). Some other backends (e.g., `msm`, `tegra`) use various alignment sizes as well. This creates inconsistencies that will ultimately lead to overruns and invalid accesses when these buffers are used in the Android framework.

To address this issue, we extend the resource format within the host-side graphics library (used by emulators) by adding the Android-specific format
based on the resource definitions of Android.
The enhancement code is provided below.

```diff
/* external/minigbm/helpers.c */
int drv_dumb_bo_create_ex(struct bo *bo, uint32_t width, uint32_t height, uint32_t format, uint64_t use_flags, uint64_t quirks)
{
    ...
	ret = drmIoctl(bo->drv->fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb);
	if (ret) {
		drv_log("DRM_IOCTL_MODE_CREATE_DUMB failed (%d, %d)\n", bo->drv->fd, errno);
		return ret;
	}

-	drv_bo_from_format(bo, create_dumb.pitch, height, format);
+	drv_bo_from_format(bo, create_dumb.pitch, bo->meta.height, format);

	for (plane = 0; plane < bo->num_planes; plane++)
		bo->handles[plane].u32 = create_dumb.handle;

	bo->total_size = create_dumb.size;
	return 0;
}
```

We have also [reported the issues and the fixes](https://issuetracker.google.com/issues/262255458) to the development team of GAE.