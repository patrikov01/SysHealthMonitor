# SysHealthMonitor

A small Linux system monitor written in C++17.

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Run

```sh
./build/sys_health_monitor [interval-seconds] [log-file] [critical-samples]
```

Defaults are a 2-second interval, `syshealth.log`, and three consecutive
critical samples. A sample is critical when CPU usage is above 95% or CPU
temperature is above 85 C. Missing NVIDIA hardware is reported as unavailable
and does not stop the monitor.

Desktop notifications require `notify-send`, normally supplied by the
`libnotify-bin` package on Ubuntu.

Tested output:
CPU 0.1%, CPU temp unavailable, GPU 0.0%, GPU temp 48.0 C, GPU memory 0.0/2048.0 MiB
CPU 0.9%, CPU temp unavailable, GPU 0.0%, GPU temp 49.0 C, GPU memory 0.0/2048.0 MiB
Monitor stopped.