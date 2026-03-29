# CANNXP
CBUS module to compute eNtry/eXit routes with a little bit of prototypical OCD.

Based on CANNX written by Sven Rosvall with, of course, acknoledgement of all his hard work.

See [description of CANNX](https://merg.org.uk/merg_wiki/doku.php?id=arduino:cannx)
in the MERG Knowledgebase.

## Overview

A route is set between an entrance button and an exit button. Each button can be valid as an entrance, exit or both. The first button to be pressed should be an entrance button and the second button an exit which makes logical sense as a route from the entrance. When an entrance button is pressed, it will start to flash. If no exit button is pressed within a certain time (typically five seconds), the process is cancelled and the flashing stops. If a button is pressed, but it is not a valid exit for the selected entrance then the processing is cancelled and the flashing stops. If a valid exit button is pressed, the entrance button light will become steady and the route will be "called".

Note that on larger panels, there is the concept of a "flasher ring" which groups the entrance buttons so that multiple signalmen can set multiple routes simultaneously. That concept is nto supported here - only a single entrance button can be active at any one time.

## Route design

It is probably easiest to create a spreadsheet for clarity. 

Each button whether entrance, exit or combined is given a unique number. Then each possible route (from an entrance to an exit) is also given a unique number. 

![alt text](Images/Buttons.png)

## Node Variables

### NV1 - Entrance button timeout

The time in seconds that the CANNXP will wait for an exit button to be pressed after an entrance button has been activated.

## Event Variables

### EV1 - Button Number 

All buttons involved with route setting are allocated a unique number. Routes are then defined as going between two buttons (a button capable of acting 
as an Entrance and a button capable of acting as an exit). The number is abitrary, cannot be zero, and should not exceed approximately 620 because it is 
multiplied by 200 when creating certain produced events and events cannot be numbered higher than 65535.

### EV2 - Button Type

Depending on their position on the track layout, buttons can have different functions. These are identified as follows:

- 1 - Entrance button
- 2 - Entrance and exit button
- 3 - Exit button

### EV3 through EV12 - Valid exit buttons

Only relevant for button types 2 and 3 (buttons which support exit functionality). Each EV contains the button number of an exit button which would 
be a valid exit for the entrance button defined in EV1. This restricts the maximum number of routes from any entrance button to ten.

### EV13 though EV22 - Route numbers

Only relevant when a valid exit button has been specified in the corresponding EV (EV3 through EV12). The unique number here is used to define 
the route between the specified entrance and exit buttons.




CANNXP makes the following changes:

## Sequential routes
The code no longer stores the potential subsequent routes when the second button in a sequence has been pressed. The original
CANNX implementation allowed routes to be set using a short cut method such as A-B-C. Whilst that works very well for a model
railway, it is not prototypical. On a real panel, the signalman would have to press A-B-B-C.

## Entrance only buttons
The original CANNX implementation assumes that all routes are bi-directional which is not necessarily the case on the prototype
railway. To that end, some buttons are "entrance only", some are "exit only" and some are "combined". To get close to the 
prototype, the CANNXP allows for buttons to be flagged as "entrance" and only those buttons will be recognised as the first 
button of a pair. If an "exit only" button if pressed first, it will be ignored.

## Entrance button illumination
When blah blah





## Event variables
The first 20 are unchanged from CANNX and represent the route numbers of routes involved with the button that the event handles.

### Button Type
The 21st EV can be either 0, 1 or 2 meaning:

- 0 - the button is used for route exits only
- 1 - the button is used for route entrances only
- 2 - the button can be used both as an entrance and an exit *** (further work is needed here to prevent the setting of backwards routes) ***

### Button Number
The 22nd EV holds the button number. This is not used internally by CANNXP. Rather, it is used to identify distinct buttons (as opposed to routes)
in events produced by the CANNXP.

## Produced events
The CANNXP produces certain events at distinct points in its processing, designed to allow a hardware panel to act prototypically.

### Entrance button pressed
When an entrance button is pressed, the butoon light should flash. To allow this to happen, the CANNXP produces an ON event with its node number and
an event number which is the pressed button number * 100.

### Route call request
This is the original event (called a route set event in CANNX) produced when the CANNXP recognises a valid combination of buttons, representing a route. An ON event with the node number and an event number equal to the route number is produced.

### Entrance button steady
When a route has been called successfully, the entrance button at the start of the route is illuminated continuously. To facililate this, the CANNXP produces an ON event with its node number and an event number which is the relevant entrance button number * 200.

### Entrance button timeout
???
