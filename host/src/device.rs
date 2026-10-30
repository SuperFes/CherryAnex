//! Finds the keyboard's VIA raw HID interface.

use hidapi::{HidApi, HidDevice};

pub const VENDOR_ID: u16 = 0xB00B;
pub const PRODUCT_ID: u16 = 0xB335;
// QMK's raw HID interface; the keyboard's other interfaces share the IDs.
pub const USAGE_PAGE: u16 = 0xFF60;
pub const USAGE: u16 = 0x61;

pub fn is_raw_hid(vendor_id: u16, product_id: u16, usage_page: u16, usage: u16) -> bool {
    (vendor_id, product_id, usage_page, usage) == (VENDOR_ID, PRODUCT_ID, USAGE_PAGE, USAGE)
}

pub fn open(api: &mut HidApi) -> Option<HidDevice> {
    api.refresh_devices().ok()?;
    api.device_list()
        .find(|d| is_raw_hid(d.vendor_id(), d.product_id(), d.usage_page(), d.usage()))
        .and_then(|d| d.open_device(api).ok())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_only_the_raw_hid_interface() {
        assert!(is_raw_hid(0xB00B, 0xB335, 0xFF60, 0x61));
        assert!(!is_raw_hid(0xB00B, 0xB335, 0x0001, 0x06), "keyboard interface");
        assert!(!is_raw_hid(0xB00B, 0xB336, 0xFF60, 0x61), "other product");
        assert!(!is_raw_hid(0x1234, 0xB335, 0xFF60, 0x61), "other vendor");
    }
}
