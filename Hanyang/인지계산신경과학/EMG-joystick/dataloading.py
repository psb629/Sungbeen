"""
Pytorch data loader for behavioral and EMG data
"""
import numpy as np
import torch
from torch.utils import data
from torch import Tensor
import glob
from scipy import signal
import easydict
from torchaudio.transforms import MelSpectrogram, Spectrogram

class Dataset(data.Dataset):
    'Characterizes a dataset for PyTorch'
    def __init__(self, args):
        'Initialization'
        self.behav_file = args.behav_file
        self.emg_file = args.emg_file
        self.FFT = args.FFT
        self.emg_fs = args.emg_fs      # EMG sampling rate (950 Hz)
        self.ds_trial = args.ds_trial  # duration of each trial (s)
        self.ds_break = args.ds_break  # duration of each break (s)
        self.n_trials = args.n_trials
        self.n_channels = args.n_channels
        # number of samples of each behavior training & test data
        self.n_samples = args.n_samples 
        self.ratio = args.ratio  # ratio of sample numbers of EMG to behavior
        self.j = 0
        self.temp = 0
        behav, EMG, EMG_power = self.preprocess(self.n_samples)
#        self.batch_size = arg.batch_size
        self.behav = behav  ## Time by c
        self.EMG = EMG
        if self.FFT:
            self.EMG_power = EMG_power

#        self.pos, self.vel, self.acc = behav_data
        #if behav_data.shape != EMG.shape:
         #   raise ValueError('Check the sizes of behav_data and EMG are matched')

        
    def __len__(self):
        'Denotes the total number of samples'
        return self.EMG.shape[0]

    def __getitem__(self, index):
        'Generates one sample of data'
        # Load data and get label
 #       X = (
 #           Tensor(self.pos[index]) ,
 #           Tensor(self.vel[index]) ,
 #           Tensor(self.acc[index]) ,
 #           )
        if self.FFT:
            return Tensor(self.behav[index]), Tensor(self.EMG[index]), Tensor(self.EMG_power[index])
        else:
            return Tensor(self.behav[index]), Tensor(self.EMG[index])

    def preprocess(self, n_samples):
        ## behavior data preprocessing
        behav_data = np.load(self.behav_file, allow_pickle=True)
        for key, value in behav_data.items():
            behav_data[key] = np.array(value)
            if behav_data[key].shape[-1] < 2:
                behav_data[key] = np.concatenate(behav_data[key])
        pos = behav_data['cursor'] # Time by 2 (samples, 2)
        t_mark = behav_data['time'] # (samples,)
        ## dt1 = [t_i+1 - t_i, ...]
        dt1 = np.diff(t_mark)
        dt1[dt1>9] = 1.e4 # boundaries of trials
        ## dt2 = [(t_i+2 - t_i)/2, ...]
        dt2 = np.array([t_mark[i+2]-t_mark[i] for i in range(t_mark.shape[0]-2)])*.5
        dt2[dt2>9] = 1.e4 # boundaries of trials
        
        vel = np.diff(pos, axis=0)/np.tile(dt1[:,np.newaxis],2) # (samples-1, 2)
        acc = np.diff(vel, axis=0)/np.tile(dt2[:,np.newaxis],2) # (samples-2, 2)

        pos = pos[:int(np.floor(len(pos)/n_samples) * n_samples)]
        vel = vel[:int(np.floor(len(vel)/n_samples) * n_samples)]
        acc = acc[:int(np.floor(len(acc)/n_samples) * n_samples)] 
        pos = np.reshape(pos, (-1, n_samples, 2)) # 2 indicates x and y
        vel = np.reshape(vel, (-1, n_samples, 2))
        acc = np.reshape(acc, (-1, n_samples, 2))
        behav = np.concatenate([pos, vel, acc], axis=2)
        target = behav_data['target']
        hit = behav_data['hit']
        ## EMG data preprocessing
        EMG = np.genfromtxt(self.emg_file, delimiter=',', skip_header=True)
        ## cutting the 1st column off
        EMG = EMG[:,1:].T  # Time by channel
        ## cut data from resting off
        nframe = self.emg_fs * self.ds_trial
        idx_trial = np.arange(nframe)
        tmp = np.ones((nframe * self.n_trials, self.n_channels))*np.nan
        # Eliminating data during break
        for trial in range(self.n_trials):
            tmp[idx_trial+nframe*trial,:] = EMG[idx_trial+(nframe + self.emg_fs * self.ds_break) * trial,:]
        ## resampling EMG at the sampling rate of behavior data
        EMG = signal.resample(tmp, behav_data['time'].shape[0]*self.ratio)
        EMG = EMG[:int(np.floor(len(EMG)/(n_samples*self.ratio)) * n_samples*self.ratio)]
        EMG = np.reshape(EMG, (-1, n_samples*self.ratio, self.n_channels))
        #temp = np.reshape(EMG, (-1, n_samples*self.ratio, self.n_channels))
        if self.FFT:
            EMG_power = Spectrogram(n_fft=600, win_length = 600)(Tensor(EMG.transpose(0,2,1)))   ## hop_length = 0.5*win_length/600 = 0.1 s
            EMG_power = EMG_power.numpy().transpose(0,3,2,1)  ## Number of data X time X number of frequency band X n_channel
#            EMG_power = EMG_power[:np.floor(EMG_power.shape[0]/temp.shape[0])*temp.shape[0]]
 #           EMG_power = np.reshape(EMG_power, (temp.shape[0], -1 ,self.n_channels))


        return behav, EMG, EMG_power
    