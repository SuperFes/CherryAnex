//! Samples the PC stats shown on the keyboard.

use chrono::{Local, Timelike};
use sysinfo::System;

use crate::protocol::Stats;

pub struct Sampler {
    system: System,
}

impl Sampler {
    pub fn new() -> Result<Self, sysinfo::Error> {
        let mut system = System::new()?;
        // CPU usage is measured between refreshes, so prime the first one.
        system.refresh_cpu_usage();
        Ok(Self { system })
    }

    pub fn sample(&mut self) -> Stats {
        self.system.refresh_cpu_usage();
        self.system.refresh_memory();
        let now = Local::now();
        Stats {
            hour: now.hour() as u8,
            minute: now.minute() as u8,
            cpu: round_percent(self.system.global_cpu_usage()),
            ram: percent(self.system.used_memory(), self.system.total_memory()),
        }
    }
}

fn round_percent(value: f32) -> u8 {
    if value.is_nan() {
        return 0;
    }
    value.round().clamp(0.0, 100.0) as u8
}

fn percent(used: u64, total: u64) -> u8 {
    if total == 0 {
        return 0;
    }
    ((used.min(total) as u128 * 100 + total as u128 / 2) / total as u128) as u8
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn rounds_and_clamps_cpu() {
        assert_eq!(round_percent(42.4), 42);
        assert_eq!(round_percent(42.5), 43);
        assert_eq!(round_percent(-1.0), 0);
        assert_eq!(round_percent(150.0), 100);
        assert_eq!(round_percent(f32::NAN), 0);
    }

    #[test]
    fn memory_percent() {
        assert_eq!(percent(0, 0), 0);
        assert_eq!(percent(1, 3), 33);
        assert_eq!(percent(2, 3), 67);
        assert_eq!(percent(16 << 30, 16 << 30), 100);
        assert_eq!(percent(u64::MAX, 1), 100);
    }

    #[test]
    fn samples_in_range() {
        let stats = Sampler::new().unwrap().sample();
        assert!(stats.hour < 24 && stats.minute < 60);
        assert!(stats.cpu <= 100 && stats.ram <= 100);
    }
}
