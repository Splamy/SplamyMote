//#![no_std]
#![no_main]

mod time;
mod ir_measure;

use core::fmt::Write;
use esp8266_hal::gpio::{Gpio2, InterruptMode, Output, PushPull};
use esp8266_hal::interrupt::{enable_interrupt, InterruptType};
use esp8266_hal::target::Peripherals;
use esp8266_hal::{prelude::*, target};
use ir_measure::IRMeasure;
use panic_halt as _;
use time::IntervalMeasure;
use xtensa_lx::mutex::{CriticalSectionMutex, Mutex};
use xtensa_lx::timer::get_cycle_count;
//use embedded_svc::wifi;

static LED: CriticalSectionMutex<Option<Gpio2<Output<PushPull>>>> = CriticalSectionMutex::new(None);
static IR_READER: CriticalSectionMutex<Option<IRMeasure>> = CriticalSectionMutex::new(None);

#[entry]
fn main() -> ! {
	let dp = Peripherals::take().unwrap();
	let pins = dp.GPIO.split();

	let mut pin_green = pins.gpio0.into_push_pull_output(); // D3
	let mut pin_red = pins.gpio2.into_push_pull_output(); // D4
	let mut pin_ir_recv = pins.gpio5.into_pull_up_input(); // D1
	let mut pin_ir_send = pins.gpio4.into_push_pull_output(); // D2
	let mut pin_button_blue = pins.gpio14.into_pull_up_input(); // D5

	let x = embedded_svc::wifi::AccessPointConfiguration::default();

	// EspWifi::new(dp.WIFI)
	// 	.connect("ESP8266-TEST", "12345678")
	// 	.unwrap();

	let (mut timer1, _) = dp.TIMER.timers();

	let mut serial = dp
		.UART0
		.serial(pins.gpio1.into_uart(), pins.gpio3.into_uart());

	enable_interrupt(InterruptType::GPIO);
	pin_button_blue.set_interrupt_mode(InterruptMode::BothEdges);
	pin_ir_recv.set_interrupt_mode(InterruptMode::BothEdges); // let _handler =

	pin_red.set_low().unwrap();
	(&LED).lock(|led_locked| *led_locked = Some(pin_red));
	(&IR_READER).lock(|ir| *ir = Some(IRMeasure::new()));

	//let mut measure = IntervalMeasure::new();

	loop {
		timer1.delay_ms(100u32);
		let mut buf: Option<([u16; 128], usize)> = None;
		(&IR_READER).lock(|ir| {
			let mut ir = ir.as_mut().unwrap();
			if ir.done() {
				buf = Some((ir.buffer, ir.index));
				ir.reset();
			}
		});
		if let Some(arr) = &buf {
			writeln!(serial, "arr: {:?}", &arr.0[..arr.1]).unwrap();
		}
		//(&LED).lock(|led| led.as_mut().unwrap().toggle().unwrap());
	}
}

// struct GpioPinData();
// struct GpioHandler<T>(PhantomData<T>);

// esp8266_hal::int_handler!(GPIO => GpioHandler(GpioPinData));

// impl GpioPinData {
// 	#[must_use = "the interrupt handler must be kept in scope for the interrupt to be handled"]
// 	pub fn attach_interrupt<F: FnMut(&mut GpioPinData)>(self,mut f: F) {
// 		(&LED).lock(|led| led.as_mut().unwrap().toggle().unwrap());
// 	}
// }


#[interrupt(gpio)]
unsafe fn gpio() {
	(*target::GPIO::ptr())
		.gpio_status_w1tc
		.write(|w| w.bits(0xffff));
	(&IR_READER).lock(|ir| {
		ir.as_mut().unwrap().measure();
	});
	(&LED).lock(|led| led.as_mut().unwrap().toggle().unwrap());
}
