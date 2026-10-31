# DC Power Dock — draft product requirements

**Scope update, 2026-10-30:** The [accepted product concept](../approved-concept.md) takes precedence over this earlier draft. Wireless clock charging, reuse of the flat dock for a compatible phone, and side USB-C output are now part of the product concept, one receiver at a time. Earlier exclusions and pogo-specific requirements below need revision; numerical targets remain draft.

**Power target update, 2026-10-31:** The user selected a **25 W maximum wireless output target** for compatible receivers. The reference point, standard/profile and conditions remain open; see [DOCK-F01](product-features.md). The earlier 5 V / 2 A / 10 W DC bench target below applies to a different draft architecture and is not the wireless output rating. This PRD still needs reconciliation.

Status: draft v0.1, 2026-10-30. Product owner and numerical release targets are not yet assigned. [Development plan](README.md) · [Verification plan](verification-plan.md).

## 1. Product intent and users

Provide a stable, repairable DC power dock for the first Neo Modular Desk Clock. The initial user runs a basic core and display on a desk, then carries the assembled clock to another place for several hours. A maintainer replaces the battery or worn contact assembly. A future product builder reuses the dock only within the same validated interface class.

Confirmed from the current user request and follow-up:

- First product family: Neo Modular Desk Clock; **dock is the first engineering deliverable**.
- Modules should be portable, replaceable and easy to assemble, with reuse across compatible NeoVerse products as a goal.
- Measure a **basic clock core and display first**; advanced AI modules do not define the first power budget.
- Portability means moving the **assembled clock** and operating for several hours.

The rendering proposes USB-C input, a replaceable 1-cell battery, NTC, a metal enclosure and four pogo contacts. These are architecture/packaging candidates. Exact battery chemistry/capacity, power rating, connector count, dimensions, processor, runtime, cost and market are not approved specifications.

## 2. Scope and use scenarios

| Scenario | Expected outcome |
|---|---|
| Desk operation | Compatible core/display operates continuously from an adequate adapter while spare power charges the battery. |
| Pick up and move | User unplugs input and carries the assembled clock; output stays within the receiver envelope without reset while battery conditions permit. |
| Return to desk | Reconnecting input restores source priority and charging without an unexplained clock reset. |
| Battery becoming empty | Clock receives warning where possible, sheds optional load and shuts down predictably before unsafe cell discharge. |
| Attach/remove receiver | Correct attachment gives controlled power; partial, unsupported or misaligned attachment remains safe. Removal need not preserve operation of the removed core. |
| Repair | With power removed, a maintainer replaces an approved pack or contact daughterboard and reruns prescribed checks. |
| Future reuse | Another receiver declares a matching electrical/mechanical class and stays within the declared total budget. |

MVP includes input qualification, regulation, battery charging and protection, automatic source transition, controlled module output, basic status, serviceable mechanics and repeatable bench verification. Add telemetry/supervisor complexity only where required to meet those behaviors.

Later candidates: richer state-of-charge estimation, programmable power policies, extra output classes, multiple protected dock ports and broader ecosystem telemetry. Out of v0: built-in mains AC/DC, high-power robot motors, USB-C source/power-bank output, arbitrary power daisy chaining, dual-dock paralleling, universal module data transport, wireless charging, camera/AI functions and a companion app. A battery gauge may be useful but voltage-only state estimates must not be labeled accurate percentage without validation.

## 3. Target register

The following are **proposed engineering targets**, not accepted release specifications. G0/G1 must replace or accept them explicitly. A TBD blocks the related gate; it cannot count as a passed requirement.

| Target | Draft value or decision | Freeze gate |
|---|---|---|
| Initial regulated rail | 5.0 V candidate | G1 |
| Receiver voltage window | Proposed 4.75–5.25 V at receiver input, including specified steady and transient cases; receiver datasheet may require tighter limits | G0/G1 |
| Continuous test envelope | Up to 2 A / 10 W at 5 V as bench target; final rating from load and thermal/contact measurements | G1/G2 |
| Peak current / duration, inrush and allowed input capacitance | TBD from real receiver | G0/G2 |
| Portable runtime | Several hours confirmed; exact hours, brightness, activity, ambient, battery age and end condition TBD | G0 |
| Recharge time | TBD for specified empty-to-full endpoint with clock operating and idle; state adapter/cable and thermal conditions | G0/G1 |
| Battery | Replaceable approved protected pack; 1S candidate; chemistry, usable energy, charge/discharge limits and NTC TBD | G1/G2 |
| Adapter and cable | Sink-only USB-C candidate; detect available current; required source power/profile and cable limits TBD | G1 |
| Ripple/noise allocation | TBD from receiver susceptibility; distinguish switching ripple from transient droop; record probe bandwidth/method | G1 |
| Protection and detach energy | Input/output OV/UV thresholds, current limits, trip/retry timing, hysteresis, fault voltage/energy boundary and residual contact voltage/energy vs time TBD from component and receiver limits | G1/G2 |
| Thermal limits | Cell charge/discharge limits, semiconductor derating and accessible-surface limits from chosen pack/components and intended market | G1/G2 |
| Mechanical envelope | Dimensions, assembled mass/CG, retention force, torque, drop/carry/tip and contact travel limits TBD | G0/G2 |
| Mating/service life | Proposed 5,000 complete assembly mating cycles; validate with selected contact rating and expected use | G2 |
| Quiescent and storage use | Off/ship leakage, sleep draw, storage duration and acceptable audible/indicator behavior TBD | G1/G2 |
| Cost and production | Retail, BOM ceiling, repair cost, volume, yield and markets TBD; obtain quotes rather than invented prices | G0/G3 |

## 4. Functional and quality requirements

All requirements below are proposed **Must** items for v0 unless marked otherwise. Ownership indicates the responsible engineering role, not an assigned person. Test IDs refer to [verification plan](verification-plan.md).

| ID | Requirement and acceptance definition | Owner | Verification |
|---|---|---|---|
| PWR-01 | Qualify USB-C input and enforce the detected advertisement/negotiated contract. Never assume 3 A from a receptacle or passive attachment alone. Unsupported/weak sources produce bounded behavior and clear status. Define input overvoltage/undervoltage, hysteresis, transient and ESD protection for supported profiles and bounded faults. | Electrical | T01, T02, T12 |
| PWR-02 | Supply the fixed agreed rail across the accepted load/input/battery envelope at the receiver input. Meet the frozen voltage, ripple and transient limits. | Electrical | T02, T03 |
| PWR-03 | With a usable battery, adapter insertion/removal causes no receiver reset and stays inside its accepted electrical envelope. At depleted/absent battery, behavior is explicitly bounded rather than promised seamless. | Electrical | T04 |
| PWR-04 | Allocate source power to system load before charging. Reduce charging within cell limits when source power is constrained; if total demand remains too high, indicate/shed/reject the mode predictably. Record any battery supplement and net drain. | Electrical/embedded | T02, T05 |
| PWR-05 | Boot and operate the supported receiver from an adequate adapter with a missing or deeply depleted pack; prove no unsafe charge or reboot loop. The architecture must meet this proposed v0 requirement. | Electrical | T05, T06 |
| PWR-06 | Provide hardware-enforced short/overcurrent/overtemperature protection, output overvoltage protection and controlled inrush. Include a regulator fault that raises output voltage. Fault energy, trip threshold/timing, recovery and no-damage conditions must be frozen before fault tests. | Electrical | T07, T08, T12 |
| PWR-07 | Block unintended power into the USB input, battery charging path and dock from the receiver within defined fault conditions. Prevent unpowered data-pin backfeeding; do not directly connect multiple supplies. | Electrical | T08, T09 |
| PWR-08 | Support minimum/dim clock load without unwanted power-bank auto-off. Define off/ship/wake behavior, undervoltage hysteresis and acceptable sleep/storage consumption. | Electrical/embedded | T06, T10 |
| BAT-01 | Use only a documented approved pack matched to charger voltage/chemistry, current limits, independent protection and keyed connector. Preserve traceability and supplier limits; prohibit unspecified replacement cells. Meet frozen recharge-time targets under the stated load and adapter conditions. | Electrical/mechanical | T05, T11, T16 |
| BAT-02 | Monitor battery temperature. Open/shorted NTC, out-of-range temperature and protection trips cause a defined safe response; hardware cell protection is not replaced by firmware. | Electrical | T11, T12 |
| BAT-03 | Meet the exact G0 portable runtime in the declared real clock use profile. Record measured watt-hours, age/health, temperature, brightness and cutoff; derive capacity from energy rather than mAh alone. | Electrical/product | T13 |
| BAT-04 | Warn before energy cutoff where energy allows, then stop discharge within pack limits. Prevent repeated low-voltage reboot cycles. Abrupt pack protection must still leave hardware safe. | Embedded/electrical | T06, T13 |
| BAT-05 | Allow approved battery replacement without destructive adhesives, pouch deformation, wire trapping or special soldering. Replacement is powered down; supplier-defined expansion space and fastener clearances are maintained. | Mechanical | T14, T16 |
| IFC-01 | Publish a versioned power/mechanical class: rail, polarity, current and peak duration, capacitance/inrush, detection, signaling, fault behavior, envelope, allowed receivers and HW/FW versions. Unknown classes do not receive unrestricted power. | Systems | T08, T09, T17 |
| IFC-02 | Default main pogo output off when undocked. Correct attach uses qualified detection, debounce and controlled enable; partial/rotated/tilted contact and conductive debris do not cause unsafe operation. Specify limited detection/bootstrap energy and maximum residual exposed-contact voltage/energy vs time after detach or disable; discharge must not sink a foreign supply unsafely. | Electrical/mechanical | T07, T08 |
| IFC-03 | Select contacts/harness for measured continuous/peak current, warm end-of-life resistance and mechanical travel. Contact count/order is not frozen by the render; parallel contacts require sharing and one-contact-fault tests. | Electrical/mechanical | T03, T08, T14 |
| IFC-04 | Provide core-visible power-available, charging/battery, low-energy and fault status through an agreed interface; basic protections work without status communication. Define invalid/stale telemetry behavior and no phantom powering. | Embedded/systems | T09, T15 |
| MEC-01 | Use physical keying, independent retention and compression stops. Housing rather than pogo springs carries display torque and carrying/cable loads. Meet the frozen assembled stability and retention envelope. | Mechanical | T14 |
| MEC-02 | Keep battery/electronics isolated from shell edges, screws and wear points; define USB-C shield/ESD and any shell bonding intentionally. Validate liners, strain relief and abrasion; do not rely on anodizing alone. | Mechanical/electrical | T08, T12, T14, T16 |
| MEC-03 | Make the wear contact assembly replaceable using documented parts and procedure. USB-C loads and daughterboard wiring have suitable strain relief and no accessible pinch points. | Mechanical | T14, T16 |
| UX-01 | Expose unambiguous normal/charging/battery/low-energy/fault states with a quiet desk/night behavior. Charging indication distinguishes actual charge from connected-but-power-limited operation. | Product/embedded | T05, T10, T15 |
| SYS-01 | Coordinate total dock budget with carrier branch limits. Accessory short or overload is isolated before global shutdown where essential clock continuity is required; unavoidable global faults are reported/documented. | Systems/carrier | T07, T18 |
| SYS-02 | Define time/alarm retention and recovery on total power loss at core level. State clearly that an undocked core and battery service do not have guaranteed uninterrupted power. | Core/product | T06, T15 |
| REL-01 | Operate within frozen battery/component/surface temperature limits in enclosure under worst supported simultaneous load/charge. Thermal limiting is observable, stable and avoids reset oscillation. | Electrical/mechanical | T12 |
| REL-02 | Safe hardware defaults persist during supervisor startup, reset, hang, corrupted/stale communication or update interruption if updates exist. Define watchdog and recovery behavior for any required firmware. | Embedded/electrical | T15 |
| REL-03 | Complete proposed minimum 24-hour representative desk run and 100 adapter transitions without unexplained resets, excessive heat or state faults. These are engineering screens, not lifetime reliability claims. | Verification | T04, T10 |
| ECO-01 | Demonstrate the interface on a second compatible receiver fixture; validate on an actual second NeoVerse product before claiming that product's compatibility. No universal compatibility claim from a single fixture. | Systems | T17 |
| MFG-01 | Produce a reviewed schematic/BOM/CAD package, service/assembly instructions and repeatable end-of-line test covering rail, charge, protection, status and traceability. Set cost targets and source critical alternates before pilot. | Engineering/manufacturing | T16, T19 |
| CMP-01 | Select target markets and shipping modes; document applicable battery transport, product safety, EMC/material obligations and required test/pack documentation. External-adapter compliance does not establish complete dock compliance. | Product/compliance | T20 |

## 5. Dependencies and readiness

The core/display owner must supply load waveforms, allowable rail window, startup capacitance, brownout behavior, time-retention design and power/status needs. The carrier owner must supply branch protection and power admission behavior. The pack supplier must supply charge/discharge/temperature limits, protection and expansion/service constraints. Product ownership must close runtime, size, price and market targets.

PRD review readiness is achieved when these unknowns have owners and closure gates. Design freeze requires accepted numerical limits and interface revision. Product acceptance requires executed evidence for mandatory requirements; a document, simulation or component datasheet alone cannot satisfy hardware acceptance.
