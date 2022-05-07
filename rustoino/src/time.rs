use esp8266_hal::time::Microseconds;
use xtensa_lx::timer::get_cycle_count;

const TICKS_PER_SECOND: u32 = 80_000_000;
const TICKS_PER_MILLI: u32 = 80_000;
const TICKS_PER_MICRO: u32 = 80;

pub struct IntervalMeasure(u32);

impl IntervalMeasure {
	pub fn new() -> Self {
		Self(get_cycle_count())
	}

	pub fn poll<T: From<Microseconds>>(&self) -> T {
		let now = get_cycle_count();
		let elapsed = if now > self.0 {
			now - self.0
		} else {
			u32::MAX - self.0 + now
		};
		Microseconds(elapsed / TICKS_PER_MICRO).into()
	}

	pub fn pace<T: From<Microseconds>>(&mut self) -> T {
		let now = get_cycle_count();
		let elapsed = if now > self.0 {
			now - self.0
		} else {
			u32::MAX - self.0 + now
		};
		self.0 = now;
		Microseconds(elapsed / TICKS_PER_MICRO).into()
	}
}
