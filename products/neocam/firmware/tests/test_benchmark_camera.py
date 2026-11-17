#!/usr/bin/env python3
"""No-server tests for framing, strict JPEG validation and qualification gates."""
from copy import deepcopy
from io import BytesIO
import json
from pathlib import Path
import subprocess
import sys
import unittest
from unittest import mock
from argparse import Namespace
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import threading
import time

import benchmark_camera as benchmark


class Fragmented:
    def __init__(self, data, fragment=1):
        self.data, self.fragment = data, fragment

    def read(self, maximum):
        size = min(self.fragment, maximum)
        result, self.data = self.data[:size], self.data[size:]
        return result


def part(payload, headers=None, boundary=b"frame"):
    fields = {"Content-Type": "image/jpeg", "Content-Length": str(len(payload)), **(headers or {})}
    return b"\r\n--" + boundary + b"\r\n" + b"".join(
        key.encode() + b": " + value.encode() + b"\r\n" for key, value in fields.items()
    ) + b"\r\n" + payload


def passing_report():
    return {
        "duration_s": 300, "scene": "good-light-detail", "read_delay_ms": 0,
        "interrupted": False, "errors": [], "profile_api_available": True, "target_fps": 30,
        "receiver": {"frames": 9000, "identity_frames": 9000, "samples_truncated": False,
                     "decode_errors": 0, "malformed_parts": 0, "unique_fps": 30, "interval_p95_ms": 34},
        "metrics": {"samples": 301, "poll_errors": 0, "reset_detected": False, "schema_complete": True,
                    "counter_deltas": {"capture_failures": 0, "oversized_frames": 0},
                    "min_free_psram": 1024 * 1024, "min_free_internal": 100 * 1024},
        "photos": {"requested": 0, "completed": 0, "failed": 0, "restored": 0},
    }


class BenchmarkTests(unittest.TestCase):
    def test_fragmented_multipart_and_metadata(self):
        headers = {"X-Sequence": "123", "X-Capture-Time-Us": "456", "X-Profile-Epoch": "7"}
        data = part(b"JPEG\r\n--fakeboundary", headers) + part(b"NEXT") + b"\r\n--frame--\r\n"
        for fragment in (1, 2, 7, 4096):
            frames = list(benchmark.MultipartReader(Fragmented(data, fragment), b"frame"))
            self.assertEqual([payload for _, payload in frames], [b"JPEG\r\n--fakeboundary", b"NEXT"])
            self.assertEqual(benchmark.frame_identity(frames[0][0]), (7, 123, 456))
        self.assertEqual(benchmark.boundary_from('multipart/x-mixed-replace; boundary="frame"'), b"frame")

    def test_truncation_invalid_lengths_and_bounds(self):
        for data in (
            part(b"abcdef")[:-2],
            part(b"x", {"Content-Length": "-1"}),
            part(b"x", {"Content-Length": "1.5"}),
            part(b"x", {"Content-Length": str(benchmark.MAX_FRAME + 1)}),
            part(b"x", {"Content-Length": "0"}),
            b"--frame\r\nContent-Type: image/jpeg\r\n\r\nx",
            b"--frame\r\nContent-Length: 1\r\nContent-Length: 1\r\n\r\nx",
            b"--wrong\r\n",
        ):
            with self.assertRaises(benchmark.StreamError):
                list(benchmark.MultipartReader(Fragmented(data, 2), b"frame"))
        with self.assertRaises(benchmark.StreamError):
            benchmark.boundary_from("image/jpeg")

    def test_full_jpeg_decode_not_only_markers(self):
        from PIL import Image
        output = BytesIO()
        Image.new("RGB", (16, 8), (30, 120, 180)).save(output, "JPEG")
        valid = output.getvalue()
        benchmark.decode_jpeg(valid, (16, 8))
        with self.assertRaises(Exception):
            benchmark.decode_jpeg(b"\xff\xd8this is not a JPEG\xff\xd9", (16, 8))
        with self.assertRaises(Exception):
            benchmark.decode_jpeg(valid[:len(valid) // 2] + b"\xff\xd9", (16, 8))
        with self.assertRaises(benchmark.StreamError):
            benchmark.decode_jpeg(valid, (640, 480))

    def test_qualification_requires_complete_measured_evidence(self):
        self.assertTrue(benchmark.qualification(passing_report())["pass"])
        cases = [
            ("duration_s", 299.99), ("scene", "unspecified"), ("scene", "low-light"),
            ("read_delay_ms", 1), ("interrupted", True), ("profile_api_available", False),
        ]
        for key, value in cases:
            report = passing_report()
            report[key] = value
            self.assertFalse(benchmark.qualification(report)["pass"], key)
        for field, value in (("identity_frames", 8999), ("unique_fps", None), ("samples_truncated", True),
                             ("decode_errors", 1), ("malformed_parts", 1), ("unique_fps", 28.4),
                             ("interval_p95_ms", 70)):
            report = passing_report()
            report["receiver"][field] = value
            self.assertFalse(benchmark.qualification(report)["pass"], field)
        for field, value in (("poll_errors", 1), ("reset_detected", True), ("schema_complete", False), ("min_free_psram", None),
                             ("min_free_psram", 511 * 1024), ("min_free_internal", 63 * 1024)):
            report = passing_report()
            report["metrics"][field] = value
            self.assertFalse(benchmark.qualification(report)["pass"], field)
        report = passing_report()
        report["metrics"]["counter_deltas"]["capture_failures"] = 1
        self.assertFalse(benchmark.qualification(report)["pass"])

    def test_photo_pass_requires_decoding_and_restoration(self):
        report = passing_report()
        report["photos"] = {"requested": 100, "completed": 100, "failed": 0, "restored": 100}
        self.assertTrue(benchmark.qualification(report)["photo_pass"])
        for field, value in (("completed", 99), ("restored", 99), ("failed", 1), ("requested", 101)):
            changed = deepcopy(report)
            changed["photos"][field] = value
            self.assertFalse(benchmark.qualification(changed)["photo_pass"])
        for field, value in (("interrupted", True), ("duration_s", 299), ("errors", ["stream EOF"])):
            changed = deepcopy(report)
            changed[field] = value
            self.assertFalse(benchmark.qualification(changed)["photo_pass"])

    def test_metrics_reset_and_bounded_receiver(self):
        receiver = benchmark.Receiver()
        initial = {"uptime_s": 10, "capture_frames": 20, "capture_failures": 2,
                   "oversized_frames": 0, "min_free_psram": 1000000, "min_free_internal": 100000}
        receiver.record_metrics(initial)
        receiver.record_metrics({**initial, "uptime_s": 11, "capture_frames": 25})
        report = receiver.report(1)
        self.assertEqual(report["metrics"]["counter_deltas"]["capture_failures"], 0)
        self.assertEqual(report["metrics"]["counter_deltas"]["capture_frames"], 5)
        receiver.record_metrics({**initial, "uptime_s": 1, "capture_frames": 1})
        self.assertTrue(receiver.report(1)["metrics"]["reset_detected"])
        self.assertIsNone(benchmark.frame_identity({"content-length": "2"}))
        with self.assertRaises(benchmark.StreamError):
            benchmark.frame_identity({"x-sequence": "-1", "x-profile-epoch": "2", "x-capture-time-us": "3"})

    def test_firmware_target_and_strict_frame_order(self):
        profile = {"id": "smooth", "width": 640, "height": 480, "jpeg_quality": 16, "target_fps": 60}
        self.assertEqual(benchmark.profile_metadata({"profiles": [profile]}, "smooth")["target_fps"], 60)
        with self.assertRaises(ValueError):
            benchmark.profile_metadata({"profiles": [{**profile, "jpeg_quality": 64}]}, "smooth")
        report = passing_report()
        report["target_fps"] = 60
        self.assertFalse(benchmark.qualification(report)["pass"])
        report["receiver"].update(unique_fps=60, interval_p95_ms=17)
        self.assertTrue(benchmark.qualification(report)["pass"])
        from PIL import Image
        output = BytesIO()
        Image.new("RGB", (16, 8)).save(output, "JPEG")
        for violation in ((1, 10, 100), (1, 9, 101), (1, 11, 100), (2, 11, 101)):
            receiver = benchmark.Receiver()
            receiver.initial_epoch = 1
            for index, (epoch, sequence, stamp) in enumerate(((1, 10, 100), violation)):
                receiver.record_frame({"x-profile-epoch": str(epoch), "x-sequence": str(sequence),
                    "x-capture-time-us": str(stamp)}, output.getvalue(), index, (16, 8))
            self.assertEqual(receiver.report(1)["receiver"]["protocol_errors"], 1, violation)
        receiver = benchmark.Receiver()
        receiver.initial_epoch = 1
        receiver.photos_attempted = 1
        receiver.record_frame({"x-profile-epoch": "3", "x-sequence": "10", "x-capture-time-us": "100"},
                              output.getvalue(), 1, (16, 8))
        self.assertEqual(receiver.report(1)["receiver"]["protocol_errors"], 0)

    def test_missing_identity_is_wire_rate_only_and_samples_are_bounded(self):
        from PIL import Image
        output = BytesIO()
        Image.new("RGB", (16, 8)).save(output, "JPEG")
        payload = output.getvalue()
        receiver = benchmark.Receiver()
        receiver.record_frame({}, payload, 1, (16, 8))
        self.assertEqual(receiver.report(1)["receiver"]["wire_fps"], 1)
        self.assertIsNone(receiver.report(1)["receiver"]["unique_fps"])
        with mock.patch.object(benchmark, "MAX_SAMPLES", 2):
            receiver = benchmark.Receiver()
            for index in range(4):
                headers = {"x-sequence": str(index), "x-profile-epoch": "1", "x-capture-time-us": str(index)}
                receiver.record_frame(headers, payload, index, (16, 8))
            self.assertLessEqual(len(receiver.keys), 2)
            self.assertLessEqual(len(receiver.sizes), 2)
            self.assertTrue(receiver.report(4)["receiver"]["samples_truncated"])

    def test_help_requires_no_network(self):
        script = Path(__file__).with_name("benchmark_camera.py")
        result = subprocess.run([sys.executable, str(script), "--help"], capture_output=True, text=True, timeout=5)
        self.assertEqual(result.returncode, 0)
        self.assertIn("--scene", result.stdout)

    def test_loopback_http_profile_stream_photo_and_restore(self):
        """Exercise actual chunked HTTP reads and concurrent control locally."""
        from PIL import Image

        def jpeg(size):
            output = BytesIO()
            Image.new("RGB", size, (25, 120, 180)).save(output, "JPEG")
            return output.getvalue()

        preview, photo = jpeg((640, 480)), jpeg((2048, 1536))
        shutdown = threading.Event()
        values = {"profile": "safe", "frames": 0, "photos": 0, "restore_pending": False}
        started = time.monotonic()

        class Handler(BaseHTTPRequestHandler):
            protocol_version = "HTTP/1.1"

            def log_message(self, *args):
                pass

            def send(self, payload, content_type="application/json", status=200):
                self.send_response(status)
                self.send_header("Content-Type", content_type)
                self.send_header("Content-Length", str(len(payload)))
                self.send_header("Connection", "close")
                self.end_headers()
                self.wfile.write(payload)

            def do_POST(self):
                body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
                self.assert_path()
                values["profile"] = body["profile"]
                self.send(b'{"accepted":true}', status=202)

            def assert_path(self):
                if self.path != "/api/profile":
                    raise AssertionError(self.path)

            def do_GET(self):
                if self.path == "/api/profile":
                    pending = values["restore_pending"]
                    values["restore_pending"] = False
                    self.send(json.dumps({"state": "photo" if pending else "running", "epoch": 1,
                        "active": "photo" if pending else values["profile"], "restore_error": 0,
                        "profiles": [{"id": "safe", "width": 640, "height": 480,
                                      "jpeg_quality": 12, "target_fps": 30}]}).encode())
                elif self.path == "/metrics":
                    self.send(json.dumps({"uptime_s": 100 + int(time.monotonic() - started),
                        "capture_frames": values["frames"], "stream_frames": values["frames"],
                        "capture_failures": 0, "oversized_frames": 0, "photos": values["photos"],
                        "profile": values["profile"], "profile_epoch": 1, "width": 640, "height": 480, "target_fps": 30,
                        "min_free_psram": 1000000, "min_free_internal": 100000,
                        "last_photo_pause_us": 123456 if values["photos"] else 0}).encode())
                elif self.path == "/photo":
                    values["photos"] += 1
                    values["restore_pending"] = True
                    self.send(photo, "image/jpeg")
                elif self.path == "/stream":
                    self.send_response(200)
                    self.send_header("Content-Type", "multipart/x-mixed-replace;boundary=frame")
                    self.send_header("Transfer-Encoding", "chunked")
                    self.end_headers()
                    try:
                        while not shutdown.is_set():
                            values["frames"] += 1
                            packet = part(preview, {"X-Sequence": str(values["frames"]),
                                "X-Profile-Epoch": "1", "X-Capture-Time-Us": str(values["frames"] * 33333)})
                            for offset in range(0, len(packet), 517):
                                fragment = packet[offset:offset + 517]
                                self.wfile.write(f"{len(fragment):x}\r\n".encode() + fragment + b"\r\n")
                            self.wfile.flush()
                            shutdown.wait(.02)
                    except (BrokenPipeError, ConnectionResetError):
                        pass
                else:
                    self.send(b"missing", status=404)

        server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
        server.daemon_threads = True
        worker = threading.Thread(target=server.serve_forever, kwargs={"poll_interval": .02})
        worker.start()
        try:
            args = Namespace(url=f"http://127.0.0.1:{server.server_port}", profile="safe", seconds=.4,
                             photos=1, scene="plain", read_delay_ms=0)
            report = benchmark.run(args)
        finally:
            shutdown.set()
            server.shutdown()
            server.server_close()
            worker.join(2)
        self.assertTrue(report["profile_api_available"])
        self.assertEqual(report["target_source"], "firmware_profile_metadata")
        self.assertEqual(report["firmware_profile"]["jpeg_quality"], 12)
        self.assertGreater(report["receiver"]["unique_frames"], 0)
        self.assertEqual(report["receiver"]["decode_errors"], 0)
        self.assertEqual(report["errors"], [])
        self.assertEqual(report["photos"]["completed"], 1)
        self.assertEqual(report["photos"]["restored"], 1)
        self.assertEqual(report["metrics"]["max_reported_photo_pause_us"], 123456)
        self.assertFalse(report["qualification"]["pass"])
        self.assertFalse(report["qualification"]["photo_pass"])


if __name__ == "__main__":
    unittest.main()
