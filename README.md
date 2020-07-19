## Project-Dragonfly
---
Complete end to end system for controlling a bi-copter drone

## Introduction
---
Project Dragonfly is a complete end to end custom built system designed to power 
a bi-copter drone as controller which runs on top of the raspberry-pi os. It has a complete network
stack that is used to facilitate the connection between the controller and the drone its-self.

Project Dragonfly as the name suggests was created out of inspiration of the majestic nature of dragonflies 
out in the wild from how they fly to their natural aerodynamic skeletal build. The software that's driving this 
project is ``Sharp``, an open-source programming language that offers several high level benefits as well as 
a very intuitive and easy to use interface for working with c++ on lower-level level tasks. 

#### The network stack
---
The network stack for the drone & controller consists of 2 nrf24L01 devices capable of transmitting data up to 2Mbps.
The network stack as well as other aspects of the project was built with clear-architecture in mind to offer modularity
to the codebase to make modifying components much easier.

```
                                +=========================+  (High-Level)
                                #    Network Interface    #
                                +============+============+   
                                             |
                           +--------------+  v   +~~~~~~~~~~~~~~+
                           | Remote Layer |<-+---|  Data Layer  |
                           +--------------+  |   +~~~~~~~~~~~~~~+
                                             |      (interface)
                                             v
                                +=========================+
                                #        Core Layer       #
                                +============+============+
                                             |
                                             v
                                +=========================+  (Low-Level)
                                #       Driver Layer      #
                                +=========================+
```

We use each level as a way to abstract the former pushing each lower level closer 
to interacting with the actual hardware at the low level. The network interface serves as the highest level
interface to send and receive data based on any specific request you would like to preform. The remote layer is used by the 
network layer and serves as the implementation of the actual requests made by the former. Each object in the remote
layer must implement the interfaces described in the data layer.

The core layer is where the rubber meats the road. The core layer takes any requests passed to it via the remote layer
and prepares the data to simply be read/written via the driver. And finally the driver represents the interface to 
send to and process information that need to be transmitted by the nrf24L01 device. The custom driver for the nrf24L01 
uses a DTP(Data Transmission Protocol) to transmit the data between devices using a packet validation system.