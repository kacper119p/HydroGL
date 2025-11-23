import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib import cm
from matplotlib import ticker

WIND_VELOCITY = np.array([0.0, 16.0])  # m/s
FETCH = 100.0  # meters
DELTA = 1.0  # directional spread factor
SWELL = 1.0  # swell factor
GAMMA = 3.3  # JONSWAP peak enhancement factor
GRAVITY = 9.81  # m/s^2


def calculate_alpha(wind_velocity, fetch, gravity=GRAVITY):
    u = np.linalg.norm(wind_velocity)
    ratio = (u * u) / (fetch * gravity)
    return 0.076 * ratio ** 0.22


def calculate_peak_wave_frequency(wind_velocity, fetch, gravity=GRAVITY):
    u = np.linalg.norm(wind_velocity)
    return 22.0 * gravity * gravity / (u * fetch)


ALPHA = calculate_alpha(WIND_VELOCITY, FETCH)
PEAK_WAVE_FREQUENCY = calculate_peak_wave_frequency(WIND_VELOCITY, FETCH)


def calculate_r(omega, omega_p):
    sigma = 0.07 if omega <= omega_p else 0.09
    return np.exp(-((omega - omega_p) ** 2) / (2 * sigma ** 2 * omega_p ** 2))


def jonswap(omega):
    r = calculate_r(omega, PEAK_WAVE_FREQUENCY)
    omega_ratio = PEAK_WAVE_FREQUENCY / omega
    return (ALPHA * GRAVITY ** 2 / (omega ** 5 * 16 * np.pi ** 4)) * \
        np.exp(-1.25 * omega_ratio ** 4) * GAMMA ** r


def calculate_beta(r_omega):
    if r_omega < 0.95:
        return 2.61 * r_omega ** 1.3
    elif r_omega < 1.6:
        return 2.28 * r_omega ** (-1.3)
    else:
        epsilon = 0.8393 * np.exp(-0.567 * np.log(r_omega ** 2)) - 0.4
        return 10 ** epsilon


def donelan_banner_spread(beta, wind_bias):
    qdb = 1.0 / np.tanh(beta * np.pi)
    sech = 1.0 / np.cosh(beta * wind_bias)
    return 0.5 * qdb * beta * sech ** 2


def swell_spread(wind_bias, r_omega):
    return np.abs(np.cos(wind_bias / 2.0)) ** (32.0 * np.tanh(1.0 / r_omega) * SWELL ** 2)


def calculate_normalization_factor(r_omega, beta):
    interval = 0.1
    theta_range = np.arange(-np.pi + interval * 0.5, np.pi + interval, interval)
    sample_count = len(theta_range)

    result = 0.0
    for theta in theta_range:
        sw_spread = swell_spread(theta, r_omega)
        db_spread = donelan_banner_spread(beta, theta)
        result += db_spread * sw_spread

    return sample_count / (result * 2.0 * np.pi)


def directional_spectrum(k_vec, omega):
    r_omega = omega / PEAK_WAVE_FREQUENCY
    wind_dir = np.arctan2(WIND_VELOCITY[1], WIND_VELOCITY[0])
    wave_dir = np.arctan2(k_vec[1], k_vec[0])
    wind_bias = wave_dir - wind_dir
    if wind_bias > np.pi:
        wind_bias -= 2.0 * np.pi
    elif wind_bias < -np.pi:
        wind_bias += 2.0 * np.pi

    beta = calculate_beta(r_omega)

    db_spread = donelan_banner_spread(beta, wind_bias)
    sw_spread = swell_spread(wind_bias, r_omega)

    norm_factor = calculate_normalization_factor(r_omega, beta)

    final_spectrum = db_spread * sw_spread * norm_factor
    return (1 - DELTA) / (2 * np.pi) + DELTA * final_spectrum


def wave_spectrum(k_vec):
    k = np.linalg.norm(k_vec)
    if k < 1e-5:
        return 0.0

    omega = np.sqrt(GRAVITY * k)  # Deep water dispersion
    freq_spectrum = jonswap(omega)
    dir_spectrum = directional_spectrum(k_vec, omega)

    return freq_spectrum * dir_spectrum


def directional_spread_only(k_vec):
    k = np.linalg.norm(k_vec)
    if k < 1e-5:
        return 0.0

    omega = np.sqrt(GRAVITY * k)
    return directional_spectrum(k_vec, omega)


def ten_pow_formatter(val, pos):
    if abs(val - round(val)) < 1e-6:
        return r'$10^{%d}$' % int(round(val))
    return r'$10^{%.1f}$' % val


# Create wavenumber grid
k_max = 2.0  # rad/m
k_res = 100
k_x = np.linspace(-k_max, k_max, k_res)
k_y = np.linspace(-k_max, k_max, k_res)
K_X, K_Y = np.meshgrid(k_x, k_y)

# Calculate spectrum
S = np.zeros_like(K_X)
for i in range(k_res):
    for j in range(k_res):
        k_vec = np.array([K_X[i, j], K_Y[i, j]])
        S[i, j] = wave_spectrum(k_vec)

# Plot 3D Full Spectrum
fig = plt.figure(figsize=(12, 5), dpi=300)
fig.subplots_adjust(left=0.07, right=0.98, top=0.95, bottom=0.10, wspace=0.35)

ax1 = fig.add_subplot(121, projection='3d')
surf = ax1.plot_surface(K_X, K_Y, np.log10(S + 1e-10), cmap=cm.viridis, linewidth=0, antialiased=True)

ax1.set_xlabel(r'$k_x\ \left[\frac{rad}{m}\right]$')
ax1.set_ylabel(r'$k_y\ \left[\frac{rad}{m}\right]$')
ax1.set_zlabel(r'$\tilde{h}_{0}(k)$', labelpad=10)
ax1.set_title('3D Wave Spectrum (log scale)')

ax1.zaxis.set_major_locator(ticker.MaxNLocator(nbins=6))
ax1.zaxis.set_major_formatter(ticker.FuncFormatter(ten_pow_formatter))

cb = fig.colorbar(surf, ax=ax1, shrink=0.5, pad=0.15)
formatter = ticker.FuncFormatter(ten_pow_formatter)
cb.formatter = formatter
cb.ax.yaxis.set_major_formatter(formatter)
cb.update_ticks()
plt.draw()

# Plot 2D
ax2 = fig.add_subplot(122)
contour = ax2.contourf(K_X, K_Y, np.log10(S + 1e-10), levels=20, cmap=cm.viridis)
ax2.set_xlabel(r'$k_x\ \left[\frac{rad}{m}\right]$')
ax2.set_ylabel(r'$k_y\ \left[\frac{rad}{m}\right]$')
ax2.set_title('2D Wave Spectrum (log scale)')
ax2.set_aspect('equal')

cb = fig.colorbar(contour, ax=ax2, pad=0.05)

formatter = ticker.FuncFormatter(ten_pow_formatter)
cb.formatter = formatter
cb.ax.yaxis.set_major_formatter(formatter)
cb.update_ticks()

plt.tight_layout(pad=3.0)
plt.show()

# Calculate Directional spectrum
D = np.zeros_like(K_X)
for i in range(k_res):
    for j in range(k_res):
        k_vec = np.array([K_X[i, j], K_Y[i, j]])
        D[i, j] = directional_spread_only(k_vec)

# Plot 3D Directional Spectrum
fig = plt.figure(figsize=(12, 5), dpi=300)
fig.subplots_adjust(left=0.07, right=0.98, top=0.95, bottom=0.10, wspace=0.35)

ax1 = fig.add_subplot(121, projection='3d')
surf = ax1.plot_surface(K_X, K_Y, D, cmap=cm.viridis, linewidth=0, antialiased=True)
ax1.set_xlabel(r'$k_x\ \left[\frac{rad}{m}\right]$')
ax1.set_ylabel(r'$k_y\ \left[\frac{rad}{m}\right]$')
ax1.set_zlabel(r'$D(k)$', labelpad=10)
ax1.set_title('Directional Spread')
fig.colorbar(surf, ax=ax1, shrink=0.5, pad=0.15)

# Plot 2D
ax2 = fig.add_subplot(122)
contour = ax2.contourf(K_X, K_Y, np.log10(D + 1e-10), levels=20, cmap=cm.viridis)
ax2.set_xlabel(r'$k_x\ \left[\frac{rad}{m}\right]$')
ax2.set_ylabel(r'$k_y\ \left[\frac{rad}{m}\right]$')
ax2.set_title('Directional Spread (log scale)')
ax2.set_aspect('equal')

cb = fig.colorbar(contour, ax=ax2, pad=0.05)

formatter = ticker.FuncFormatter(ten_pow_formatter)
cb.formatter = formatter
cb.ax.yaxis.set_major_formatter(formatter)
cb.update_ticks()

plt.tight_layout(pad=3.0)
plt.show()

# Plot 1D frequency spectrum
omega_range = np.linspace(0.75, 4.0, 1000)
S_omega = np.array([jonswap(w) for w in omega_range])

plt.figure(figsize=(8, 5), dpi=300)
plt.plot(omega_range, S_omega)
plt.xlabel(r'Dispersion $\omega\ \left[\frac{rad}{s}\right]$')
plt.ylabel(r'$S(\omega)\ \left[\frac{m^2s}{rad}\right]$')
plt.title('JONSWAP Frequency Spectrum')
plt.grid(True)
plt.show()

# Plot 1D frequency spectrum segmented and colored
freq_hz = omega_range / (2.0 * np.pi)
S_f = np.array([jonswap(w) * (16.0 * np.pi ** 4) * (2.0 * np.pi) for w in omega_range])

k_t1 = 12.0 / (16.0 * np.pi)
k_t2 = 12.0 / (4.0 * np.pi)
f_t1 = np.sqrt(GRAVITY * k_t1) / (2.0 * np.pi)  # ≈ 0.244 Hz
f_t2 = np.sqrt(GRAVITY * k_t2) / (2.0 * np.pi)  # ≈ 0.487 Hz

colors = ['#2b91c4', '#00c39a', '#ebcb00']
hatch = '///'
masks = [
    freq_hz <= f_t1,
    (freq_hz > f_t1) & (freq_hz <= f_t2),
    freq_hz > f_t2
]

plt.figure(figsize=(8, 5), dpi=300)

handles = []
for mask, color in zip(masks, colors):
    x_seg = freq_hz[mask]
    y_seg = S_f[mask]
    if x_seg.size == 0:
        continue
    plt.plot(x_seg, y_seg, color=color, linewidth=1.5, zorder=3)
    plt.fill_between(x_seg, y_seg, color=color, alpha=0.25, zorder=1)
    plt.fill_between(x_seg, y_seg, facecolor='none', edgecolor=color, hatch=hatch, linewidth=0.0, zorder=2)
    handles.append(mpatches.Patch(facecolor=color, edgecolor=color, hatch=hatch, alpha=0.25))

labels = [
    'Cascade 0',
    'Cascade 1',
    'Cascade 2'
]

plt.xlabel(r'Frequency $f\ [Hz]$')
plt.ylabel(r'Wave Spectral Density $\left[\frac{m^{2}}{Hz}\right]$')
plt.title('JONSWAP Frequency Spectrum')
plt.grid(True)
plt.legend(handles, labels[:len(handles)], loc='upper right')
plt.show()
