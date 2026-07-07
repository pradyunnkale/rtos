# RTOS

A lightweight real-time kernel written in C for embedded guidance, navigation, and control (GNC) systems.

## Building

Configure the build:

```bash
cmake --preset host
```

Build the project:

```bash
cmake --build build/host
```

## Available Presets

| Preset | Target |
|--------|--------|
| `host` | Native host simulation |
| `mimxrt1064` | NXP MIMXRT1064 |

Example:

```bash
cmake --preset mimxrt1064
cmake --build build/mimxrt1064
```
