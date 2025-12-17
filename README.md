# LinuxBeaver GEGL Plugins

A collection of additional **GEGL operations** created by LinuxBeaver.

The GEGL operations themselves are application-agnostic.  
Only the **installation method** differs depending on:
- operating system
- application (e.g. GIMP, RasterFlow)
- distribution method

Multiple installer variants may exist for the same plugin set.

---

## Installation

### Windows - GIMP and RasterFlow
This section describes installation for GIMP on Windows
(compatible with GIMP 2.x and GIMP 3.x).

Requirements:
- Windows 10 / 11 (64-bit)
- GIMP (2.x or 3.x, official Windows build)

No additional dependencies are required.

#### Download
Download the latest Windows installer, for example:
```bash
LinuxBeaverGEGL-0.1.0-Setup-win64.exe
```

#### Install
1. Run the installer:
```bash
LinuxBeaverGEGL-<version>-Setup-win64.exe
```
2. Follow the installer steps.
3. The GEGL plugin files will be installed automatically into the appropriate GEGL plugin directory (!!!)
4. Start (or restart) GIMP.

The new GEGL operations will then be available inside GIMP or RasterFlow (depends on what you use it for).

#### How to uninstall?
1. Open Apps & Features (or Add or remove programs).
2. Use a regular procedure for uninstalling applications under Windows.

### Linux – GIMP (Flatpak)

This section describes installation for **GIMP 3.x installed via Flatpak**.

#### Requirements
- Linux
- GIMP installed as Flatpak (`org.gimp.GIMP`)

#### Download
Download the **latest Linux installer** for GIMP, for example:

```bash
LinuxBeaverGEGL-0.1.0.run
```
#### Install

1. Make the installer executable:
```bash
chmod +x LinuxBeaverGEGL-<version>.run
```

2. Run the installer:
```bash
./LinuxBeaverGEGL-<version>.run
```

3. You will be asked to confirm the installation:
```bash
Install LinuxBeaver GEGL plugins? [y/N]:
```

4. After confirmation, the plugin files will be installed into:
```bash
~/.var/app/org.gimp.GIMP/data/gegl-0.4/plug-ins
```

The new GEGL operations will then be available inside GIMP.

#### How to uninstall?
1. Run the installer with `uninstall` argument:
```bash
./LinuxBeaverGEGL-<version>.run uninstall
```

2. Confirm the removal when prompted:
```bash
Uninstall LinuxBeaver GEGL plugins? [y/N]:
```

3. Restart GIMP.

All the plugins should be gone now.