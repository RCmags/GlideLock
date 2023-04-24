# Glide Lock for a radio-controlled ornithopter
This Arduino sketch is the [glide lock](http://ovirc.free.fr/GLDAB_English.php) for a crank-driven ornithopter. It's meant to stop the wings at a small dihedral angle when the motor is turned off.  

## How it works
The system works by iddling the motor below a certain throttle until the crankshaft reaches a known position. The position of the rotor is measured using a hall sensor and a magnet attached to the crank shaft. When the magnet passes in front of the sensor, the motor turns off. However, the motor will continue to spin even after the throttle is set to zero. To work around this, one can continue to spin the motor for a short time interval after the magnet passes the sensor. If this delay is tuned correctly, the crank will stop in more or less the desired position. 

## Schematics

### Arduino nano: 
The code can accommodate either an analog or digital hall sensor as both use the same circuit.

<p align="center"> 
<img src = "/diagrams/nano/schematic-nano.png" width = "80%"></img>
</p>

### ATTiny85: 
Due to some odd behaviour, the input pin for the hall sensor has to change depending on whether it is analog or digital. Use __P0 for digital__  and __P4 for analog__. 
  
<p align="center"> 
<img src = "/diagrams/attiny/schematic-attiny.png" width = "80%"></img>
</p>

## Example
See: [Video of the Glide Lock](https://youtu.be/v0z1ojpoehQ) 
