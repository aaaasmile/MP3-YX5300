# MP3-YX5300
This is a Mp3 player build on ESP8266 and the mp3 player YX5300.
The ESP8266 is a very cheap microcontroller and the YX5300 is a serial mp3 player with an integrated Sd card slot.

The ESP8266 is used to send serial AT commands to the YX5300.

The ESP8266 can recive commands to chage the song using 3 buttons.
* Next song
* Previous song
* Next folder

This is the device that I have built used to listen music into my car:

![Device](https://github.com/aaaasmile/MP3-YX5300/blob/master/doc/IMG_0632.JPG?raw=true)

![Device2](https://github.com/aaaasmile/MP3-YX5300/blob/master/doc/IMG_0633.JPG?raw=true)

It is also possible to control the player using an integrated hot spot 
that could be used with the IP address 192.168.4.1. In this case I can use the browser on my Iphone to control the device.

## Wire
    ESP8266
    D5 for RX  ------> TX on YX5300
    D6 for TX  <-----  RX on YX5300
    Buttons:
    D4 Previous Song
    D2 Next Song
    D3 Next Folder

The other wire on the button goes to Gnd.
I use also 2 condensator between  Vcc and Gnd to reduce noise.

## Prepare the SD Card
The structure on the SD-card should match the code in this project.
The file structure is found on the file synch-card-data.inc.
Here is coded the information about how many folders are available 
and how many files are available in that folder. Such kind of information
should be automatically available with the YX5300 device, but I was unable to
make it working, so I coded that information into the device firmware.

Mp3 files on the sd card should be named as:
- 0001abc.mp3
- 0002abc.mp3
- 0003abc.mp3
- ...

and so on. Usually you do not have mp3 files with such names, so I have created
another project to rename my mp3 files. See the https://github.com/aaaasmile/rename-file.

## Motivation
At first there are no clear reason why I should build such a fat mp3 player when an Ipod
is ready to use, supported by my car and much more smaller.
For a ESP bebinner like me, building and programming the ESP8266 is funny und much more rewarding that buying an ipod
and playing around with itunes. 

But there are other reasons: 
- the random playing list on my car
- sound quality

I have setted up an ipod with hundred of songs, but very often when I plug the device 
into my car the random song sequence is the same as the old one. 
Hearing always the same songs is very annoying and also the sound quality via Ipod-USB is not so
good as the Line-In in my car. 

## Random sequence
On the Ipod the random song is choosed within a list of all songs. If you are putting a lot of
songs in your ipod, chances are that you mixing different music genre. 
There is also no limit how many time the same song is played. You can hear the same song 4 times and one another song never.
This is because the random is done every time on the full list.

In MP3-YX5300 I use different folders. On the first folder, the default when the device is pluggen-in into my car,
I want a random list of songs without repetition. The random list should not be the same as the last time I started the car.
This is a tricky part, becuase if you use the time until the device is started there are chances that same random seed is used
again. So I mixed it with an analog sensor value that should provide always a good random seed.

Different folders have different music, so when I switch the folder I get something different. 
I have coded the possibility to avoid a random sequence into the folder. This is because 
for some music you want to hear in a known sequence. 

## Drawbacks
- The size of the device.
- Some time I hear noise, probably is the wiring disturbing the music out-line. 
The Wifi is not enabled as default for this reason.
- Preparing the SD-Card needs much more time that with itunes and ipod. 
- When I stop the car for a break, the song sequence is recreated again. 
Probably restoring the seqence from the Eprom could be an improvement. In this case a sequence reset somehow is
also needed, otherwise you are hearing the same sequence forever. I think when a long trip is started
the random sequence shoulld be recreated, but not when you make a brake into the gas station.