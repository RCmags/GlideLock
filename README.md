# Glide Lock for RC ornithopter

This is a sketch for a [glide lock](http://ovirc.free.fr/GLDAB_English.php) for a crank-driven ornithopter. It's intended to allow the wings to stop a dihedral angle that permits efficient gliding when the motor is turned off.  

## How it works
The system works by iddling the motor below a certain throttle until the crankshaft reaches a known position. The position of the rotor is measured using a hall sensor and a magnet attached to the crank shaft. When the magnet passes infront of the sensor, the motor turns off. However, the motor will continue to spin even after the throttle is set to zero. To work around this, one can continue to spin the motor for a short time interval after the magnet passes the sensor. If this delay is tuned correctly, the crank will stop in more or less the desired position. 

## Schematic

### Arduino nano:
The code can accomodate either an analog or digital hall sensor. Whichever type of sensor is used, they both require the same circuit: 

<p align="center"> 
<img src = "/diagrams/nano/schematic-nano.png" width = "80%"></img>
</p>

### ATTiny85: 
Due to some odd behaviour, the input pin for the hall sensor has to change depending on whether it is analog or digital. Use __P0 for a digital__ sensor and __P4 for an analog__ sensor. 
  
<p align="center"> 
<img src = "/diagrams/attiny/schematic-attiny.png" width = "80%"></img>
</p>

## Example
See: [Video of the Glide Lock working](https://youtu.be/v0z1ojpoehQ) 
