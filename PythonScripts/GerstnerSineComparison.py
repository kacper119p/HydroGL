import numpy as np
import matplotlib.pyplot as plt

A_HORIZONTAL = 0.75
A_VERTICAL = 1.0
K  = 1.0
W  = 1.0
PHI = 0.0
T = 0.0

xi = np.linspace(0, 4*np.pi, 3000)
theta = K * xi

Xg = xi - A_HORIZONTAL * np.sin(theta)
Yg = A_VERTICAL * np.cos(theta)

idx = np.argsort(Xg)
Xg_sorted = Xg[idx]
Yg_sorted = Yg[idx]

gerstner_wave_length = Xg_sorted[np.searchsorted(xi, 2 * np.pi)] - Xg_sorted[0]

x = np.linspace(Xg_sorted.min(), Xg_sorted.min() + 2 * gerstner_wave_length, 3000)

Yg_uniform = np.interp(x, Xg_sorted, Yg_sorted)
crest_x = x[np.argmax(Yg_uniform)]
k_sine = 2 * np.pi / gerstner_wave_length
y_sine = A_VERTICAL * np.sin(k_sine * (x - crest_x) + np.pi / 2)

plt.figure(figsize=(10,4))
plt.plot(x, y_sine, '--', label="Sine", linewidth=2, color='#0f6e00')
plt.plot(x, Yg_uniform, label="Gerstner", linewidth=2, color='#cc000a')

plt.title("Gerstner vs Sine")
plt.xlabel("x")
plt.ylabel("Amplitude")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()
