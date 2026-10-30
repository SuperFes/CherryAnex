mod protocol;
mod stats;

use std::thread;
use std::time::Duration;

fn main() {
    let mut sampler = stats::Sampler::new().expect("system stats unavailable");
    loop {
        thread::sleep(Duration::from_secs(1));
        println!("{:?}", sampler.sample());
    }
}
