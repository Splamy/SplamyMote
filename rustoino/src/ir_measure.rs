use esp8266_hal::time::Microseconds;

use crate::time::IntervalMeasure;

pub struct IRMeasure {
	time: IntervalMeasure,
	done: bool,
	pub index: usize,
	pub buffer: [u16; 128],
}

impl IRMeasure {
	pub fn new() -> Self {
		Self {
			time: IntervalMeasure::new(),
			index: 0,
			done: false,
			buffer: [0; 128],
		}
	}

	pub fn reset(&mut self) {
		self.index = 0;
		self.done = false;
	}

	pub fn measure(&mut self) {
		let elapsed = self.time.pace::<Microseconds>().0;
		if self.done || self.index == 128 {
			return;
		}
		if elapsed > u16::MAX as u32 {
			if self.index > 0 {
				self.done = true;
			}
			return;
		}
		self.buffer[self.index] = elapsed as u16;
		self.index += 1;
	}

	pub fn done(&self) -> bool {
		if self.index == 0 {
			return false;
		}
		if self.done {
			return true;
		}
		let elapsed = self.time.poll::<Microseconds>().0;
		elapsed > u16::MAX as u32
	}
}
