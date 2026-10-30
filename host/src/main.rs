//! Sends PC stats to the Anex keyboard's displays once a second.

// Started at login, so release builds shouldn't open a console window.
#![cfg_attr(all(windows, not(debug_assertions)), windows_subsystem = "windows")]

mod device;
mod protocol;
mod stats;

use std::process::ExitCode;
use std::thread;
use std::time::Duration;

use hidapi::HidApi;

const INTERVAL: Duration = Duration::from_secs(1);

const USAGE: &str = "usage: anex-host [--verbose]

Sends the time and CPU/RAM usage to the keyboard every second, reconnecting
whenever it is plugged back in.";

fn main() -> ExitCode {
    let mut verbose = false;
    for arg in std::env::args().skip(1) {
        match arg.as_str() {
            "-v" | "--verbose" => verbose = true,
            "-V" | "--version" => {
                println!("anex-host {}", env!("CARGO_PKG_VERSION"));
                return ExitCode::SUCCESS;
            }
            "-h" | "--help" => {
                println!("{USAGE}");
                return ExitCode::SUCCESS;
            }
            _ => {
                eprintln!("{USAGE}");
                return ExitCode::FAILURE;
            }
        }
    }

    let mut api = match HidApi::new() {
        Ok(api) => api,
        Err(err) => {
            eprintln!("anex-host: HID unavailable: {err}");
            return ExitCode::FAILURE;
        }
    };
    let mut sampler = match stats::Sampler::new() {
        Ok(sampler) => sampler,
        Err(err) => {
            eprintln!("anex-host: system stats unavailable: {err}");
            return ExitCode::FAILURE;
        }
    };

    let mut keyboard = None;
    loop {
        thread::sleep(INTERVAL);
        let stats = sampler.sample();
        if keyboard.is_none() {
            keyboard = device::open(&mut api);
            if keyboard.is_some() {
                eprintln!("anex-host: keyboard connected");
            }
        }
        if let Some(device) = &keyboard {
            if let Err(err) = device.write(&protocol::encode(&stats)) {
                eprintln!("anex-host: keyboard disconnected ({err})");
                keyboard = None;
            } else if verbose {
                eprintln!("anex-host: sent {stats:?}");
            }
        }
    }
}
