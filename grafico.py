import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv("dados.txt", delimiter=',')

sA_mtx = df[df['thread'] == 'sA_mtx']
sB_mtx = df[df['thread'] == 'sB_mtx']
sA = df[df['thread'] == 'sA']
sB = df[df['thread'] == 'sB']
pcA = df[df['thread'] == 'pcA']
pcB = df[df['thread'] == 'pcB']

fig, axs = plt.subplots(6, figsize=[10,13], tight_layout=True)
idx=0
axs[idx].plot(sA['time'], sA['status'])
axs[idx].set_title("Source A")
axs[idx].set_xlim([df["time"].iloc[0], df["time"].iloc[-1]])

idx+=1
axs[idx].plot(sA_mtx['time'], sA_mtx['status'])
axs[idx].set_title("MTX A")
axs[idx].set_xlim([df["time"].iloc[0], df["time"].iloc[-1]])

idx+=1
axs[idx].plot(sB['time'], sB['status'])
axs[idx].set_title("Source B")
axs[idx].set_xlim([df["time"].iloc[0], df["time"].iloc[-1]])

idx+=1
axs[idx].plot(sB_mtx['time'], sB_mtx['status'])
axs[idx].set_title("MTX B")
axs[idx].set_xlim([df["time"].iloc[0], df["time"].iloc[-1]])

idx+=1
axs[idx].plot(pcA['time'], pcA['status'])
axs[idx].set_title("proces A")
axs[idx].set_xlim([df["time"].iloc[0], df["time"].iloc[-1]])

idx+=1
axs[idx].plot(pcB['time'], pcB['status'])
axs[idx].set_title("proces B")
axs[idx].set_xlim([df["time"].iloc[0], df["time"].iloc[-1]])

plt.savefig('../profile.png')
plt.show()