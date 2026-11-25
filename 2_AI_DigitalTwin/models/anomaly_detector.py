import math
from typing import Dict, List, Optional

FEATURES: List[str] = [
"Voltage",
"Current",
"Power_Factor",
"Temperature",
"Pressure",
"Active_Power",
"Apparent_Power",
"Impedance",
"Thermal_Index",
"ElectroMechanical_Ratio",
]

THRESHOLD = 33.9377194830621
LOG_2PI = 1.83787706640934548356

SCALER_MEAN: List[float] = [
220.06510118891958,
2.1047114286980055,
0.8201906060031251,
37.96330558394594,
101.27365101975913,
379.8948951236716,
463.17605290811497,
104.70498849596956,
0.1169493054286265,
3.7512258312905433,
]

SCALER_SCALE: List[float] = [
1.4677158309704295,
0.0789417444959397,
0.010006512753365235,
1.1568210375899732,
0.4046756827730691,
15.301657368778956,
17.712793244542812,
3.9776609940912646,
0.009356866650512902,
0.15171587914359358,
]

MODEL_MEAN: List[float] = [
3.859781226197656e-14,
1.880084092342552e-14,
-4.986964626895902e-14,
-3.268249731810676e-14,
7.765445120758139e-14,
-5.400424453743585e-15,
-5.583844596141558e-14,
-1.4381932223310726e-14,
6.55071498520325e-15,
1.0663636885997961e-15,
]

MODEL_VARIANCE: List[float] = [
1.000999999999999,
1.0010000000000008,
1.0010000000000003,
1.0009999999999983,
1.0010000000000012,
1.0009999999999988,
1.0010000000000001,
1.0009999999999988,
1.001,
1.0009999999999997,
]

def compute_nll_score(values: List[float]) -> float:
"""Computes Negative Log-Likelihood across the 10 MURSAD physical features."""
if len(values) != len(FEATURES):
raise ValueError(f"Expected {len(FEATURES)} features, got {len(values)}.")


Python
nll = 0.0for x, mu_s, scale_s, mu_m, var_m in zip(
    values, SCALER_MEAN, SCALER_SCALE, MODEL_MEAN, MODEL_VARIANCE
):
    if not (math.isfinite(x) and scale_s > 0.0 and var_m > 0.0):
        return float("nan")
    z = (x - mu_s) / scale_s
    diff = z - mu_m
    nll += 0.5 * (LOG_2PI + math.log(var_m) + (diff * diff) / var_m)
return nll
def evaluate_sample(sample: Dict[str, float], threshold: float = THRESHOLD) -> Dict[str, object]:
"""Evaluates a telemetry dictionary and checks GMM anomaly threshold & ±5% impedance deviation."""
vec = [float(sample[feat]) for feat in FEATURES]
score = compute_nll_score(vec)
gmm_anomaly = (not math.isfinite(score)) or (score > threshold)


Python
expected_z = SCALER_MEAN[FEATURES.index("Impedance")]
actual_z = float(sample["Impedance"])
z_deviation_pct = abs(actual_z - expected_z) / expected_z * 100.0 if expected_z else 0.0
physical_dev_gt_5pct = z_deviation_pct > 5.0

return {
    "nll_score": round(score, 4) if math.isfinite(score) else None,
    "threshold": threshold,
    "gmm_anomaly": gmm_anomaly,
    "z_deviation_pct": round(z_deviation_pct, 2),
    "physical_dev_gt_5pct": physical_dev_gt_5pct,
    "flag_isolation_review": gmm_anomaly or physical_dev_gt_5pct,
}
if name == "main":
baseline_sample: Dict[str, float] = {
"Voltage": 220.065,
"Current": 2.105,
"Power_Factor": 0.820,
"Temperature": 37.96,
"Pressure": 101.27,
"Active_Power": 379.89,
"Apparent_Power": 463.18,
"Impedance": 104.70,
"Thermal_Index": 0.117,
"ElectroMechanical_Ratio": 3.751,
}
print("Baseline evaluation:", evaluate_sample(baseline_sample))
