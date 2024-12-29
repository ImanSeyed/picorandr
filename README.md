`picorandr` is a lightweight tool that uses the Linux kernel's Direct Rendering Manager (DRM) to detect GPUs, retrieve their vendor and device details via PCI addresses, and list connected display resolutions, connectors, and their specific drivers. This specially helps laptop users understand their device's hardware and driver scheme.

### Building
Install the `libpci` and `libdrm` libraries, including their header files. Next, build the project as shown below:
```bash
$ cmake -B build; make -C build
```

### Installing
```bash
$ sudo make -C build install
```

### Usage
To get a brief output, run the program without any options:
```bash
$ picorandr
8086:9a49 Intel Corporation TigerLake-LP GT2 [Iris Xe Graphics]
          Kernel driver in use: i915
          Connectors:
                  eDP-1 (connected)
                          * 1920x1080
                  DP-1 (disconnected)
                  DP-2 (disconnected)
                  HDMI-1 (disconnected)

10de:2520 NVIDIA Corporation GA106M [GeForce RTX 3060 Mobile / Max-Q]
          Kernel driver in use: nvidia
          Connectors:
                  HDMI-2 (connected)
                          * 1920x1080
```

For detailed Linux kernel logs specific to each GPU driver, execute the command below:
```bash
$ picorandr -k
8086:9a49 Intel Corporation TigerLake-LP GT2 [Iris Xe Graphics]
          Kernel driver in use: i915
          Connectors:
                  eDP-1 (connected)
                          * 1920x1080
                  DP-1 (disconnected)
                  DP-2 (disconnected)
                  HDMI-1 (disconnected)
<6>[    0.383587] i915 0000:00:02.0: [drm] Found TIGERLAKE/UY (device ID 9a49) display version 12.00 stepping C0
<6>[    0.383997] i915 0000:00:02.0: [drm] VT-d active for gfx access
<6>[    0.384000] i915 0000:00:02.0: vgaarb: deactivate vga console
<5>[    0.384031] i915 0000:00:02.0: [drm] Transparent Hugepage support is recommended for optimal performance on this platform!
[...]

10de:2520 NVIDIA Corporation GA106M [GeForce RTX 3060 Mobile / Max-Q]
          Kernel driver in use: nvidia
          Connectors:
                  HDMI-2 (disconnected)
<4>[    1.027168] nvidia: loading out-of-tree module taints kernel.
<4>[    1.027180] nvidia: module license 'NVIDIA' taints kernel.
<4>[    1.027186] nvidia: module license taints kernel.
<6>[    1.231402] nvidia-nvlink: Nvlink Core is being initialized, major device number 243
[...]
```

**Note 1**: If you're using an NVIDIA GPU and want the program to work properly, ensure that the `nvidia_drm` module is loaded first.

**Note 2**: If your graphics card is only used for 3D acceleration, you won't see any DRM connectors. This is expected behavior, so no need to panic or yell at me. It's just how it works!

### Contribution
For contributions to development, please run the command below before creating a PR:
```bash
$ clang-format -i --style=file src/* include/*
```
