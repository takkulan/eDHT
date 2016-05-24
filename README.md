# eDHT
a simple DHT22 + LDR sensor utility for Photon, that checks readings every 10 minutes, publishes them to cloud, and saves power rest of time.

Files:
  Photon-DHT.ino
  	 Photon sketch. Trivial, just go ahead and read it.

  gotoSafeMode.py
    A crude python script that listens to Particle cloud messages and calls cloud function "safe" once device goes online. A hack.
