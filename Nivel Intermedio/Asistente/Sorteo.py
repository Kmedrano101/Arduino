# -*- coding: utf-8 -*-
"""
Created on Sat Feb 20 19:01:09 2021

@author: Kevin Medrano Ayala
"""

import random

L= [random.randint(1, 10)] 
i=1
while i<5:
  x = random.randint(1,10)
  if x not in L:
    L.append(x)
    i+=1

print(L)
