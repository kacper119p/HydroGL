import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib import cm
from matplotlib import ticker

# ==========================================
# Constants and Parameters
# ==========================================
WIND_VELOCITY = np.array([0.0, 16.0])  # m/s
FETCH = 100.0  # meters
DELTA = 1.0  # directional spread factor (mix between isotropic and directional)
SWELL = 1.0  # swell factor
GAMMA = 3.3  # JONSWAP peak enhancement factor
GRAVITY = 9.81  # m/s^2
PI = np.pi

def calculate_alpha(wind_velocity, fetch, gravity=GRAVITY):
    u = np.linalg.norm(wind_velocity)
    ratio = (u * u) / (fetch * gravity)
    return 0.076 * ratio ** 0.22


def calculate_peak_wave_frequency(wind_velocity, fetch, gravity=GRAVITY):
    u = np.linalg.norm(wind_velocity)
    return 22.0 * gravity * gravity / (u * fetch)


ALPHA = calculate_alpha(WIND_VELOCITY, FETCH)
PEAK_WAVE_FREQUENCY = calculate_peak_wave_frequency(WIND_VELOCITY, FETCH)


# ==========================================
# Core Spectrum Logic (Matching InitialSpectrum.comp)
# ==========================================

def calculate_r(omega):
    sigma = 0.07 if omega <= PEAK_WAVE_FREQUENCY else 0.09
    numerator = omega - PEAK_WAVE_FREQUENCY
    denominator = -2.0 * sigma * sigma * PEAK_WAVE_FREQUENCY * PEAK_WAVE_FREQUENCY
    return np.exp((numerator * numerator) / denominator)


def calculate_beta(r_omega):
    if r_omega < 0.95:
        return 2.61 * (r_omega ** 1.3)
    elif r_omega < 1.6:
        return 2.28 * (r_omega ** -1.3)
    else:
        epsilon = 0.8393 * np.exp(-0.567 * np.log(r_omega ** 2)) - 0.4
        return 10.0 ** epsilon


def donelan_banner_spread(beta, wind_bias):
    qdb = 1.0 / np.tanh(beta * PI)
    sech = 1.0 / np.cosh(beta * wind_bias)
    return 0.5 * qdb * beta * sech * sech


def swell_spread(wind_bias, r_omega):
    return np.abs(np.cos(wind_bias / 2.0)) ** (32.0 * np.tanh(1.0 / r_omega) * SWELL * SWELL)


def jonswap(omega):
    r = calculate_r(omega)

    omega2 = omega * omega
    omega5 = omega2 * omega2 * omega

    omega_p_by_omega = PEAK_WAVE_FREQUENCY / omega
    omega_p_by_omega4 = omega_p_by_omega ** 4

    return ALPHA * (GRAVITY ** 2) / omega5 * np.exp(-1.25 * omega_p_by_omega4) * (GAMMA ** r)


def calculate_normalization_factor(r_omega, beta):
    interval = 0.1
    theta_range = np.arange(-PI + interval * 0.5, PI, interval)
    sample_count = len(theta_range)

    result = 0.0
    for theta in theta_range:
        sw_spread = swell_spread(theta, r_omega)
        db_spread = donelan_banner_spread(beta, theta)
        result += db_spread * sw_spread

    return sample_count / (result * 2.0 * PI)


def directional_spectrum_val(k_vec, omega):
    r_omega = omega / PEAK_WAVE_FREQUENCY

    wind_dir = np.arctan2(WIND_VELOCITY[1], WIND_VELOCITY[0])
    wave_dir = np.arctan2(k_vec[1], k_vec[0])
    wind_bias = wave_dir - wind_dir

    if wind_bias > PI:
        wind_bias -= 2.0 * PI
    elif wind_bias < -PI:
        wind_bias += 2.0 * PI

    beta = calculate_beta(r_omega)

    db_spread = donelan_banner_spread(beta, wind_bias)
    sw_spread = swell_spread(wind_bias, r_omega)
    norm_factor = calculate_normalization_factor(r_omega, beta)

    final_spectrum = db_spread * sw_spread * norm_factor

    isotropic = 1.0 / (2.0 * PI)
    return (isotropic * (1.0 - DELTA)) + (final_spectrum * DELTA)


def wave_spectrum_k(k_vec):
    k = np.linalg.norm(k_vec)
    if k < 1e-5:
        return 0.0

    omega = np.sqrt(GRAVITY * k)

    freq_spectrum = jonswap(omega)
    dir_spectrum = directional_spectrum_val(k_vec, omega)

    omega_dk_by_k = (GRAVITY / (2.0 * omega)) / k

    return freq_spectrum * dir_spectrum * omega_dk_by_k


# ==========================================
# Plotting Formatters
# ==========================================
def ten_pow_formatter(val, pos):
    if abs(val - round(val)) < 1e-6:
        return r'$10^{%d}$' % int(round(val))
    return r'$10^{%.1f}$' % val


def ten_pow_from_value(x, pos):
    if x <= 0:
        return '0'
    e = np.log10(x)
    exponent = int(np.floor(e))
    mantissa = x / (10 ** exponent)
    return r'$%.1f \cdot 10^{%d}$' % (mantissa, exponent)


# ==========================================
# Main Execution & Plotting
# ==========================================
if __name__ == "__main__":
    # Setup grid
    k_max = 2.0
    k_res = 128
    k_x = np.linspace(-k_max, k_max, k_res)
    k_y = np.linspace(-k_max, k_max, k_res)
    K_X, K_Y = np.meshgrid(k_x, k_y)

    # Calculate S(k)
    S = np.zeros_like(K_X)
    for i in range(k_res):
        for j in range(k_res):
            k_vec = np.array([K_X[i, j], K_Y[i, j]])
            S[i, j] = wave_spectrum_k(k_vec)

    # --- Plot 1: 3D & 2D Full Wave Spectrum ---
    fig = plt.figure(figsize=(12, 5), dpi=150)
    fig.subplots_adjust(left=0.07, right=0.98, top=0.90, bottom=0.10, wspace=0.35)

    ax1 = fig.add_subplot(121, projection='3d')
    surf = ax1.plot_surface(K_X, K_Y, np.log10(np.maximum(S , 1e-20)), cmap=cm.viridis, linewidth=0, antialiased=True)
    ax1.set_xlabel(r'$k_x$')
    ax1.set_ylabel(r'$k_y$')
    ax1.set_zlabel(r'$\log_{10}(S(k))$', labelpad=10)
    ax1.set_title('3D Wave Spectrum S(k)')
    ax1.zaxis.set_major_locator(ticker.MaxNLocator(nbins=6))
    ax1.zaxis.set_major_formatter(ticker.FuncFormatter(ten_pow_formatter))
    cb1 = fig.colorbar(surf, ax=ax1, shrink=0.5, pad=0.15)
    cb1.formatter = ticker.FuncFormatter(ten_pow_formatter)
    cb1.update_ticks()

    ax2 = fig.add_subplot(122)
    contour = ax2.contourf(K_X, K_Y, np.log10(S + 1e-20), levels=20, cmap=cm.viridis)
    ax2.set_xlabel(r'$k_x$')
    ax2.set_ylabel(r'$k_y$')
    ax2.set_title('2D Wave Spectrum S(k)')
    ax2.set_aspect('equal')
    cb2 = fig.colorbar(contour, ax=ax2, pad=0.05)
    cb2.formatter = ticker.FuncFormatter(ten_pow_formatter)
    cb2.update_ticks()
    plt.show()

    # --- Plot 2: Directional Spread Only ---
    D = np.zeros_like(K_X)
    for i in range(k_res):
        for j in range(k_res):
            k_vec = np.array([K_X[i, j], K_Y[i, j]])
            k_len = np.linalg.norm(k_vec)
            if k_len > 1e-5:
                omega = np.sqrt(GRAVITY * k_len)
                D[i, j] = directional_spectrum_val(k_vec, omega)

    fig = plt.figure(figsize=(12, 5), dpi=150)
    fig.subplots_adjust(left=0.07, right=0.98, top=0.90, bottom=0.10, wspace=0.35)

    # 3D Plot (Linear Scale for shape visualization)
    ax1 = fig.add_subplot(121, projection='3d')
    surf = ax1.plot_surface(K_X, K_Y, D, cmap=cm.viridis, linewidth=0, antialiased=True)
    ax1.set_xlabel(r'$k_x$')
    ax1.set_ylabel(r'$k_y$')
    ax1.set_zlabel(r'$D(k)$')
    ax1.set_title('Directional Spread Function (Linear)')
    fig.colorbar(surf, ax=ax1, shrink=0.5, pad=0.15)

    # 2D Plot
    ax2 = fig.add_subplot(122)
    contour = ax2.contourf(K_X, K_Y, np.log10(D + 1e-10), levels=20, cmap=cm.viridis)
    ax2.set_xlabel(r'$k_x$')
    ax2.set_ylabel(r'$k_y$')
    ax2.set_title('Directional Spread (Log Scale)')
    ax2.set_aspect('equal')

    cb2 = fig.colorbar(contour, ax=ax2, pad=0.05)
    cb2.formatter = ticker.FuncFormatter(ten_pow_formatter)
    cb2.ax.yaxis.set_major_formatter(ticker.FuncFormatter(ten_pow_formatter))
    cb2.update_ticks()

    plt.show()

    # --- Plot 3: 1D Frequency Spectrum (S_omega) ---
    omega_range = np.linspace(0.5, 4.0, 1000)
    S_omega = np.array([jonswap(w) for w in omega_range])

    plt.figure(figsize=(8, 5), dpi=150)
    plt.plot(omega_range, S_omega)
    plt.xlabel(r'Dispersion $\omega\ \left[\frac{rad}{s}\right]$')
    plt.ylabel(r'$S(\omega)\ \left[\frac{m^2 s}{rad}\right]$', labelpad=2)
    plt.title('JONSWAP Frequency Spectrum (1D)')
    plt.grid(True)
    plt.show()

    # --- Plot 4: Segmented Frequency Spectrum ---
    freq_hz = omega_range / (2.0 * PI)
    S_f = np.array([jonswap(w) * (2.0 * PI) for w in omega_range])

    # Thresholds
    k_t1 = 12.0 / (16.0 * PI)
    k_t2 = 12.0 / (4.0 * PI)
    f_t1 = np.sqrt(GRAVITY * k_t1) / (2.0 * PI)
    f_t2 = np.sqrt(GRAVITY * k_t2) / (2.0 * PI)

    colors = ['#2b91c4', '#00c39a', '#ebcb00']
    hatch = '///'
    masks = [
        freq_hz <= f_t1,
        (freq_hz > f_t1) & (freq_hz <= f_t2),
        freq_hz > f_t2
    ]

    plt.figure(figsize=(8, 5), dpi=150)
    handles = []
    labels = ['Cascade 0', 'Cascade 1', 'Cascade 2']

    for i, mask in enumerate(masks):
        x_seg = freq_hz[mask]
        y_seg = S_f[mask]
        if x_seg.size == 0: continue
        col = colors[i % len(colors)]
        plt.plot(x_seg, y_seg, color=col, linewidth=1.5, zorder=3)
        plt.fill_between(x_seg, y_seg, color=col, alpha=0.25, zorder=1)
        plt.fill_between(x_seg, y_seg, facecolor='none', edgecolor=col, hatch=hatch, linewidth=0.0, zorder=2)
        handles.append(mpatches.Patch(facecolor=col, edgecolor=col, hatch=hatch, alpha=0.25))

    plt.xlabel(r'Frequency $f\ [Hz]$')
    plt.ylabel(r'Wave Spectral Density $S(f)\ \left[\frac{m^{2}}{Hz}\right]$')
    plt.title('JONSWAP Frequency Spectrum (Segmented)')
    plt.grid(True, alpha=0.3)
    plt.legend(handles, labels[:len(handles)], loc='upper right')
    plt.tight_layout()
    plt.show()