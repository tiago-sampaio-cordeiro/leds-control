#!/bin/bash

PORT=/dev/ttyACM0

echo "[BR] Stopping Border Router"

# Derruba interface primeiro
sudo ip link set tun0 down 2>/dev/null

# Mata apenas tunslip6 ligado à porta serial
sudo pkill -f "tunslip6.*$PORT"
sleep 1

# Libera porta serial (último recurso)
sudo fuser -k "$PORT" 2>/dev/null

# Desativa forwarding IPv6
sudo sysctl -w net.ipv6.conf.all.forwarding=0 >/dev/null
sudo sysctl -w net.ipv6.conf.default.forwarding=0 >/dev/null

echo "[BR] Clean shutdown complete"
