# Background Management Strategy Adaptation

The following source code and fixes are based on the `android-10.0.0_r47` branch.
The related logic in another Android versions stay largely unchanged, and therefore you can port the fix to most Android versions that you desire. 

## Step 1

Some physical device vendors use an aggressive background app inhibition strategy by simultaneously killing all the processes of an app using the `forceStopPackage()` provided by `ActivityManagerService`.
We adopt that strategy by making the following changes.

```diff
/* frameworks/base/services/core/java/com/android/server/am/ActivityManagerService.java */
@GuardedBy("this")
final void appDiedLocked(ProcessRecord app, int pid, IApplicationThread thread, boolean fromBinderDied) {
    if (!app.killed) {
        if (!fromBinderDied) {
            killProcessQuiet(pid);
        }
-       ProcessList.killProcessGroup(app.uid, pid);
+       forceStopPackage(AppGlobals.getPackageManager().getNameForUid(app.uid), 0);
        app.killed = true;
    }
}
```

## Step 2

Besides, a timeout is present in `ProcessGroup::killProcessGroup()` in `libprocessgroup` that prevents `forceStopPackage()` from stopping the app atomically.
The fix is provided below.

```diff
/* system/core/libprocessgroup/processgroup.cpp */
static int KillProcessGroup(uid_t uid, int initialPid, int signal, int retries, int* max_processes) {
    ...
    int processes;
    while ((processes = DoKillProcessGroupOnce(cgroup, uid, initialPid, signal)) > 0) {
        LOG(VERBOSE) << "Killed " << processes << " processes for processgroup " << initialPid;
        if (retry > 0) {
-           std::this_thread::sleep_for(5ms);
            --retry;
        } else {
            break;
        }
    }
    ...
}
```