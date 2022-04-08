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
# from utils import moving_average
from torchaudio.transforms import MelSpectrogram, Spectrogram

class Dataset(data.Dataset):
    'Characterizes a dataset for PyTorch'
    def __init__(self, args):
        'Initialization'
        self.behav_file = args.behav_file
        self.emg_file = args.emg_file
        self.FFT = args.FFT
        self.behav_fs = args.behav_fs  # Behavior sampling to be set, not actual sampling rate (e.g., 60 Hz)
        self.emg_fs = args.emg_fs      # EMG sampling rate (950 Hz)
        self.win_length = args.win_length # EMG window length (ms)
        self.hope_length = args.hope_length # a hope length of EMG windows (ms)
        self.ds_trial = args.ds_trial  # duration of each trial (s)
        self.ds_break = args.ds_break  # duration of each break (s)
        self.n_trials = args.n_trials  # 20 
        self.n_channels = args.n_channels
        # number of samples of each behavior training & test data
        self.behav, self.EMG, self.EMG_spec = self.preprocess()

#         self.pos, self.vel, self.acc = behav_data
#         if behav_data.shape != EMG.shape:
#             raise ValueError('Check the sizes of behav_data and EMG are matched')
        
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
#         if self.FFT:
#             return Tensor(self.behav[index]), Tensor(self.EMG[index]), Tensor(self.EMG_power[index])
#         else:
#             return Tensor(self.behav[index]), Tensor(self.EMG[index])

    def preprocess(self):
        ## behavior data preprocessing
        behav_data = np.load(self.behav_file, allow_pickle=True)
        for key, value in behav_data.items():
            behav_data[key] = np.array(value)
            if behav_data[key].shape[-1] < 2:
                behav_data[key] = np.concatenate(behav_data[key])
        pos = behav_data['cursor'] # Time by 2
        t_mark = behav_data['time']
        trial_mark = np.array(np.where(np.diff(t_mark)>10)).squeeze()
        if self.n_trials!=trial_mark.shape[0]+1:
            raise ValueError('Number of trials is not matched to time mark')
        ## reshape a 1D-array (sample) to a 2D-array (time, trial)
        pos = pos.reshape((-1,self.n_trials,2), order='F') # (time, trial, xy)
        t_mark = t_mark.reshape((-1,self.n_trials), order='F') # (time, trial)
        ## calculate the velocity and acceleration
        vel = np.diff(pos, axis=0)/np.tile(np.diff(t_mark, axis=0)[:,:,np.newaxis],2)
        acc = np.diff(vel, axis=0)/np.tile(np.diff(t_mark, axis=0)[1:,:,np.newaxis],2)
        ## concatenate pos, vel, and acc
        behav = np.concatenate([pos[:-2,:,:], vel[:-1,:,:], acc], axis=2)
        behav = behav.transpose((1,0,2))  ## (batch, time, xy)

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
        EMG = tmp
#         ## bandpass filter
#         b, a = signal.butter(N=5, Wn=[5,450], btype='bandpass', fs=self.emg_fs)
#         EMG = signal.lfilter(b, a, tmp, axis=0)
        ## reshape EMG from (sample, channel) to (time, trial, channel)
        EMG = EMG.reshape((-1, 20, 10), order='F')
        ## use STFT then get a spectrogram
        if self.FFT:
            win_length = int(self.emg_fs*self.win_length*.001)
            ## EMG_spec.shape = (channel, trial, freq, time)
            EMG_spec = Spectrogram(
                n_fft=2**8, win_length=win_length, hop_length=int(self.emg_fs*self.hope_length*.001)
            )(Tensor(EMG.T))
        EMG = EMG.transpose((1,0,2))
        EMG_spec = EMG_spec.detach().numpy().transpose((1, 0, 2, 3))

        return behav, EMG, EMG_spec


    