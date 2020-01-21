import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.axes_grid1 import host_subplot
import matplotlib.colors as mcolors

sf = [ 'SF = 10x', 'SF = 100x']

ind = np.arange(len(sf))  # the x locations for the groups
width = 0.15  # the width of the bars

#Slowdown of 1

hr_owlfu_wmfu =   [ 0.737873, 0.674953]
hr_lru_wmfu =     [ 0.721148, 0.687144]
hr_random_wmfu =  [ 0.677436, 0.583817] 
hr_owlfu_owmfu =  [ 0.741261, 0.693476]
hr_lru_owmfu =    [ 0.725866, 0.705787]
hr_random_owmfu = [ 0.681026, 0.610689] 

import matplotlib.pyplot as plt

ax1 = host_subplot(111)

rects1 = ax1.bar(ind - 2.5*width, hr_owlfu_wmfu, width,
                label='OWLFU+WMFU', color='lightcoral', edgecolor='black',hatch='//')#, color='red')
rects2 = ax1.bar(ind - 1.5*width, hr_lru_wmfu, width,
                label='LRU+WMFU', color='cornflowerblue', edgecolor='black', hatch="\\\\")#, color='blue')
rects3 = ax1.bar(ind - width/2, hr_random_wmfu, width,
                label='Random+WMFU',  edgecolor='black', color='palegreen',hatch="--")#, color='green')
rects4 = ax1.bar(ind + width/2, hr_owlfu_owmfu, width,
                label='OWLFU+OWMFU', edgecolor='black', color='cyan',hatch="..")#, color='red')
rects5 = ax1.bar(ind + 1.5*width, hr_lru_owmfu, width,
                label='LRU+OWMFU', edgecolor='black', color='hotpink',hatch="||")#, color='blue')
rects6 = ax1.bar(ind + 2.5*width, hr_random_owmfu, width,
                label='Random+OWMFU', edgecolor='black', color='firebrick')#, color='green')

plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15),
          ncol=3, fancybox=True, prop={'size': 9} )
ax1.tick_params(axis ='x', which ='both')#, rotation = 45.0) 
#plt.xticks(cache_size,cache_size)
plt.grid(True, axis='both')
#ax1.set_yticks(np.arange(0, hr_owlfu[7]+0.05, 0.1))
ax1.set_yticks(np.arange(0, 0.75, 0.1))

ax1.set_xticks(ind)
ax1.set_xticklabels(sf)

#ax1.set_xlabel('Cache Size [Entries]')
ax1.set_ylabel('Hit Ratio')

#ax1.set_xtick(np.arrage(len(cache_size)))

##Adjust label
plt.gcf().subplots_adjust(bottom=0.15)

plt.savefig("promo.pdf", bbox_inches = 'tight', pad_inches = 0.1)
plt.clf()
#plt.show()

hr_owlfu_wmfu =   [ 0.925914, 0.887363]
hr_lru_wmfu =     [ 0.907204, 0.895346]
hr_random_wmfu =  [ 0.925914, 0.86013] 
hr_owlfu_owmfu =  [ 0.926508, 0.89452]
hr_lru_owmfu =    [ 0.907254, 0.887334]
hr_random_owmfu = [ 0.890574, 0.856564] 

import matplotlib.pyplot as plt

ax1 = host_subplot(111)

rects1 = ax1.bar(ind - 2.5*width, hr_owlfu_wmfu, width,
                label='OWLFU+WMFU', color='lightcoral', edgecolor='black',hatch='//')#, color='red')
rects2 = ax1.bar(ind - 1.5*width, hr_lru_wmfu, width,
                label='LRU+WMFU', color='cornflowerblue', edgecolor='black', hatch="\\\\")#, color='blue')
rects3 = ax1.bar(ind - width/2, hr_random_wmfu, width,
                label='Random+WMFU',  edgecolor='black', color='palegreen',hatch="--")#, color='green')
rects4 = ax1.bar(ind + width/2, hr_owlfu_owmfu, width,
                label='OWLFU+OWMFU', edgecolor='black', color='cyan',hatch="..")#, color='red')
rects5 = ax1.bar(ind + 1.5*width, hr_lru_owmfu, width,
                label='LRU+OWMFU', edgecolor='black', color='hotpink',hatch="||")#, color='blue')
rects6 = ax1.bar(ind + 2.5*width, hr_random_owmfu, width,
                label='Random+OWMFU', edgecolor='black', color='firebrick')#, color='green')

plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15),
          ncol=3, fancybox=True, prop={'size': 9} )
ax1.tick_params(axis ='x', which ='both')#, rotation = 45.0) 
#plt.xticks(cache_size,cache_size)
plt.grid(True, axis='both')
#ax1.set_yticks(np.arange(0, hr_owlfu[7]+0.05, 0.1))
ax1.set_yticks(np.arange(0, 0.95, 0.1))

ax1.set_xticks(ind)
ax1.set_xticklabels(sf)

#ax1.set_xlabel('Cache Size [Entries]')
ax1.set_ylabel('Size Weighted Hit Ratio')

#ax1.set_xtick(np.arrage(len(cache_size)))

##Adjust label
plt.gcf().subplots_adjust(bottom=0.15)

plt.savefig("promo_weighted.pdf", bbox_inches = 'tight', pad_inches = 0.1)

