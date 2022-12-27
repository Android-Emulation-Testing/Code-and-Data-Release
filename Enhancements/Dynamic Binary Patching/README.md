# Dynamic Binary Patching

## Background

In order to essentially reduce false negative test results, we need to be able to quickly prototype and deploy fixes to the design defects and implementation bugs inside vendors’ proprietary system components.
In fact, vendors are usually not well motivated to fix seemingly app-specific issues, unless persuasive evidences can be provided by us beforehand.
Worse still, we cannot simply patch a vendor component by directly replacing them without root privileges because the proprietary vendor components are immutable.

## Our Solution

To address the above problems, we develop a *dynamic binary patching* method to realize in-memory manipulation of the problematic components during the app’s run time.
Upon the startup of an app, we locate the base address of the problematic component’s binary that we wish to patch.
We then calculate the address of the problematic instructions we wish to patch in the binary’s code segment. 
Finally, by resetting the write privilege of the corresponding memory region via the `mprotect` system call (no root privileges required), we can rewrite the original instructions or insert trampolines into the binary, so that the problematic implementations can be overwritten or bypassed.

To demonstrate our method, we take the most frequent false negative failure (i.e., AOSP’s implementation bugs when Android initiates vendor-customized system services) as an example, and provide our *dynamic binary patching* method in [`dynamic_binary_patching.cpp`](dynamic_binary_patching.cpp).
