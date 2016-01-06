#!/bin/bash
sudo modprobe can
sudo modprobe can_raw
sudo modprobe vcan

sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0

sudo ip link add dev vcan1 type vcan
sudo ip link set up vcan1

sudo ip link add dev vcan2 type vcan
sudo ip link set up vcan2

