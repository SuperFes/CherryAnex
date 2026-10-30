//! Raw HID reports understood by the keyboard firmware.
//! Mirrors keyboards/cosmos/anex/keymaps/via/hostinfo.h.

/// Outside VIA's command IDs, so VIA never sees these reports.
pub const COMMAND: u8 = 0x80;
pub const VERSION: u8 = 1;
/// VIA's raw HID report size.
pub const REPORT_LEN: usize = 32;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Stats {
    pub hour: u8,
    pub minute: u8,
    pub cpu: u8,
    pub ram: u8,
}

/// Encodes a report, prefixed with the zero report ID hidapi expects.
pub fn encode(stats: &Stats) -> [u8; REPORT_LEN + 1] {
    let mut report = [0; REPORT_LEN + 1];
    report[1..7].copy_from_slice(&[
        COMMAND,
        VERSION,
        stats.hour.min(23),
        stats.minute.min(59),
        stats.cpu.min(100),
        stats.ram.min(100),
    ]);
    report
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_firmware_layout() {
        let report = encode(&Stats { hour: 13, minute: 37, cpu: 42, ram: 64 });
        assert_eq!(report[0], 0, "report ID");
        assert_eq!(&report[1..7], &[0x80, 1, 13, 37, 42, 64]);
        assert!(report[7..].iter().all(|&b| b == 0));
        assert_eq!(report.len(), 33);
    }

    #[test]
    fn clamps_out_of_range_values() {
        let report = encode(&Stats { hour: 99, minute: 99, cpu: 255, ram: 101 });
        assert_eq!(&report[3..7], &[23, 59, 100, 100]);
    }
}
