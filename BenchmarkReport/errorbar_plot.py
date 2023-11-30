import matplotlib.pyplot as plt
from scipy.interpolate import interp1d
import numpy as np

plt.style.use('_mpl-gallery')

# set data:
np.random.seed(1)
x = [1, 2, 4, 8]                                               # threads used
y = [14.82, 9.75, 8.47, 8.36]                                  # average times per thread
yerr = [[4.35, 0.12, 0.21, 0.12],[0.36, 0.2, 0.56, 0.33]]      # lower and upper diff from average (in that order)

# plot:
fig, ax = plt.subplots()

ax.errorbar(x, y, yerr, fmt='o', linewidth=2, capsize=6)

ax.set(xlim=(0, 11), xticks=np.arange(0, 11),
       ylim=(6, 21), yticks=np.arange(6, 21))
# Set title and axis labels
plt.title('Wator parallelization benchmark', fontweight='bold')
plt.xlabel('Threads', style='italic', loc='center')
plt.ylabel('Time (seconds)', style='italic', loc='center')

plt.plot(x, y)       #plot the line


plt.show()