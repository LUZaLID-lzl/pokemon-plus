#!/usr/bin/env python3
import argparse,time
p=argparse.ArgumentParser();p.add_argument('port');a=p.parse_args()
try:import serial
except ImportError:raise SystemExit('需要 pyserial：python3 -m pip install pyserial')
with serial.Serial(a.port,115200,timeout=2) as s:
 s.write(f'TIME {int(time.time())}\n'.encode());s.flush()
print('已发送当前时间；以设备串口 Clock synchronized 日志和屏幕为准。')
