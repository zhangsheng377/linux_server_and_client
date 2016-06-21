#!/bin/sh
cvlc -vvv /home/zsd/1.rmvb --sout "#transcode{vcodec=h264,acodec=mpga,ab=128,channels=2,samplerate=44100}:udp{dst=255.255.255.255:1234}" --sout-all --sout-keep

