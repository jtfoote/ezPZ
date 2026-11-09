<p align="center">
  <img src="ezpz.svg" width="150" alt="ezPZ Logo"/>
</p>

<h1 align="center">ezPZ</h1>
<p align="center">
  <b>Interactive Control Theory and Digital Signal Processing Tool</b>
</p>

---
**ezPZ** (formerly known as **Control Systems Suite**) is an all-in-one tool for electrical engineering students, specifically those in **Classical or Modern Control Theory** and **Digital Signals Processing**. ezPZ is cross-platform, built on `C++` with `Eigen3` and `Qt6`. The goal is to unify continuous-time $s$-plane root locus design, discrete-time $z$-plane digital filter visualization, time-domain response simulation, frequency analysis, block diagram algebra, Routh-Hurwitz stability tables, and signal plotting into a single desktop application.

---

## Features

* **1. $s$-plane Root Locus & Compensator Designer (`rootlocustab`)**

* **2. Continuous Time-Response & PID Synthesizer (`PIDTimeResponseTab`)**

* **3. $z$-plane Digital Filter Designer (`ZPlaneFilterTab`)**

* **4. Discrete-Time Response (`DiscreteTimeResponseTab`)**

* **5. Frequency-Domain Suite (`FrequencyResponseTab` & `BodyNyquistCanvas`)**

* **6. Routh-Hurwitz Table Constructor and Symbolic Solver (`RouthHurwitzTab`)**

* **7. Block Diagram Studio (`BlockDiagramStudioTab` & `BlockDiagramCanvas`)**

* **8. Signal Plotter, Scientific Calculator & Formula Reference**

* **9. Built-in Themes, Vector `.svg` Export & CLI Mode**

---

## Pre-Built Downloads and Running Instructions

Pre-compiled standalone bundles for **Windows**, **macOS (Universal Intel & Apple Silicon)**, and **Linux** are automatically built via GitHub Actions. Download them from the **Releases** page or the latest workflow run under the **Actions** tab.

### Windows (`ezPZ-Windows-x64.zip`)
1. Extract the entire `.zip` archive. Keep `ezpz.exe` alongside its bundled `.dll` files and plugin folders.
2. Launch `ezpz.exe`.
3. On first launch, Windows SmartScreen may prompt you. If so, click **More Info** $\rightarrow$ **Run Anyway**.

### macOS (`ezPZ-macOS-Universal.dmg` / `.zip`)
1. Open `ezPZ-macOS-Universal.dmg` and drag `ezPZ.app` to **Applications**.
2. Before first launch, you may need to bypass the Gatekeeper. To do so, run the following in your terminal:
    ```bash
    xattr -cr /Applications/ezPZ.app
    ```
### Linux (`ezPZ-Linux-x86_64`)
1. Unzip the artifact to access the standalone AppImage, portable tarball, and desktop menu installer:
#### Option A - 1-click Start Menu install (Recommended)
```bash
./install.sh
```
#### Option B - Direct AppImage execution
```bash
chmod +x ezPZ-Linux-x86_64.AppImage
./ezPZ-Linux-x86_64.AppImage
```


