# CANNXP
CBUS module to compute eNtry/eXit routes with a little bit of prototypical OCD

Based on CANNX written by Sven Rosvall with, of course, acknoledgement of all his hard work.

See [description of CANNX](https://merg.org.uk/merg_wiki/doku.php?id=arduino:cannx)
in the MERG Knowledgebase.

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
