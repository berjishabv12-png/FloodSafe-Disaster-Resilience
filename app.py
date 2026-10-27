from datetime import datetime, timezone
from flask import Flask, jsonify, request, render_template
import os

app = Flask(__name__)
LOW_MAX_CM = 5.0
CAUTION_MAX_CM = 15.0
STALE_AFTER_SECONDS = 30
SENSOR_HEIGHT_CM = float(os.getenv("SENSOR_HEIGHT_CM", "30.0"))
latest = {}

def classify(depth):
    if depth < LOW_MAX_CM:
        return "LOW"
    if depth <= CAUTION_MAX_CM:
        return "CAUTION"
    return "HIGH WATER"

@app.get("/")
def dashboard():
    return render_template("index.html")

@app.get("/api/roads")
def get_roads():
    now = datetime.now(timezone.utc)
    roads = []
    for road_id, item in latest.items():
        age = (now - item["received_at"]).total_seconds()
        stale = age > STALE_AFTER_SECONDS
        roads.append({
            "road_id": road_id,
            "distance_cm": round(item["distance_cm"], 2),
            "water_depth_cm": round(item["water_depth_cm"], 2),
            "status": "UNKNOWN" if stale else item["status"],
            "last_updated": item["timestamp"],
            "age_seconds": round(age, 1),
            "stale": stale
        })
    return jsonify({"roads": roads, "server_time": now.isoformat(),
                    "note": "Prototype only; UNKNOWN means no recent reading."})

@app.post("/api/update")
def update_reading():
    data = request.get_json(silent=True) or {}
    road_id = str(data.get("road_id", "ROAD_1")).strip()[:40]
    try:
        distance = float(data["distance_cm"])
    except (KeyError, TypeError, ValueError):
        return jsonify({"error": "Provide numeric distance_cm."}), 400
    if not road_id or distance < 0 or distance > 500:
        return jsonify({"error": "Invalid road_id or distance_cm."}), 400
    depth = max(0.0, SENSOR_HEIGHT_CM - distance)
    now = datetime.now(timezone.utc)
    status = classify(depth)
    latest[road_id] = {"distance_cm": distance, "water_depth_cm": depth,
                       "status": status, "timestamp": now.isoformat(timespec="seconds"),
                       "received_at": now}
    return jsonify({"ok": True, "road_id": road_id, "distance_cm": round(distance, 2),
                    "water_depth_cm": round(depth, 2), "status": status})

if __name__ == "__main__":
    # Local demonstration only; do not expose this development server publicly.
    app.run(host="0.0.0.0", port=5000, debug=False)
