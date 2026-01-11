#!/bin/bash

PORT=/dev/ttyACM0
PREFIX=fd00::1/64

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TUNSLIP="$SCRIPT_DIR/../serial-io/tunslip6"

sudo sysctl -w net.ipv6.conf.all.forwarding=1
sudo sysctl -w net.ipv6.conf.default.forwarding=1

sudo "$TUNSLIP" -s "$PORT" "$PREFIX"
