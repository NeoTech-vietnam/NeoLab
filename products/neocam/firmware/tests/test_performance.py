#!/usr/bin/env python3
"""No-build tests of performance candidate selection and provenance semantics."""
from contextlib import redirect_stderr, redirect_stdout
from importlib.util import module_from_spec, spec_from_file_location
from io import StringIO
import json
import hashlib
import os
from pathlib import Path
import subprocess
import sys
from tempfile import TemporaryDirectory
import unittest
from unittest import mock


SCRIPT = Path(__file__).resolve().parents[1] / "STConf/TOOLCONF/performance.py"
SPEC = spec_from_file_location("neocam_performance", SCRIPT)
performance = module_from_spec(SPEC)
SPEC.loader.exec_module(performance)


class PerformanceTests(unittest.TestCase):
    def setUp(self):
        self.directory = TemporaryDirectory(prefix="neocam-performance-test-")
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.sdk = self.root / "sdk"
        (self.sdk / "tools/cmake").mkdir(parents=True)
        (self.sdk / "tools/cmake/version.cmake").write_text(
            "set(IDF_VERSION_MAJOR 6)\nset(IDF_VERSION_MINOR 0)\nset(IDF_VERSION_PATCH 1)\n"
        )
        (self.root / "sdkconfig.defaults").write_text('CONFIG_IDF_TARGET="esp32s3"\n')
        self.stdout, self.stderr = StringIO(), StringIO()

    def run_builder(self, arguments, *, environment=None, side_effect=None):
        env = {"IDF_PATH": str(self.sdk)} if environment is None else environment
        self.stdout, self.stderr = StringIO(), StringIO()
        def run(command, **kwargs):
            if command[0] == "git":
                return subprocess.CompletedProcess(command, 1, "", "not a git repository")
            if side_effect is not None:
                if callable(side_effect):
                    return side_effect(command, **kwargs)
                raise side_effect
            return subprocess.CompletedProcess(command, 0, "ESP-IDF v6.0.1\n", "")

        with mock.patch.object(performance, "ROOT", self.root), \
             mock.patch.object(sys, "argv", [str(SCRIPT), *arguments]), \
             mock.patch.dict(os.environ, env, clear=True), \
             mock.patch.object(performance.subprocess, "run", side_effect=run) as run, \
             redirect_stdout(self.stdout), redirect_stderr(self.stderr):
            self.run_mock = run
            performance.main()
        return json.JSONDecoder().raw_decode(self.stdout.getvalue().lstrip())[0]

    def fixture_build(self, variant="baseline", suffix="", *, overrides=None):
        selected = [] if variant == "baseline" else list(performance.ORDER[:performance.ORDER.index(variant) + 1])
        build = self.root / f"build-perf-{variant}{suffix}"
        (build / "config").mkdir(parents=True, exist_ok=True)
        config = {
            "ESP_DEFAULT_CPU_FREQ_MHZ": 240 if "cpu240" in selected else 160,
            "SPIRAM_SPEED": 80 if "psram80" in selected else 40,
            "CAMERA_PSRAM_DMA": "dma" in selected,
            "COMPILER_OPTIMIZATION_PERF": "perf" in selected,
            "COMPILER_OPTIMIZATION_ASSERTIONS_ENABLE": True,
            "CAMERA_JPEG_MODE_FRAME_SIZE": 1048576,
            "CAMERA_CORE1": "cores" in selected,
            "LWIP_TCP_SND_BUF_DEFAULT": 16384 if "network" in selected else 5744,
            "ESP_WIFI_DYNAMIC_TX_BUFFER_NUM": 48 if "network" in selected else 32,
            "ESPTOOLPY_FLASHSIZE": "8MB",
            "ESPTOOLPY_FLASHMODE": "dio",
            "ESPTOOLPY_OCT_FLASH": False,
            "ESPTOOLPY_FLASHFREQ": "80m",
            "SPIRAM": True,
            "SPIRAM_MODE_OCT": True,
            "CAMERA_JPEG_MODE_FRAME_SIZE_CUSTOM": True,
        }
        config.update(overrides or {})
        (build / "config/sdkconfig.json").write_text(json.dumps(config))
        (build / "sdkconfig").write_text("# generated candidate fixture\n")
        (build / "firmware.bin").write_bytes(b"test firmware binary")
        (build / "project_description.json").write_text(json.dumps({
            "project_name": "firmware", "app_bin": "firmware.bin", "target": "esp32s3",
            "idf_path": str(self.sdk), "git_revision": "v6.0.1",
            "build_component_info": {name: {} for name in ("BSW", "Rte", "ApplicationLayer")},
        }))
        return build

    @staticmethod
    def definitions(command):
        return dict(command[index + 1].split("=", 1) for index, value in enumerate(command[:-1]) if value == "-D")

    def test_invalid_arguments_never_run_build_or_flash(self):
        cases = (
            [], ["unknown"], ["baseline", "--quality", "-1"], ["baseline", "--quality", "64"],
            ["baseline", "--smooth-fps", "45"], ["baseline", "--channel", "0"],
            ["baseline", "--channel", "12"], ["baseline", "--bandwidth", "80"],
            ["baseline", "--configure-only", "--port", "/dev/fake"],
        )
        for arguments in cases:
            with self.subTest(arguments=arguments), self.assertRaises(SystemExit) as exit:
                self.run_builder(arguments)
            self.assertEqual(exit.exception.code, 2)
            self.run_mock.assert_not_called()
        with self.assertRaises(SystemExit):
            self.run_builder(["baseline", "--dry-run"], environment={})
        self.run_mock.assert_not_called()

    def test_dry_run_is_isolated_and_overlays_are_cumulative(self):
        expected = (
            ("baseline", []),
            ("cpu240", ["cpu240"]),
            ("psram80", ["cpu240", "psram80"]),
            ("perf", ["cpu240", "psram80", "perf"]),
            ("dma", ["cpu240", "psram80", "perf", "dma"]),
            ("cores", ["cpu240", "psram80", "perf", "dma", "cores"]),
            ("network", ["cpu240", "psram80", "perf", "dma", "cores", "network"]),
        )
        before = set(self.root.rglob("*"))
        for variant, selected in expected:
            with self.subTest(variant=variant):
                report = self.run_builder([variant, "--dry-run"])
                self.run_mock.assert_not_called()
                command = report["command"]
                self.assertTrue(report["experimental"])
                self.assertEqual(command[-1], "build")
                build = self.root / f"build-perf-{variant}"
                self.assertEqual(command[command.index("-B") + 1], str(build))
                definitions = self.definitions(command)
                self.assertEqual(definitions["SDKCONFIG"], str(build / "sdkconfig"))
                defaults = definitions["SDKCONFIG_DEFAULTS"].split(";")
                self.assertEqual(defaults, [str(self.root / "sdkconfig.defaults"), *[
                    str(self.root / "STConf/TOOLCONF/variants" / f"{name}.defaults") for name in selected
                ]])
                self.assertEqual(definitions["NEOCAM_PRODUCER_CORE"], "1" if "cores" in selected else "-1")
                self.assertEqual(definitions["NEOCAM_WIFI_CHANNEL"], "1")
                self.assertEqual(definitions["NEOCAM_WIFI_BANDWIDTH"], "20")
        self.assertEqual(set(self.root.rglob("*")), before)

    def test_explicit_knobs_get_distinct_candidate_and_exact_c_definitions(self):
        report = self.run_builder([
            "network", "--quality", "0", "--smooth-fps", "60", "--channel", "11",
            "--bandwidth", "40", "--dry-run",
        ])
        command = report["command"]
        build = Path(command[command.index("-B") + 1])
        self.assertEqual(build.name, "build-perf-network-q0-fps60-ch11-ht40")
        definitions = self.definitions(command)
        self.assertEqual(definitions["NEOCAM_BENCH_JPEG_QUALITY"], "0")
        self.assertEqual(definitions["NEOCAM_BENCH_SMOOTH_FPS"], "60")
        self.assertEqual(definitions["NEOCAM_WIFI_CHANNEL"], "11")
        self.assertEqual(definitions["NEOCAM_WIFI_BANDWIDTH"], "40")
        self.run_mock.assert_not_called()

    def test_configure_only_never_claims_binary_or_hardware_validation(self):
        build = self.fixture_build()
        self.run_builder(["baseline", "--configure-only"])
        command = next(call.args[0] for call in self.run_mock.call_args_list if call.args[0][0] != "git")
        self.assertEqual(command[-1], "reconfigure")
        manifest = json.loads((build / "variant.json").read_text())
        self.assertFalse(manifest["build_completed"])
        self.assertFalse(manifest["performance_validated"])
        self.assertTrue(manifest["experimental"])
        self.assertEqual(manifest["overlays"], [])

    def test_success_records_requested_knobs_without_qualifying_performance(self):
        build = self.fixture_build("network", "-q63-fps60-ch6-ht40")
        self.run_builder([
            "network", "--quality", "63", "--smooth-fps", "60", "--channel", "6", "--bandwidth", "40",
        ])
        manifest = json.loads((build / "variant.json").read_text())
        self.assertTrue(manifest["build_completed"])
        self.assertFalse(manifest["performance_validated"])
        self.assertEqual(manifest["quality_override"], 63)
        self.assertEqual(manifest["smooth_fps"], 60)
        self.assertEqual(manifest["channel"], 6)
        self.assertEqual(manifest["configured_bandwidth_mhz"], 40)
        self.assertEqual(manifest["sdkconfig"], str(build / "sdkconfig"))
        self.assertEqual(manifest["sdk"]["version"], "6.0.1")
        self.assertEqual(manifest["sdk"]["verification"], "version_file")
        self.assertEqual(manifest["sdkconfig_sha256"], hashlib.sha256((build / "sdkconfig").read_bytes()).hexdigest())
        self.assertEqual(manifest["binary"]["sha256"], hashlib.sha256((build / "firmware.bin").read_bytes()).hexdigest())
        self.assertEqual(len(manifest["source"]["sha256"]), 64)
        self.assertGreater(manifest["source"]["files"], 0)

    def test_configuration_mismatch_does_not_create_success_manifest(self):
        build = self.fixture_build("psram80", overrides={"SPIRAM_SPEED": 40})
        with self.assertRaises((AssertionError, RuntimeError, ValueError)):
            self.run_builder(["psram80"])
        manifest = build / "variant.json"
        if manifest.exists():
            self.assertFalse(json.loads(manifest.read_text())["build_completed"])

    def test_failed_attempt_invalidates_old_success_before_running_build(self):
        build = self.fixture_build()
        path = build / "variant.json"
        path.write_text(json.dumps({"build_completed": True, "state": "built", "binary": {"sha256": "old"}}))

        def failed_build(command, **kwargs):
            pending = json.loads(path.read_text())
            self.assertEqual(pending["state"], "pending")
            self.assertFalse(pending["build_completed"])
            self.assertNotIn("binary", pending)
            raise subprocess.CalledProcessError(1, command)

        with self.assertRaises(subprocess.CalledProcessError):
            self.run_builder(["baseline"], side_effect=failed_build)
        manifest = json.loads(path.read_text())
        self.assertEqual(manifest["state"], "failed")
        self.assertFalse(manifest["build_completed"])
        self.assertFalse(manifest["performance_validated"])
        self.assertFalse((build / "variant.json.tmp").exists())

    def test_wrong_sdk_version_cannot_build_or_keep_old_success(self):
        build = self.fixture_build()
        manifest = build / "variant.json"
        manifest.write_text('{"build_completed":true}')
        (self.sdk / "tools/cmake/version.cmake").write_text(
            "set(IDF_VERSION_MAJOR 5)\nset(IDF_VERSION_MINOR 5)\nset(IDF_VERSION_PATCH 6)\n"
        )
        with self.assertRaisesRegex(RuntimeError, "6.0.1 required"):
            self.run_builder(["baseline"])
        self.assertFalse(json.loads(manifest.read_text())["build_completed"])
        self.run_mock.assert_not_called()

    def test_source_changes_during_build_do_not_claim_success(self):
        build = self.fixture_build()

        def change_source(command, **kwargs):
            (self.root / "sdkconfig.defaults").write_text("# changed during build\n")
            return subprocess.CompletedProcess(command, 0)

        with self.assertRaisesRegex(RuntimeError, "Source changed"):
            self.run_builder(["baseline"], side_effect=change_source)
        self.assertFalse(json.loads((build / "variant.json").read_text())["build_completed"])

    def test_validation_remains_active_under_python_optimization(self):
        result = subprocess.run([
            sys.executable, "-O", "-c",
            "import runpy; m=runpy.run_path(" + repr(str(SCRIPT)) + "); m['validate_config']({}, [])",
        ], capture_output=True, text=True, timeout=5)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Candidate configuration mismatch", result.stderr)

    def test_check_build_rejects_wrong_configuration_under_optimization(self):
        build = self.fixture_build()
        (build / "bootloader/config").mkdir(parents=True)
        (build / "bootloader/config/sdkconfig.json").write_text((build / "config/sdkconfig.json").read_text())
        (build / "firmware.map").write_text(
            ".rodata.neocam 0x3c000100 0x20 libmain.a(NeoCam_Composition.c.obj)\n"
            ".rodata.neocam 0x3c000200 0x20 libBSW.a(HttpSrv_Cfg.c.obj)\n"
            ".bss.slots 0x3fc80000 0x60 libBSW.a(MediaBuf.c.obj)\n"
        )
        checker = SCRIPT.with_name("check_build.py")
        result = subprocess.run([sys.executable, "-O", str(checker), str(build)], capture_output=True, text=True, timeout=5)
        self.assertEqual(result.returncode, 0, result.stderr)
        config = json.loads((build / "config/sdkconfig.json").read_text())
        config["ESP_DEFAULT_CPU_FREQ_MHZ"] = 80
        (build / "config/sdkconfig.json").write_text(json.dumps(config))
        result = subprocess.run([sys.executable, "-O", str(checker), str(build)], capture_output=True, text=True, timeout=5)
        self.assertNotEqual(result.returncode, 0)
        self.assertNotIn("PASS", result.stdout)
        self.assertIn("Unsupported CPU clock", result.stderr)


if __name__ == "__main__":
    unittest.main()
