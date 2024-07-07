# -*- coding: utf-8 -*-
"""
Game of life script with animated evolution

Created on Tue Jan 15 12:37:52 2019

@author: shakes
"""
import conway

N = 1800

#create the game of life object
life = conway.GameOfLife(N, fastMode=True)

# SECTION 1 A
#life.insertBlinker((0,0))

# SECTION 1 B
#life.insertGlider((0,0))

# SECTION 1 C
#life.insertGliderGun((0,0))

# SECTION 1 D
#reads plaintext files
def _fileToPlainText(fileName):
    # open and reads the file
    file = open(fileName, "r")

    patternTxtString = ""

    for line in file:
        if line[0] != "!":
            patternTxtString += line

    file.close()
    return patternTxtString

# Three plaintext patterns
#txtString = _fileToPlainText('Code breaking 2\\50P35.txt')
#txtString = _fileToPlainText('Code breaking 2\\beluchenko.txt')
#txtString = _fileToPlainText('Code breaking 2\\112P23.txt')

# Insert from plaint text
#life.insertFromPlainText(txtString)

# SECTION 2 E
#txtString = _fileToPlainText('Code breaking 2\\7-in-a-row Cordership.txt')
#life.insertFromPlainText(txtString,800)
#Change the N value to > 1024

# SECTION 2 F
#reads RLE files
def _fileToRle(fileName):
    # open and reads the file
    file = open(fileName, "r")

    rleString = ""

    for line in file:
        rleString += line
    
    file.close()
    return rleString
#rleString = _fileToRle('Code breaking 2\\6bits.rle')
#rleString = _fileToRle('Code breaking 2\\50P35.rle')
#rleString = _fileToRle('Code breaking 2\\68p32.rle')
#life.insertFromRLE(rleString)

# SECTION 2 G
rleString = _fileToRle('Code breaking 2\\turingmachine.rle')
life.insertFromRLE(rleString, 0)
#Change N to 1800

# SECTION 2 H
# Whether GoOL is Turing complete means it can simulate any Turing machine, thereby
# capable of performing any computation that can be described algorithmically.

# GoL is Turing complete. 
# The pattern in GolL includes the following components:
#1.	Tape: in the pattern, the tape is represented by a line of cells that can change states which allows it to serve as the memory that the machine reads and write to just like a Turing machine tape would.
#2.	Head: Reads and writes data on the tape and can move left or right. In GoL, this is simulated by the pattern that moves along the tape, reading the state of cells and changing according to a set of rules.
#3.	State control: the state of the Turing machine which dictates its operations, is simulated in GoL by additional patterns that interact with the head and modifies its behaviour based on its program.

# The fact that GoL can build structures that function equivalently to a Turing machine’s components—tape, head, and control—indicates that GoL can support any rule-based transformation that a Turing machine can, 
# thus it is Turing complete.


cells = life.getStates() #initial state

#-------------------------------
#plot cells
import matplotlib.pyplot as plt
import matplotlib.animation as animation

fig = plt.figure()

plt.gray()

img = plt.imshow(cells, animated=True)

def animate(i):
    """perform animation step"""
    global life
    
    life.evolve()
    cellsUpdated = life.getStates()
    
    img.set_array(cellsUpdated)
    
    return img,

interval = 20 #ms

#animate 24 frames with interval between them calling animate function at each frame
ani = animation.FuncAnimation(fig, animate, frames=24, interval=interval, blit=True)

plt.show()

