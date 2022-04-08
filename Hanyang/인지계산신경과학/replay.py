#!/usr/bin/env python

from os.path import join

import pandas as pd
import numpy as np
import pickle

from scipy import stats
from scipy import signal

from matplotlib import pyplot as plt
from matplotlib import patches
from matplotlib import animation, rc

# 1. Loading joystick data
## 29300 frames = 58.6 frames/sec * 25 sec/trial * 20 trials
joystick = np.load(join('/mnt/sdb2/joystick/rawdata/test_kookmin','behavior_data_run1.pkl')
                   , allow_pickle=True)

for key, value in joystick.items():
    ## restore values as a numpy array
    joystick[key] = np.array(value)
    ## make sure it is 1D array
    if joystick[key].shape[-1] < 2:
        joystick[key] = np.concatenate(joystick[key])

# 2. Loading preprocessed EMG data
## 950 frames/sec (=HZ)
EMG = np.genfromtxt(join('/mnt/sdb2/joystick/preprocessed/test_kookmin','km_EMG_run1_processed_30.csv')
                     , delimiter=',', skip_header=True)
## cutting the 1st column off
EMG = EMG[:,1:].T

## # frames per trial
nframe = 950*25
## index range of a trial
idx_trial = np.arange(nframe)

## cut data from resting off
tmp = np.ones((nframe*20, 10))*np.nan
print(tmp.shape)

for trial in range(20):
    print('trial%02d:'%trial, idx_trial+nframe*trial, idx_trial+(nframe+950*10)*trial)
    tmp[idx_trial+nframe*trial,:] = EMG[idx_trial+(nframe+950*10)*trial,:]

EMG = signal.resample(tmp, joystick['time'].shape[0])

# 3. Animation

