import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.axes_grid1 import host_subplot

cache_size = [ '1024', '2048', '4096', '8192', '16384', '32768' ]
hr_lfu = [ 0.101825, 0.151517, 0.218346,0.308362,0.379355,0.472623]
hr_lru = [ 0.013797,0.0380477,0.0526254,0.0721625,0.115248,0.167093]
ht_random = [ 0.323655,0.395717,0.479216,0.561566,0.63325,0.710234]

ax1 = host_subplot(111)

#ax2 = ax1.twinx()

#ax2.set_ylim([0, 3])


ax1.scatter(cache_size, hr_lfu, c='red', marker='o',label='LFU')
ax1.scatter(cache_size, hr_lru, c='blue', marker='s', label='LRU')
ax1.scatter(cache_size, ht_random, c='green', marker='^', label='Random')

plt.legend(loc='upper left')
ax1.tick_params(axis ='x', which ='both', rotation = 45.0) 
plt.xticks(cache_size,cache_size)

ax1.set_xlabel('Cache Size [Entries]')
ax1.set_ylabel('Hit Ratio')

#ax1.set_xtick(np.arrage(len(cache_size)))
ax1.set_xticklabels(cache_size)

##Adjust label
plt.gcf().subplots_adjust(bottom=0.15)

plt.savefig("hit_ratio.pdf")
plt.show()


