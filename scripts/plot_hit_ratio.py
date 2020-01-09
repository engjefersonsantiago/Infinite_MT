import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.axes_grid1 import host_subplot

cache_size = [ '64', '128', '256', '512', '1024', '2048', '4096', '8192']

#Slowdown of 1

hr_lfu = [ 0.140645, 0.146035, 0.156401, 0.177007, 0.205551, 0.255405, 0.314457, 0.378781]
hr_wlfu = [ 0.139835, 0.159303,  0.179421, 0.210911, 0.261959, 0.324446, 0.418839, 0.503237]
hr_olfu = [ 0.375785, 0.413346, 0.447922, 0.487938, 0.529442, 0.584318, 0.663253, 0.71744]
hr_owlfu = [ 0.513771, 0.544384, 0.570556, 0.59932, 0.63502, 0.667598, 0.707892, 0.780801]
hr_lru = [ 0.512028, 0.535884, 0.566586, 0.604458, 0.639359, 0.672431, 0.736526, 0.778123]
hr_random = [ 0.476925, 0.51229, 0.54645, 0.580924, 0.616242, 0.656333, 0.701969, 0.745443]

import matplotlib.pyplot as plt

ax1 = host_subplot(111)

ax1.plot(cache_size, hr_lfu, c='red', marker='o',label='LFU')
ax1.plot(cache_size, hr_wlfu, c='cyan', marker='d',label='WLFU')
ax1.plot(cache_size, hr_olfu, c='orange', marker='P',label='OLFU')
ax1.plot(cache_size, hr_owlfu, c='black', marker='X',label='OWLFU')
ax1.plot(cache_size, hr_lru, c='blue', marker='s', label='LRU')
ax1.plot(cache_size, hr_random, c='green', marker='^', label='Random')

plt.legend(loc='upper left',ncol=2)
ax1.tick_params(axis ='x', which ='both', rotation = 45.0) 
plt.xticks(cache_size,cache_size)
plt.grid(True, axis='both')
#ax1.set_yticks(np.arange(0, hr_owlfu[7]+0.05, 0.1))
ax1.set_yticks(np.arange(0, 0.91, 0.1))

ax1.set_xlabel('Cache Size [Entries]')
ax1.set_ylabel('Hit Ratio')
#ax1.set_ylabel('Size Weighted Hit Ratio')

#ax1.set_xtick(np.arrage(len(cache_size)))
ax1.set_xticklabels(cache_size)

##Adjust label
plt.gcf().subplots_adjust(bottom=0.15)

plt.savefig("hit_ratio_sf1.pdf")
plt.clf()


#Weighted
hr_lfu = [ 0.189246, 0.194584, 0.207132, 0.232799, 0.26084, 0.298957, 0.348925, 0.420497]
hr_wlfu = [ 0.197753, 0.2299, 0.261048, 0.310832, 0.388137, 0.465832, 0.567007, 0.658162]
hr_olfu = [ 0.542637, 0.600932, 0.651874, 0.709894, 0.763159, 0.819725, 0.875377, 0.909122]
hr_owlfu = [ 0.718678, 0.751902, 0.780353, 0.811223, 0.849622, 0.881323, 0.912497, 0.947628]
hr_lru = [ 0.710093, 0.7302, 0.764309, 0.806843, 0.845444, 0.876835, 0.914078, 0.937735]
hr_random = [ 0.667432, 0.706192, 0.743432, 0.781008, 0.817884, 0.853593, 0.887418, 0.915903]

import matplotlib.pyplot as plt

ax1 = host_subplot(111)

ax1.plot(cache_size, hr_lfu, c='red', marker='o',label='LFU')
ax1.plot(cache_size, hr_wlfu, c='cyan', marker='d',label='WLFU')
ax1.plot(cache_size, hr_olfu, c='orange', marker='P',label='OLFU')
ax1.plot(cache_size, hr_owlfu, c='black', marker='X',label='OWLFU')
ax1.plot(cache_size, hr_lru, c='blue', marker='s', label='LRU')
ax1.plot(cache_size, hr_random, c='green', marker='^', label='Random')

plt.legend(loc='upper left',ncol=2)
ax1.tick_params(axis ='x', which ='both', rotation = 45.0) 
plt.xticks(cache_size,cache_size)
plt.grid(True, axis='both')
#ax1.set_yticks(np.arange(0, 1.01, 0.1))
ax1.set_yticks(np.arange(0, 1.01, 0.1))

ax1.set_xlabel('Cache Size [Entries]')
ax1.set_ylabel('Size Weighted Hit Ratio')

ax1.set_xticklabels(cache_size)

##Adjust label
plt.gcf().subplots_adjust(bottom=0.15)

plt.savefig("weighted_hit_ratio_sf1.pdf")
plt.clf()


#Slowdown of 10

hr_lfu = [ 0.0500428, 0.0712384, 0.0883832,  0.111215, 0.142809, 0.197382, 0.263934, 0.343526]
hr_wlfu = [ 0.0583096, 0.0796963, 0.107218, 0.152972, 0.212647, 0.294384, 0.39484, 0.49289]
hr_olfu = [ 0.263294, 0.295123, 0.334427, 0.381698, 0.441084, 0.521256, 0.595733,  0.657677]
hr_owlfu = [  0.31796, 0.356211, 0.402444, 0.455432, 0.508667, 0.565512, 0.644251, 0.72209]
hr_lru = [ 0.307441, 0.371289,  0.420726, 0.467872, 0.511303, 0.588226, 0.648161, 0.692325]
hr_random = [ 0.251857,  0.304028, 0.356522, 0.408248, 0.461283, 0.51789, 0.580871, 0.650761]

import matplotlib.pyplot as plt

ax1 = host_subplot(111)

ax1.plot(cache_size, hr_lfu, c='red', marker='o',label='LFU')
ax1.plot(cache_size, hr_wlfu, c='cyan', marker='d',label='WLFU')
ax1.plot(cache_size, hr_olfu, c='orange', marker='P',label='OLFU')
ax1.plot(cache_size, hr_owlfu, c='black', marker='X',label='OWLFU')
ax1.plot(cache_size, hr_lru, c='blue', marker='s', label='LRU')
ax1.plot(cache_size, hr_random, c='green', marker='^', label='Random')

plt.legend(loc='upper left',ncol=2)
ax1.tick_params(axis ='x', which ='both', rotation = 45.0) 
plt.xticks(cache_size,cache_size)
plt.grid(True, axis='both')
#ax1.set_yticks(np.arange(0, hr_owlfu[7]+0.05, 0.1))
ax1.set_yticks(np.arange(0, 0.91, 0.1))

ax1.set_xlabel('Cache Size [Entries]')
ax1.set_ylabel('Hit Ratio')

#ax1.set_xtick(np.arrage(len(cache_size)))
ax1.set_xticklabels(cache_size)

##Adjust label
plt.gcf().subplots_adjust(bottom=0.15)

plt.savefig("hit_ratio_sf10.pdf")
plt.clf()

#Weighted
hr_lfu = [ 0.0748705, 0.0849807, 0.108391, 0.137867, 0.173723, 0.234745, 0.30431, 0.383965]
hr_wlfu = [ 0.090826, 0.123937, 0.169045, 0.241005, 0.326722, 0.428652, 0.538091, 0.64198]
hr_olfu = [ 0.394782, 0.443141, 0.503044, 0.572061, 0.651147, 0.737899, 0.803485, 0.849709]
hr_owlfu = [ 0.464473, 0.519201, 0.584335, 0.657599, 0.725768, 0.789511, 0.855285, 0.90133]
hr_lru = [ 0.439792, 0.533882, 0.603269, 0.664721, 0.713714, 0.768156, 0.817612, 0.857575]
hr_random = [ 0.367173, 0.440168, 0.512418,  0.580994, 0.645387, 0.70673, 0.766343, 0.825239]


import matplotlib.pyplot as plt

ax1 = host_subplot(111)

ax1.plot(cache_size, hr_lfu, c='red', marker='o',label='LFU')
ax1.plot(cache_size, hr_wlfu, c='cyan', marker='d',label='WLFU')
ax1.plot(cache_size, hr_olfu, c='orange', marker='P',label='OLFU')
ax1.plot(cache_size, hr_owlfu, c='black', marker='X',label='OWLFU')
ax1.plot(cache_size, hr_lru, c='blue', marker='s', label='LRU')
ax1.plot(cache_size, hr_random, c='green', marker='^', label='Random')

plt.legend(loc='upper left',ncol=2)
ax1.tick_params(axis ='x', which ='both', rotation = 45.0) 
plt.xticks(cache_size,cache_size)
plt.grid(True, axis='both')
#ax1.set_yticks(np.arange(0, hr_owlfu[7]+0.05, 0.1))
ax1.set_yticks(np.arange(0, 1.01, 0.1))

ax1.set_xlabel('Cache Size [Entries]')
ax1.set_ylabel('Size Weighted Hit Ratio')

ax1.set_xticklabels(cache_size)

##Adjust label
plt.gcf().subplots_adjust(bottom=0.15)

plt.savefig("weighted_hit_ratio_sf10.pdf")
plt.clf()

#slowdown 100
hr_lfu = [ 0.0404024, 0.0548628, 0.0705874, 0.108116, 0.159101, 0.228518, 0.312078, 0.395493]
hr_wlfu = [0.0381802, 0.0584243, 0.0898663, 0.14739, 0.217505, 0.30227, 0.405013, 0.52219]
hr_olfu =  [ 0.145086, 0.190393, 0.236304, 0.298946, 0.369033, 0.445534, 0.516992, 0.574646]
hr_owlfu = [ 0.187586, 0.233917, 0.28747, 0.350576, 0.422982, 0.516831, 0.608637, 0.673009]
hr_lru = [ 0.206822, 0.238445, 0.272603, 0.307951, 0.381341, 0.481079, 0.594285, 0.669165]
hr_random = [ 0.130891, 0.16627, 0.207446, 0.258369, 0.323263, 0.401838, 0.487116, 0.568082]

import matplotlib.pyplot as plt

ax1 = host_subplot(111)

ax1.plot(cache_size, hr_lfu, c='red', marker='o',label='LFU')
ax1.plot(cache_size, hr_wlfu, c='cyan', marker='d',label='WLFU')
ax1.plot(cache_size, hr_olfu, c='orange', marker='P',label='OLFU')
ax1.plot(cache_size, hr_owlfu, c='black', marker='X',label='OWLFU')
ax1.plot(cache_size, hr_lru, c='blue', marker='s', label='LRU')
ax1.plot(cache_size, hr_random, c='green', marker='^', label='Random')

plt.legend(loc='upper left',ncol=2)
ax1.tick_params(axis ='x', which ='both', rotation = 45.0) 
plt.xticks(cache_size,cache_size)
plt.grid(True, axis='both')
#ax1.set_yticks(np.arange(0, hr_owlfu[7]+0.05, 0.1))
ax1.set_yticks(np.arange(0, 0.91, 0.1))

ax1.set_xlabel('Cache Size [Entries]')
ax1.set_ylabel('Hit Ratio')
#ax1.set_ylabel('Size Weighted Hit Ratio')

#ax1.set_xtick(np.arrage(len(cache_size)))
ax1.set_xticklabels(cache_size)

##Adjust label
plt.gcf().subplots_adjust(bottom=0.15)

plt.savefig("hit_ratio_sf100.pdf")
plt.clf()

#Weighted
hr_lfu = [ 0.0387561, 0.0613918, 0.0828191, 0.135683, 0.202871, 0.291501, 0.384546, 0.474001]
hr_wlfu = [ 0.0611505, 0.0944225, 0.14302, 0.232512, 0.327869, 0.434685, 0.546786, 0.68259]
hr_olfu =  [ 0.207914, 0.278939, 0.348502, 0.438039, 0.531159, 0.618358, 0.689488, 0.737165]
hr_owlfu = [ 0.27768, 0.349129, 0.430798, 0.521389, 0.617489, 0.715039, 0.786366, 0.840665]
hr_lru = [ 0.304681, 0.350534, 0.392898, 0.421408, 0.494409, 0.611677, 0.748812, 0.82598]
hr_random = [ 0.194291, 0.244786, 0.303459, 0.37335, 0.460122, 0.557247, 0.653926, 0.737559]


import matplotlib.pyplot as plt

ax1 = host_subplot(111)

ax1.plot(cache_size, hr_lfu, c='red', marker='o',label='LFU')
ax1.plot(cache_size, hr_wlfu, c='cyan', marker='d',label='WLFU')
ax1.plot(cache_size, hr_olfu, c='orange', marker='P',label='OLFU')
ax1.plot(cache_size, hr_owlfu, c='black', marker='X',label='OWLFU')
ax1.plot(cache_size, hr_lru, c='blue', marker='s', label='LRU')
ax1.plot(cache_size, hr_random, c='green', marker='^', label='Random')

plt.legend(loc='upper left',ncol=2)
ax1.tick_params(axis ='x', which ='both', rotation = 45.0) 
plt.xticks(cache_size,cache_size)
plt.grid(True, axis='both')
#ax1.set_yticks(np.arange(0, hr_owlfu[7]+0.05, 0.1))
ax1.set_yticks(np.arange(0, 1.01, 0.1))

ax1.set_xlabel('Cache Size [Entries]')
ax1.set_ylabel('Size Weighted Hit Ratio')

#ax1.set_xtick(np.arrage(len(cache_size)))
ax1.set_xticklabels(cache_size)

##Adjust label
plt.gcf().subplots_adjust(bottom=0.15)

plt.savefig("weighted_hit_ratio_sf100.pdf")
plt.clf()
#plt.show()


