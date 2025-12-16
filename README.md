[[TOC]]

# LinuxBeaver GEGL Plugins

A collection of additional **GEGL operations** created by LinuxBeaver, intended for use with **GIMP 3.x**.

The GEGL operations themselves are application-agnostic.  
Only the **installation method** differs depending on:
- operating system
- application (e.g. GIMP, RasterFlow)
- distribution method

Multiple installer variants may exist for the same plugin set.

---

## Installation

### Linux – GIMP (Flatpak)

This section describes installation for **GIMP 3.x installed via Flatpak**.

#### Requirements
- Linux
- GIMP installed as Flatpak (`org.gimp.GIMP`)

### Download
Download the **latest Linux installer** for GIMP, for example:

```bash
LinuxBeaverGEGL-0.1.0.run
```
### Install

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

### How to uninstall?
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