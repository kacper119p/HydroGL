import matplotlib.pyplot as plt

# Data extracted from the image
# Resolution (side length in pixels)
resolutions = [128, 256, 512, 1024]
# Calculation time in ms
times_ms = [0.38, 0.86, 3.25, 13.57]

# Create labels for the X-axis
labels = [f"{r}x{r} px" for r in resolutions]

# Setup the plot
plt.figure(figsize=(10, 6))

# Plotting the data
# Using 'resolutions' as x-values ensures the visual distance between points is proportional
plt.plot(resolutions, times_ms, marker='o', linestyle='-', color='#ff5555', label='Calculation Time')

# Formatting the chart
plt.xlabel('Rozdzielczość', fontsize=24)
plt.ylabel('Czas [ms]', fontsize=24)

# Setting custom tick labels for X-axis to match the table format
plt.xticks(resolutions, labels, fontsize=16, rotation=15)
plt.yticks(fontsize=16)
# Adding grid and legend
plt.grid(True, linestyle='--', alpha=0.7)

# Show the plot
plt.tight_layout()
plt.show()