# -*- coding: utf-8 -*-
"""
The Game of Life (GoL) module named in honour of John Conway

This module defines the classes required for the GoL simulation.

Created on Tue Jan 15 12:21:17 2019

@author: shakes
"""
import numpy as np
from scipy import signal
import rle

class GameOfLife:
    '''
    Object for computing Conway's Game of Life (GoL) cellular machine/automata
    '''
    def __init__(self, N=256, finite=False, fastMode=False):
        self.grid = np.zeros((N,N), np.int64)
        self.neighborhood = np.ones((3,3), np.int64) # 8 connected kernel
        self.neighborhood[1,1] = 0 #do not count centre pixel
        self.finite = finite
        self.fastMode = fastMode
        self.aliveValue = 1
        self.deadValue = 0
    
    def getStates(self):
        '''
        Returns the current states of the cells
        '''
        return self.grid
    
    def getGrid(self):
        '''
        Same as getStates()
        '''
        return self.getStates()
               
    def evolve(self):
        '''
        Given the current states of the cells, apply the GoL rules:
        - Any live cell with fewer than two live neighbors dies, as if by underpopulation.
        - Any live cell with two or three live neighbors lives on to the next generation.
        - Any live cell with more than three live neighbors dies, as if by overpopulation.
        - Any dead cell with exactly three live neighbors becomes a live cell, as if by reproduction
        '''
        #get weighted sum of neighbors
        #PART A & E CODE HERE
        
        #implement the GoL rules by thresholding the weights
        #PART A

        if self.fastMode == False:
            #an array that contains the weighted sum of neighbours for each cell
            #each index of the array corresponds to the index of the cell
            neighbours = []
            
            #keeps count which row is being iterated
            rowIndex = 0

            #iterates throughe row of the grid, and checks the weighted sum of neighbours
            for row in self.grid:
                cellIndex = 0

                # adds an array to nieghbour that represents a row of cells
                neighbours.append([])

                for cell in row:
                    #the number of neighbours for this cell
                    neighbourCount = 0

                    if cellIndex != 0:
                        #checks left-side neighbours
                        if row[cellIndex - 1] == self.aliveValue:
                            neighbourCount += 1
                
                    if cellIndex != len(row) - 1:
                        #checks right-side neighbours
                        if row[cellIndex + 1] == self.aliveValue:
                            neighbourCount += 1

                    if rowIndex != 0:
                        #checks top-side neighbours
                        if self.grid[rowIndex - 1][cellIndex] == self.aliveValue:
                            neighbourCount += 1
                        
                        if cellIndex != len(row) - 1:
                            #checks top-right neighbours
                            if self.grid[rowIndex - 1][cellIndex + 1] == self.aliveValue:
                                neighbourCount += 1
                        
                        if cellIndex != 0:
                            #checks top-left neighbours
                            if self.grid[rowIndex - 1][cellIndex - 1] == self.aliveValue:
                                neighbourCount += 1

                    if rowIndex != len(self.grid) - 1:
                        #checks bottom-side neighbours
                        if self.grid[rowIndex + 1][cellIndex] == self.aliveValue:
                            neighbourCount += 1
                            
                        #checks bottom-right neighbours
                        if cellIndex != len(row) - 1:
                            if self.grid[rowIndex + 1][cellIndex + 1] == self.aliveValue:
                                neighbourCount += 1
                        
                        #checks bottom-left neighbours
                        if cellIndex != 0:
                            if self.grid[rowIndex + 1][cellIndex - 1] == self.aliveValue:
                                neighbourCount += 1

                    neighbours[rowIndex].append(neighbourCount)
                    cellIndex += 1
                        
                rowIndex += 1


            rowIndex = 0
            updatedGrid = []
            for row in self.grid: 
                cellIndex = 0
                updatedGrid.append([])
                for cell in row:
                    updatedGrid[rowIndex].append(0)
                    neighbourCount = neighbours[rowIndex][cellIndex]  
                    
                    if cell == self.deadValue:
                        #dead cell becomes alive
                        if neighbourCount == 3:
                            updatedGrid[rowIndex][cellIndex] = self.aliveValue
                    else:
                        if neighbourCount < 2 or neighbourCount > 3:
                            #alive cell dies from under population or overpopulation
                            updatedGrid[rowIndex][cellIndex] = 0
                        else:
                            #alive cell lives on to the next generation
                            updatedGrid[rowIndex][cellIndex] = self.aliveValue
                            
                    cellIndex += 1

                rowIndex += 1
        
            #update the grid     
            self.grid = updatedGrid[:]  

        #PART E
        if self.fastMode: 
            neighbourCount = signal.convolve2d(self.grid, self.neighborhood, mode='same', boundary='fill', fillvalue=0)
            survival = ((self.grid == self.aliveValue) & ((neighbourCount == 2) | (neighbourCount == 3)))
            reproduction = (self.grid == self.deadValue) & (neighbourCount == 3)

            #update the grid
            self.grid = (survival | reproduction)
    
    def insertBlinker(self, index=(0,0)):
        '''
        Insert a blinker oscillator construct at the index position
        '''
        self.grid[index[0], index[1]+1] = self.aliveValue
        self.grid[index[0]+1, index[1]+1] = self.aliveValue
        self.grid[index[0]+2, index[1]+1] = self.aliveValue
        
    def insertGlider(self, index=(0,0)):
        '''
        Insert a glider construct at the index position
        '''
        self.grid[index[0], index[1]+1] = self.aliveValue
        self.grid[index[0]+1, index[1]+2] = self.aliveValue
        self.grid[index[0]+2, index[1]] = self.aliveValue
        self.grid[index[0]+2, index[1]+1] = self.aliveValue
        self.grid[index[0]+2, index[1]+2] = self.aliveValue
        
    def insertGliderGun(self, index=(0,0)):
        '''
        Insert a glider construct at the index position
        '''
        self.grid[index[0]+1, index[1]+25] = self.aliveValue
        
        self.grid[index[0]+2, index[1]+23] = self.aliveValue
        self.grid[index[0]+2, index[1]+25] = self.aliveValue
        
        self.grid[index[0]+3, index[1]+13] = self.aliveValue
        self.grid[index[0]+3, index[1]+14] = self.aliveValue
        self.grid[index[0]+3, index[1]+21] = self.aliveValue
        self.grid[index[0]+3, index[1]+22] = self.aliveValue
        self.grid[index[0]+3, index[1]+35] = self.aliveValue
        self.grid[index[0]+3, index[1]+36] = self.aliveValue
        
        self.grid[index[0]+4, index[1]+12] = self.aliveValue
        self.grid[index[0]+4, index[1]+16] = self.aliveValue
        self.grid[index[0]+4, index[1]+21] = self.aliveValue
        self.grid[index[0]+4, index[1]+22] = self.aliveValue
        self.grid[index[0]+4, index[1]+35] = self.aliveValue
        self.grid[index[0]+4, index[1]+36] = self.aliveValue
        
        self.grid[index[0]+5, index[1]+1] = self.aliveValue
        self.grid[index[0]+5, index[1]+2] = self.aliveValue
        self.grid[index[0]+5, index[1]+11] = self.aliveValue
        self.grid[index[0]+5, index[1]+17] = self.aliveValue
        self.grid[index[0]+5, index[1]+21] = self.aliveValue
        self.grid[index[0]+5, index[1]+22] = self.aliveValue
        
        # Sixth set of values
        self.grid[index[0]+6, index[1]+1] = self.aliveValue
        self.grid[index[0]+6, index[1]+2] = self.aliveValue
        self.grid[index[0]+6, index[1]+11] = self.aliveValue
        self.grid[index[0]+6, index[1]+15] = self.aliveValue
        self.grid[index[0]+6, index[1]+17] = self.aliveValue
        self.grid[index[0]+6, index[1]+18] = self.aliveValue
        self.grid[index[0]+6, index[1]+23] = self.aliveValue
        self.grid[index[0]+6, index[1]+25] = self.aliveValue
        
        self.grid[index[0]+7, index[1]+11] = self.aliveValue
        self.grid[index[0]+7, index[1]+17] = self.aliveValue
        self.grid[index[0]+7, index[1]+25] = self.aliveValue
        
        self.grid[index[0]+8, index[1]+12] = self.aliveValue
        self.grid[index[0]+8, index[1]+16] = self.aliveValue
        
        self.grid[index[0]+9, index[1]+13] = self.aliveValue
        self.grid[index[0]+9, index[1]+14] = self.aliveValue    

    def insertFromPlainText(self, txtString, pad=0):
        '''
        Assumes txtString contains the entire pattern as a human readable pattern without comments
        '''
        rowIndex = 0
        txtString = txtString.split("\n")
        for row in txtString:   
            cellIndex = 0         
            for cell in row:
                if cell == 'O':
                    self.grid[rowIndex + pad][cellIndex + pad] = self.aliveValue
                cellIndex += 1
            rowIndex += 1
       
    def insertFromRLE(self, rleString, pad=0):
        '''
        Given string loaded from RLE file, populate the game grid
        '''
        rleInfo = rle.RunLengthEncodedParser(rleString)
        rowIndex = 0
        for row in rleInfo.pattern_2d_array:
            cellIndex = 0
            for cell in row:
                if cell == 'o':
                    self.grid[rowIndex + pad][cellIndex + pad] = self.aliveValue
                cellIndex += 1
            rowIndex += 1


       


        