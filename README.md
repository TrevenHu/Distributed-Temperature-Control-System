# Distributed-Temperature-Control-System
this project required us to use standardized software development and design methods. 
This distributed temperature control system can adjust temperature, wind speed and direction according to the customers' requirements. 
Customer can view the power consumption and cost from the system display panel. 
Moreover, the hotel can use the system to inquire each slave’s usage and print detail records. 
There are three main functional modules: customer slave control, front line control and central control system.
Our system can serve as both the central control system (NEWAIR) and a slave (Sub). 
We used JSON data communication based on TCP socket to communicate with other slaves or the central control system. 
