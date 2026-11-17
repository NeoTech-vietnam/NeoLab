#!/usr/bin/env python3
"""Bounded NeoCam receiver benchmark. Does not change firmware qualification flags."""
import argparse
from collections import deque
from io import BytesIO
import json
import math
import platform
from pathlib import Path
import socket
import sys
import threading
import time
import urllib.error
import urllib.parse
import urllib.request
import http.client

MAX_FRAME = 2 * 1024 * 1024
MAX_SAMPLES = 200000
PROFILES = {"safe": (640, 480, 30), "smooth": (640, 480, 30),
            "balanced": (1280, 720, 30), "detail": (1600, 1200, 10)}
SCENES = ("good-light-detail", "motion", "plain", "low-light", "unspecified")
COUNTERS = ("capture_frames", "capture_failures", "oversized_frames", "stream_frames", "failures", "photos")
REQUIRED_METRICS = ("uptime_s", "capture_frames", "capture_failures", "oversized_frames",
                    "min_free_psram", "min_free_internal")


class StreamError(ValueError):
    pass


class MultipartReader:
    """Content-Length framing; bounds every line, header block, and frame."""
    def __init__(self, stream, boundary, max_frame=MAX_FRAME):
        if not boundary or len(boundary) > 200 or b"\r" in boundary or b"\n" in boundary:
            raise StreamError("invalid multipart boundary")
        self.stream, self.boundary, self.maximum = stream, b"--" + boundary, max_frame
        self.buffer = bytearray()

    def _more(self):
        read = getattr(self.stream, "read1", self.stream.read)
        data = read(4096)
        if not data:
            raise StreamError("truncated multipart stream")
        self.buffer.extend(data)

    def _line(self, limit=8192):
        while b"\n" not in self.buffer:
            if len(self.buffer) > limit:
                raise StreamError("multipart line too long")
            self._more()
        end = self.buffer.index(b"\n") + 1
        if end > limit:
            raise StreamError("multipart line too long")
        result = bytes(self.buffer[:end])
        del self.buffer[:end]
        return result

    def _exact(self, size):
        while len(self.buffer) < size:
            self._more()
        result = bytes(self.buffer[:size])
        del self.buffer[:size]
        return result

    def __iter__(self):
        while True:
            line = self._line().rstrip(b"\r\n")
            while line == b"":
                line = self._line().rstrip(b"\r\n")
            if line == self.boundary + b"--":
                return
            if line != self.boundary:
                raise StreamError("expected multipart boundary")
            headers, total = {}, 0
            while True:
                line = self._line()
                total += len(line)
                if total > 16384:
                    raise StreamError("multipart headers too large")
                if line in (b"\r\n", b"\n"):
                    break
                key, separator, value = line.partition(b":")
                if not separator:
                    raise StreamError("invalid multipart header")
                try:
                    name = key.decode("ascii").strip().lower()
                except UnicodeDecodeError as error:
                    raise StreamError("non-ASCII multipart header") from error
                if not name or name in headers:
                    raise StreamError("duplicate/empty multipart header")
                headers[name] = value.decode("latin1").strip()
            length = headers.get("content-length", "")
            if len(length) > 20 or not length.isascii() or not length.isdigit():
                raise StreamError("missing/invalid Content-Length")
            size = int(length)
            if size <= 0 or size > self.maximum:
                raise StreamError("JPEG length exceeds configured bound")
            if headers.get("content-type", "").split(";")[0].lower() != "image/jpeg":
                raise StreamError("multipart part is not JPEG")
            yield headers, self._exact(size)


def boundary_from(content_type):
    from email.message import Message
    message = Message()
    message["Content-Type"] = content_type
    if message.get_content_type() != "multipart/x-mixed-replace":
        raise StreamError("response is not multipart MJPEG")
    boundary = message.get_param("boundary")
    if boundary is None:
        raise StreamError("missing boundary parameter")
    return boundary.encode("ascii")


def decode_jpeg(payload, expected):
    """Full decode; markers alone do not establish a valid image."""
    from PIL import Image, ImageFile
    ImageFile.LOAD_TRUNCATED_IMAGES = False
    with Image.open(BytesIO(payload)) as image:
        if image.format != "JPEG":
            raise StreamError("payload is not JPEG")
        if image.size != tuple(expected):
            raise StreamError(f"unexpected image dimensions: {image.size}, expected {expected}")
        image.load()


def frame_identity(headers):
    names = ("x-profile-epoch", "x-sequence", "x-capture-time-us")
    if any(name not in headers for name in names):
        return None
    values = []
    for name in names:
        raw = headers[name]
        if len(raw) > 20 or not raw.isascii() or not raw.isdigit():
            raise StreamError("invalid frame identity/timestamp header")
        value = int(raw)
        if value > (2**32 - 1 if name == names[0] else 2**64 - 1):
            raise StreamError("frame identity/timestamp out of range")
        values.append(value)
    return tuple(values)


def percentile(values, fraction):
    if not values:
        return None
    ordered = sorted(values)
    return ordered[max(0, math.ceil(len(ordered) * fraction) - 1)]


def profile_metadata(status, name):
    entries = [entry for entry in status.get("profiles", []) if entry.get("id") == name]
    if len(entries) != 1:
        raise ValueError("missing/ambiguous firmware profile metadata")
    selected = entries[0]
    for field in ("width", "height", "target_fps", "jpeg_quality"):
        if not isinstance(selected.get(field), int) or isinstance(selected[field], bool):
            raise ValueError("invalid firmware profile metadata: " + field)
    if (selected["width"] <= 0 or selected["height"] <= 0 or selected["width"] * selected["height"] > 2048 * 1536
            or selected["target_fps"] <= 0 or not 0 <= selected["jpeg_quality"] <= 63):
        raise ValueError("firmware profile metadata outside supported bounds")
    return dict(selected)


def qualification(report):
    """Strict whole-run criteria, including deliberate photo pauses."""
    reasons = []
    measured = report["receiver"]
    if report["duration_s"] < 300:
        reasons.append("duration_below_300_seconds")
    if report["scene"] not in ("good-light-detail", "motion", "plain"):
        reasons.append("scene_is_diagnostic_only")
    if report["read_delay_ms"] != 0:
        reasons.append("deliberately_slowed_client")
    if report["interrupted"] or report["errors"]:
        reasons.append("incomplete_or_transport_error")
    if not report["profile_api_available"]:
        reasons.append("profile_api_unavailable")
    if measured["frames"] == 0 or measured["identity_frames"] != measured["frames"]:
        reasons.append("missing_frame_identity")
    if measured["samples_truncated"]:
        reasons.append("bounded_sample_limit_reached")
    if measured["decode_errors"] != 0 or measured["malformed_parts"] != 0:
        reasons.append("malformed_or_undecodable_JPEG")
    if measured.get("protocol_errors", 0) != 0:
        reasons.append("nonmonotonic_frame_identity_or_unexpected_epoch")
    if measured["unique_fps"] is None or measured["unique_fps"] < report["target_fps"] * 0.95:
        reasons.append("unique_FPS_below_95_percent_target")
    interval = measured["interval_p95_ms"]
    if interval is None or interval > 2000 / report["target_fps"]:
        reasons.append("receiver_interval_p95_above_two_frame_periods")
    metrics = report["metrics"]
    if (metrics["samples"] < 2 or metrics["poll_errors"] or metrics["reset_detected"]
            or not metrics.get("schema_complete", False) or metrics.get("unexpected_profile", False)):
        reasons.append("metrics_incomplete_or_reset_detected")
    for key in ("capture_failures", "oversized_frames"):
        if metrics["counter_deltas"].get(key) != 0:
            reasons.append(key + "_missing_or_increased")
    if metrics["min_free_psram"] is None or metrics["min_free_psram"] < 512 * 1024:
        reasons.append("insufficient_PSRAM_headroom")
    if metrics["min_free_internal"] is None or metrics["min_free_internal"] < 64 * 1024:
        reasons.append("insufficient_internal_RAM_headroom")
    stream_pass = not reasons
    photos = report["photos"]
    photo_pass = None
    if photos["requested"]:
        complete_run = (not report["interrupted"] and not report["errors"]
                        and report["duration_s"] >= max(300, report.get("requested_seconds", 300))
                        and report["profile_api_available"] and not metrics["reset_detected"]
                        and metrics.get("schema_complete", False) and not metrics.get("unexpected_profile", False))
        photo_pass = (complete_run and photos["completed"] >= 100 and photos["completed"] == photos["requested"]
                      and photos["failed"] == 0 and photos["restored"] == photos["completed"])
        if not photo_pass:
            reasons.append("photo_qualification_requires_100_decoded_restored_photos")
    return {"pass": not reasons, "stream_pass": stream_pass, "photo_pass": photo_pass,
            "reasons": reasons, "firmware_flags_changed": False}


class Receiver:
    def __init__(self):
        self.lock = threading.Lock()
        self.frames = self.identity_frames = self.unique = self.duplicates = self.sequence_gaps = 0
        self.payload_bytes = self.decode_errors = self.malformed_parts = 0
        self.samples_truncated = False
        self.protocol_errors = 0
        self.expected_profile = self.initial_epoch = self.expected_dimensions = self.expected_target = None
        self.unexpected_profile = False
        self.keys, self.sizes, self.intervals = set(), [], []
        self.previous_received = None
        self.last_identity = None
        self.metrics = deque(maxlen=1024)
        self.metrics_first = self.metrics_last = None
        self.metric_samples = self.poll_errors = 0
        self.reset_detected = False
        self.schema_complete = True
        self.min_psram = self.min_internal = None
        self.max_photo_pause_us = 0
        self.errors = []
        self.photos_requested = self.photos_attempted = self.photos_completed = self.photos_failed = self.photos_restored = 0

    def error(self, message):
        with self.lock:
            if len(self.errors) < 100:
                self.errors.append(str(message))

    def record_frame(self, headers, payload, received, expected):
        with self.lock:
            self.frames += 1
            self.payload_bytes += len(payload)
            if len(self.sizes) < MAX_SAMPLES:
                self.sizes.append(len(payload))
                if self.previous_received is not None:
                    self.intervals.append((received - self.previous_received) * 1000)
            else:
                self.samples_truncated = True
            self.previous_received = received
        try:
            decode_jpeg(payload, expected)
        except Exception as error:
            with self.lock:
                self.decode_errors += 1
            self.error("JPEG decode/dimensions: " + str(error))
            return
        try:
            identity = frame_identity(headers)
        except StreamError as error:
            with self.lock:
                self.malformed_parts += 1
            self.error(error)
            return
        if identity is None:
            return
        epoch, sequence, capture_us = identity
        protocol_error = None
        with self.lock:
            self.identity_frames += 1
            key = (epoch, sequence)
            allowed_epoch = (self.initial_epoch is None or epoch == self.initial_epoch or
                (self.photos_attempted > 0 and self.initial_epoch < epoch <= self.initial_epoch + 2 * self.photos_attempted))
            if not allowed_epoch:
                protocol_error = "unexpected profile epoch"
            elif self.last_identity:
                previous_epoch, previous_sequence, previous_capture = self.last_identity
                if epoch < previous_epoch or (epoch == previous_epoch and
                        (sequence <= previous_sequence or capture_us <= previous_capture)):
                    protocol_error = "non-increasing sequence/camera timestamp"
            if protocol_error:
                self.protocol_errors += 1
            if key in self.keys:
                self.duplicates += 1
            elif len(self.keys) < MAX_SAMPLES:
                self.keys.add(key)
                self.unique += 1
                if self.last_identity and self.last_identity[0] == epoch:
                    self.sequence_gaps += max(0, sequence - self.last_identity[1] - 1)
                if protocol_error is None:
                    self.last_identity = identity
            else:
                self.samples_truncated = True
        if protocol_error:
            self.error(protocol_error)

    def record_metrics(self, value):
        with self.lock:
            self.schema_complete = self.schema_complete and all(
                isinstance(value.get(key), int) and not isinstance(value[key], bool) and value[key] >= 0
                for key in REQUIRED_METRICS)
            if self.expected_profile is not None:
                active = value.get("profile")
                permitted_photo = self.photos_attempted > 0 and active == "photo"
                if active != self.expected_profile and not permitted_photo:
                    self.unexpected_profile = True
                if active == self.expected_profile and (value.get("target_fps") != self.expected_target or
                        (value.get("width"), value.get("height")) != self.expected_dimensions):
                    self.unexpected_profile = True
                epoch = value.get("profile_epoch")
                if not isinstance(epoch, int) or isinstance(epoch, bool) or epoch < self.initial_epoch or \
                        epoch > self.initial_epoch + 2 * self.photos_attempted:
                    self.unexpected_profile = True
            if self.metrics_first is None:
                self.metrics_first = value
            if self.metrics_last is not None:
                for key in ("uptime_s", *COUNTERS):
                    if isinstance(value.get(key), (int, float)) and isinstance(self.metrics_last.get(key), (int, float)):
                        if value[key] < self.metrics_last[key]:
                            self.reset_detected = True
            self.metrics_last = value
            self.metric_samples += 1
            self.metrics.append(value)
            pause = value.get("last_photo_pause_us")
            if isinstance(pause, int) and not isinstance(pause, bool) and pause >= 0:
                self.max_photo_pause_us = max(self.max_photo_pause_us, pause)
            for fields, attribute in ((("free_psram", "min_free_psram"), "min_psram"),
                                      (("free_internal", "min_free_internal"), "min_internal")):
                for key in fields:
                    amount = value.get(key)
                    if isinstance(amount, int) and not isinstance(amount, bool) and amount >= 0:
                        old = getattr(self, attribute)
                        setattr(self, attribute, amount if old is None else min(old, amount))

    def report(self, duration):
        with self.lock:
            first, last = self.metrics_first or {}, self.metrics_last or {}
            deltas = {key: last[key] - first[key] for key in COUNTERS
                      if isinstance(first.get(key), int) and not isinstance(first[key], bool)
                      and isinstance(last.get(key), int) and not isinstance(last[key], bool)}
            return {
                "receiver": {"frames": self.frames, "identity_frames": self.identity_frames,
                    "unique_frames": self.unique if self.identity_frames == self.frames and not self.samples_truncated else None,
                    "wire_fps": self.frames / duration if duration else 0,
                    "unique_fps": self.unique / duration if duration and self.identity_frames == self.frames and not self.samples_truncated else None,
                    "duplicates": self.duplicates, "latest_frame_sequence_gaps": self.sequence_gaps,
                    "protocol_errors": self.protocol_errors,
                    "jpeg_payload_bytes": self.payload_bytes,
                    "jpeg_payload_mbit_s": self.payload_bytes * 8 / duration / 1e6 if duration else 0,
                    "jpeg_size_p50": percentile(self.sizes, .50), "jpeg_size_p95": percentile(self.sizes, .95),
                    "interval_p50_ms": percentile(self.intervals, .50), "interval_p95_ms": percentile(self.intervals, .95),
                    "decode_errors": self.decode_errors, "malformed_parts": self.malformed_parts,
                    "samples_truncated": self.samples_truncated},
                "metrics": {"samples": self.metric_samples, "poll_errors": self.poll_errors,
                    "reset_detected": self.reset_detected, "counter_deltas": deltas,
                    "schema_complete": self.schema_complete,
                    "unexpected_profile": self.unexpected_profile,
                    "min_free_psram": self.min_psram, "min_free_internal": self.min_internal,
                    "max_reported_photo_pause_us": self.max_photo_pause_us,
                    "first": first, "last": last, "snapshots": list(self.metrics),
                    "snapshots_retained": len(self.metrics)},
                "photos": {"requested": self.photos_requested, "attempted": self.photos_attempted,
                    "completed": self.photos_completed, "failed": self.photos_failed, "restored": self.photos_restored},
                "errors": list(self.errors)}


def run(args):
    import PIL
    parsed = urllib.parse.urlsplit(args.url)
    if parsed.scheme not in ("http", "https") or not parsed.hostname or parsed.username or parsed.password:
        raise ValueError("--url must be an HTTP(S) camera URL without embedded credentials")
    receiver, stop = Receiver(), threading.Event()
    opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
    base = args.url.rstrip("/")
    status_available = False
    initial_status = {}
    selected_profile = None
    expected_dimensions = PROFILES[args.profile][:2]
    stream_connection = None
    start = end = None
    interrupted = False
    threads = []
    receiver.photos_requested = args.photos

    def request(path, data=None, timeout=3):
        headers = {"Cache-Control": "no-cache"}
        if data is not None:
            data = json.dumps(data).encode()
            headers["Content-Type"] = "application/json"
        return opener.open(urllib.request.Request(base + path, data=data, headers=headers), timeout=timeout)

    def json_request(path, data=None, timeout=3):
        with request(path, data, timeout=timeout) as response:
            payload = response.read(65537)
            if len(payload) > 65536:
                raise ValueError("JSON response exceeds bound")
            value = json.loads(payload)
            if not isinstance(value, dict):
                raise ValueError("expected JSON object")
            return value

    def wait_restored(deadline):
        while not stop.is_set() and time.monotonic() < deadline:
            status = json_request("/api/profile", timeout=max(.01, min(3, deadline - time.monotonic())))
            if status.get("state") == "error" or status.get("restore_error", 0):
                raise ValueError("camera profile restoration failed")
            if (status.get("state") == "running" and status.get("active") == args.profile
                    and time.monotonic() < deadline):
                return status
            stop.wait(.1)
        raise TimeoutError("profile did not reach requested running state")

    def stream():
        nonlocal stream_connection
        try:
            connection_class = http.client.HTTPSConnection if parsed.scheme == "https" else http.client.HTTPConnection
            # Photo mode can legitimately pause publication while the sensor
            # reinitializes; stop still interrupts this bounded socket read.
            stream_connection = connection_class(parsed.hostname, parsed.port, timeout=20)
            stream_connection.request("GET", (parsed.path.rstrip("/") + "/stream") or "/stream",
                                      headers={"Cache-Control": "no-cache"})
            response = stream_connection.getresponse()
            if response.status != 200:
                raise StreamError(f"stream HTTP {response.status}")
            parser = MultipartReader(response, boundary_from(response.getheader("Content-Type", "")))
            for headers, payload in parser:
                if stop.is_set():
                    break
                received = time.monotonic()
                receiver.record_frame(headers, payload, received, expected_dimensions)
                if stop.wait(args.read_delay_ms / 1000):
                    break
            if not stop.is_set():
                raise StreamError("stream ended before test deadline")
        except Exception as error:
            if not stop.is_set():
                with receiver.lock:
                    receiver.malformed_parts += isinstance(error, StreamError)
                receiver.error("stream: " + str(error))
                stop.set()
        finally:
            if stream_connection is not None:
                stream_connection.close()

    def metrics():
        while not stop.is_set():
            try:
                receiver.record_metrics(json_request("/metrics"))
            except Exception as error:
                with receiver.lock:
                    receiver.poll_errors += 1
                receiver.error("metrics: " + str(error))
            stop.wait(1)

    def photos():
        # Space requests across the requested test period, never overlap photos.
        interval = args.seconds / args.photos if args.photos else 0
        for index in range(args.photos):
            if stop.wait(max(0, start + index * interval - time.monotonic())):
                return
            with receiver.lock:
                receiver.photos_attempted += 1
            try:
                with request("/photo", timeout=20) as response:
                    payload = response.read(MAX_FRAME + 1)
                    if len(payload) > MAX_FRAME:
                        raise StreamError("photo exceeds JPEG bound")
                    decode_jpeg(payload, (2048, 1536))
                with receiver.lock:
                    receiver.photos_completed += 1
                wait_restored(time.monotonic() + 10)
                with receiver.lock:
                    receiver.photos_restored += 1
            except Exception as error:
                with receiver.lock:
                    receiver.photos_failed += 1
                receiver.error("photo: " + str(error))

    try:
        try:
            json_request("/api/profile", {"profile": args.profile})
            initial_status = wait_restored(time.monotonic() + 10)
            selected_profile = profile_metadata(initial_status, args.profile)
            epoch = initial_status.get("epoch")
            if not isinstance(epoch, int) or isinstance(epoch, bool) or not 0 <= epoch <= 2**32 - 1:
                raise ValueError("missing/invalid initial firmware epoch")
            expected_dimensions = (selected_profile["width"], selected_profile["height"])
            receiver.expected_profile, receiver.initial_epoch = args.profile, epoch
            receiver.expected_dimensions = expected_dimensions
            receiver.expected_target = selected_profile["target_fps"]
            status_available = True
        except urllib.error.HTTPError as error:
            if error.code != 404:
                raise
            receiver.error("Profile API absent; legacy wire-rate diagnostic only")
        receiver.record_metrics(json_request("/metrics"))
        start = time.monotonic()
        for target in (stream, metrics, photos):
            thread = threading.Thread(target=target, daemon=True)
            threads.append(thread)
            thread.start()
        stop.wait(args.seconds)
    except KeyboardInterrupt:
        interrupted = True
        receiver.error("interrupted by user")
    except Exception as error:
        receiver.error("setup: " + str(error))
    finally:
        end = time.monotonic()
        stop.set()
        # Interrupt a blocked stream read; the worker alone closes its response.
        if stream_connection is not None and stream_connection.sock is not None:
            try:
                stream_connection.sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
        join_deadline = time.monotonic() + 24
        for thread in threads:
            thread.join(max(0, join_deadline - time.monotonic()))
            if thread.is_alive():
                receiver.error("worker did not stop within bounded timeout")
        if start is not None:
            try:
                receiver.record_metrics(json_request("/metrics"))
            except Exception as error:
                with receiver.lock:
                    receiver.poll_errors += 1
                receiver.error("final metrics: " + str(error))
    duration = max(0, end - start) if start is not None else 0
    report = {"schema_version": 1, "url": base, "profile": args.profile,
              "profile_api_available": status_available, "initial_status": initial_status,
              "scene": args.scene, "requested_seconds": args.seconds, "duration_s": duration,
              "firmware_profile": selected_profile,
              "target_fps": selected_profile["target_fps"] if selected_profile else PROFILES[args.profile][2],
              "target_source": "firmware_profile_metadata" if selected_profile else "unverified_legacy_default",
              "read_delay_ms": args.read_delay_ms,
              "interrupted": interrupted,
              "client": {"python": sys.version, "platform": platform.platform(), "pillow": PIL.__version__},
              "measurement": "Host-received fully decoded JPEG cadence; not camera-to-screen latency.",
              **receiver.report(duration)}
    report["qualification"] = qualification(report)
    return report


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", default="http://192.168.4.1")
    parser.add_argument("--profile", choices=PROFILES, default="safe")
    parser.add_argument("--seconds", type=float, default=300)
    parser.add_argument("--photos", type=int, default=0)
    parser.add_argument("--scene", choices=SCENES, default="unspecified")
    parser.add_argument("--read-delay-ms", type=float, default=0)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if not math.isfinite(args.seconds) or args.seconds <= 0 or args.photos < 0 or not math.isfinite(args.read_delay_ms) or args.read_delay_ms < 0:
        parser.error("seconds must be positive; photos/read-delay must be nonnegative")
    try:
        report = run(args)
    except ImportError:
        parser.error("Pillow is required for full JPEG decoding; install it before benchmarking")
    except ValueError as error:
        parser.error(str(error))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report["qualification"], indent=2))
    return 0 if report["qualification"]["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
