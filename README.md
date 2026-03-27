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
Blah