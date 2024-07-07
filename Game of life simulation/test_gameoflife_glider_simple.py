# -*- coding: utf-8 -*-
"""
Game of life simple script for checking init states and checking if the evolution is
implemented correctly.

Created on Tue Jan 15 12:37:52 2019

@author: shakes
"""
import conway

N = 88

#create the game of life object
life = conway.GameOfLife(N)

# SECTION I TASK A
#life.insertBlinker((0,0))

# SECTION I TASK B
# Just search up the pattern of the glider online and see if it matches Figures 0, 1 and 2
#life.insertGlider((0,0))

# SECTION I TASK C
#life.insertGliderGun((0,0))

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

# txtString = _fileToPlainText('Code breaking 2\\50P35.txt')
# life.insertFromPlainText(txtString, 0)

cells = life.getStates() #initial state

#evolve once
life.evolve()
cellsUpdated1 = life.getStates()

#evolve twice
life.evolve()
cellsUpdated2 = life.getStates()


#-------------------------------
#plot cells
import matplotlib.pyplot as plt
import numpy as np

plt.figure(0)
plt.gray()
plt.imshow(cells)

ax = plt.gca()
# Minor ticks
ax.set_xticks(np.arange(-.5, N, 1), minor=True);
ax.set_yticks(np.arange(-.5, N, 1), minor=True);
#grid
ax.grid(which='minor', color='w', linestyle='-', linewidth=1)

plt.figure(1)
plt.imshow(cellsUpdated1)

# plt.figure(2)
# plt.imshow(cellsUpdated2)

plt.show()
