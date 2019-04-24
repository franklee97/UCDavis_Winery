# UCDavis_Winery

This repo contains all the information regarding my project at UC Davis RMI Winery. The goal of the project was to come up with a system that can collect and record data from a fermentation tank inside the winery. The data that were collected were Brix, or sugar concentration of a wine, Redox, and temperature. This system consisted of a PCB carrier board with mounting holes and necessary electrical connections, modules that either plugged into the mounting holes of the carrier board or resided on a din rail, and a box to hold all the components and wires. This repo has software information as well as hardware and layout information of the box. The system was only tested with one fermentation tank, but it is capable of expanding the number of tanks by a simple change in code. It also has the capability of sending data wirelessly via Bluetooth or XBee.


## BOM
This folder has the Bill of Materials(BOM) necessary for this project. The parts were ordered in four different batches. `Final.xlsx` contains all the components that are necessary for this project as some parts were never used from other orders.

## Code
This folder contains the code necessary to collect data. It contains two Arduino code, `data_collection_brix.ino` and `data_collection_redox.ino`. They both use `ModbusMaster.h` library to read values from the slave devices and store into SD cards, but the brix code reads two sensors' data while the redox code read one sensor's data. The polling rates for each code are also different. The code was written for Arduino Mega board, but any Arduino boards with at least two Serial ports will suffice. 

### Data Format
The data are stored as `.csv` file. The sample data for each code are inside each code folder. The name of the file uses the following format: `DATA[month][date].csv`. For example, if it is a data file from January 5th, the filename would be `DATA15.csv`. A new file was created every day to prevent an overload inside one file. More information about how this was done is in the comments for each code.


## Diagram
This folder contains two files, `Circuit_Diagram.png` and `Data_Flow_2.0.pdf`. `Circuit_Diagram.png` shows a circuit level diagram of how everything is being powered. It also shows data communication as well. `Data_Flow_2.0.pdf` is a diagram that specifically shows how data is being transferred between the modules inside the system. `Circuit_Diagram.png` was created using EAGLE and `Data_Flow_2.0.pdf` was created using Microsoft Vizio.


## Schematic
This folder contains a folder named `carrier_final` which has the necessary EAGLE files for the carrier PCB. It also has a top and bottom layout pictures given by Oshpark. The folder contains `Archives` folder that has previous EAGLE designs.


## Solidworks
This folder has `.SLDPRT` files that was necessary for the design of the box. Some components were downloaded from the manufacturing websites, and some were created by me. `Final.SLDPRT` has all the components.
