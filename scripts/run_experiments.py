"""Run the simulator and produce every figure and number used in the README.

    python scripts/run_experiments.py [--exe build/ecosystem] [--seeds 20]

Outputs go to results/: population plots, a phase portrait, grid snapshots,
an animated GIF and summary.json.
"""
import argparse
import csv
import json
import subprocess
import tempfile
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402
from PIL import Image  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
RESULTS = ROOT / "results"
COLORS = {"plants": "#2ca02c", "herbivores": "#1f77b4", "carnivores": "#d62728"}
RGB = {"P": (90, 170, 80), "H": (40, 110, 200), "C": (215, 40, 40)}
BACKGROUND = (245, 240, 225)


def simulate(exe, config, seed, out, steps=None):
    cmd = [str(exe), str(config), "--seed", str(seed), "--out", str(out)]
    if steps is not None:
        cmd += ["--steps", str(steps)]
    subprocess.run(cmd, check=True, capture_output=True)
    with open(Path(out) / "population.csv") as f:
        rows = list(csv.DictReader(f))
    return {k: np.array([int(r[k]) for r in rows]) for k in rows[0]}


def first_extinction(pop):
    dead = np.flatnonzero((pop["herbivores"] == 0) | (pop["carnivores"] == 0))
    return int(pop["step"][dead[0]]) if dead.size else None


def read_positions(path):
    frames = {}
    with open(path) as f:
        for r in csv.DictReader(f):
            frames.setdefault(int(r["step"]), []).append((int(r["x"]), int(r["y"]), r["type"]))
    return frames


def frame_image(cells, width, height):
    img = np.full((height, width, 3), BACKGROUND, dtype=np.uint8)
    for kind in "PHC":  # animals drawn over plants
        for x, y, t in cells:
            if t == kind:
                img[y, x] = RGB[t]
    return img


def plot_population(ax, pop, title):
    for k, c in COLORS.items():
        ax.plot(pop["step"], pop[k], color=c, lw=1.4, label=k)
    ax.set_yscale("symlog", linthresh=10)
    ax.set_xlabel("step")
    ax.set_ylabel("population")
    ax.set_title(title)
    ax.grid(alpha=0.3)
    ax.legend(loc="center right", fontsize=8)


def dominant_period(x, min_period=20, max_period=300):
    """Period of the strongest spectral peak after removing the slow trend."""
    trend = np.convolve(x, np.ones(101) / 101, mode="same")
    y = (x - trend)[50:-50]
    spec = np.abs(np.fft.rfft(y * np.hanning(y.size)))
    f = np.fft.rfftfreq(y.size)
    band = (f >= 1 / max_period) & (f <= 1 / min_period)
    return int(round(1 / f[band][np.argmax(spec[band])]))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", default=str(ROOT / "build" / "ecosystem"))
    ap.add_argument("--seeds", type=int, default=20)
    args = ap.parse_args()
    args.exe = Path(args.exe).resolve()
    RESULTS.mkdir(exist_ok=True)
    original, tuned = ROOT / "config" / "original.json", ROOT / "config" / "default.json"
    width = json.load(open(tuned))["grid_width"]
    height = json.load(open(tuned))["grid_height"]
    summary = {}

    with tempfile.TemporaryDirectory() as tmp:
        # 1. One run of each configuration
        pop_o = simulate(args.exe, original, 1, tmp)
        pop_t = simulate(args.exe, tuned, 1, tmp, steps=1000)
        frames = read_positions(Path(tmp) / "positions.csv")

        fig, axes = plt.subplots(1, 2, figsize=(12, 4.2))
        plot_population(axes[0], pop_o, "Original parameters: boom, bust, extinction")
        plot_population(axes[1], pop_t, "Tuned parameters: sustained predator-prey cycles")
        fig.tight_layout()
        fig.savefig(RESULTS / "population_original_vs_tuned.png", dpi=150)
        plt.close(fig)

        # 2. Phase portrait and cycle period (after a 200-step transient)
        h, c = pop_t["herbivores"][200:], pop_t["carnivores"][200:]
        period = dominant_period(h.astype(float))
        lag = int(np.argmax([np.corrcoef(h[:-L or None], c[L:])[0, 1] for L in range(0, period)]))
        fig, ax = plt.subplots(figsize=(5.5, 4.8))
        sc = ax.scatter(h, c, c=np.arange(h.size) + 200, s=4, cmap="viridis")
        ax.plot(h, c, color="gray", lw=0.4, alpha=0.6)
        fig.colorbar(sc, label="step")
        ax.set_xlabel("herbivores")
        ax.set_ylabel("carnivores")
        ax.set_title(f"Phase portrait (period ~ {period} steps, predators lag by ~{lag})")
        ax.grid(alpha=0.3)
        fig.tight_layout()
        fig.savefig(RESULTS / "phase_portrait.png", dpi=150)
        plt.close(fig)
        summary["tuned_seed1"] = {"period_steps": period, "predator_lag_steps": lag,
                                  "mean_after_transient": {k: float(pop_t[k][200:].mean()) for k in COLORS}}
        summary["original_seed1"] = {"first_extinction_step": first_extinction(pop_o),
                                     "peak": {k: int(pop_o[k].max()) for k in COLORS}}

        # 3. Grid snapshots and animated GIF
        snap_steps = [0, 50, 150, 300, 500, 1000]
        fig, axes = plt.subplots(1, len(snap_steps), figsize=(3 * len(snap_steps), 3.3))
        for ax, s in zip(axes, snap_steps):
            ax.imshow(frame_image(frames[s], width, height), interpolation="nearest")
            ax.set_title(f"step {s}")
            ax.axis("off")
        fig.suptitle("Green: plants, blue: herbivores, red: carnivores. Travelling waves of grazing and predation")
        fig.tight_layout()
        fig.savefig(RESULTS / "grid_snapshots.png", dpi=120)
        plt.close(fig)
        gif_steps = [s for s in sorted(frames) if s <= 600 and s % 5 == 0]
        images = [Image.fromarray(frame_image(frames[s], width, height)).resize((320, 320), Image.NEAREST)
                  for s in gif_steps]
        images[0].save(RESULTS / "ecosystem.gif", save_all=True, append_images=images[1:],
                       duration=60, loop=0, optimize=True)

        # 4. Robustness over many seeds
        for name, cfg in (("original", original), ("tuned", tuned)):
            ext = [first_extinction(simulate(args.exe, cfg, s, tmp, steps=1000)) for s in range(1, args.seeds + 1)]
            alive = [e for e in ext if e is None]
            summary[f"{name}_{args.seeds}_seeds"] = {
                "coexist_1000_steps": f"{len(alive)}/{len(ext)}",
                "extinction_steps": [e for e in ext if e is not None]}

    (RESULTS / "summary.json").write_text(json.dumps(summary, indent=2))
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
