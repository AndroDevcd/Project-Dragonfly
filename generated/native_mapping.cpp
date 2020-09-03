#include "native_mapping.h"

using namespace std;
using namespace snb_api;

#ifdef __cplusplus
extern "C" {
#endif

EXPORTED uint32_t snb_link_proc(const char* funcName) {
	string name = funcName;

	if(name == "common.gpio#__srt_global.setup") {  return 3598;  }
	else if(name == "common.gpio#__srt_global.pin_mode") {  return 3599;  }
	else if(name == "common.gpio#__srt_global.write_pin") {  return 3600;  }
	else if(name == "common.gpio#__srt_global.read_pin") {  return 3601;  }
	else if(name == "common.network.driver#__srt_global.setup") {  return 3602;  }
	else if(name == "common.network.driver#__srt_global.set_transmission_lvl") {  return 3603;  }
	else if(name == "common.network.driver#__srt_global.set_transmission_rate") {  return 3604;  }
	else if(name == "common.network.driver#__srt_global.dump_details") {  return 3605;  }
	else if(name == "common.network.driver#__srt_global.set_retry_count") {  return 3606;  }
	else if(name == "common.network.driver#__srt_global.power_down") {  return 3607;  }
	else if(name == "common.network.driver#__srt_global.get_signal_strength") {  return 3608;  }
	else if(name == "common.network.driver#__srt_global.get_network_quality") {  return 3609;  }
	else if(name == "common.network.driver#__srt_global.jam") {  return 3610;  }
	else if(name == "common.network.driver#__srt_global.read") {  return 3611;  }
	else if(name == "common.network.driver#__srt_global.listen") {  return 3612;  }
	else if(name == "common.network.driver#__srt_global.send") {  return 3613;  }
	else if(name == "common.network.driver#__srt_global.get_last_error") {  return 3614;  }
	else if(name == "ui.driver#__srt_global.setup") {  return 3777;  }
	else if(name == "ui.driver#__srt_global.clear_display") {  return 3778;  }
	else if(name == "ui.driver#__srt_global.display") {  return 3779;  }
	else if(name == "ui.driver#__srt_global.dim") {  return 3780;  }
	else if(name == "ui.driver#__srt_global.move_cursor") {  return 3781;  }
	else if(name == "ui.driver#__srt_global.draw_word") {  return 3782;  }
	else if(name == "ui.driver#__srt_global.draw_img") {  return 3783;  }
	else return -1;
}

void call_common_gpio$__srt_global_setup() {
	import(common_gpio); 

	setup();
}

void call_common_gpio$__srt_global_pin_mode() {
	import(common_gpio); 

	var pin(internal::getfpNumAt(0));
	var mode(internal::getfpNumAt(1));
	pin_mode(pin, mode);
}

void call_common_gpio$__srt_global_write_pin() {
	import(common_gpio); 

	var pin(internal::getfpNumAt(0));
	var value(internal::getfpNumAt(1));
	write_pin(pin, value);
}

void call_common_gpio$__srt_global_read_pin() {
	import(common_gpio); 

	var pin(internal::getfpNumAt(0));
	var $result(internal::getfpNumAt(0));
	$result = read_pin(pin);
}

void call_common_network_driver$__srt_global_setup() {
	import(common_network_driver); 

	var trnsLvl(internal::getfpNumAt(0));
	var rate(internal::getfpNumAt(1));
	var delay(internal::getfpNumAt(2));
	var retryCount(internal::getfpNumAt(3));
	var isClient(internal::getfpNumAt(4));
	setup(trnsLvl, rate, delay, retryCount, isClient);
}

void call_common_network_driver$__srt_global_set_transmission_lvl() {
	import(common_network_driver); 

	var level(internal::getfpNumAt(0));
	set_transmission_lvl(level);
}

void call_common_network_driver$__srt_global_set_transmission_rate() {
	import(common_network_driver); 

	var level(internal::getfpNumAt(0));
	set_transmission_rate(level);
}

void call_common_network_driver$__srt_global_dump_details() {
	import(common_network_driver); 

	dump_details();
}

void call_common_network_driver$__srt_global_set_retry_count() {
	import(common_network_driver); 

	var delay(internal::getfpNumAt(0));
	var count(internal::getfpNumAt(1));
	set_retry_count(delay, count);
}

void call_common_network_driver$__srt_global_power_down() {
	import(common_network_driver); 

	power_down();
}

void call_common_network_driver$__srt_global_get_signal_strength() {
	import(common_network_driver); 

	var $result(internal::getfpNumAt(0));
	$result = get_signal_strength();
}

void call_common_network_driver$__srt_global_get_network_quality() {
	import(common_network_driver); 

	object $result = internal::getfpLocalAt(0);
	var_array $returnVal_ = get_network_quality();
	set($result, $returnVal_);
}

void call_common_network_driver$__srt_global_jam() {
	import(common_network_driver); 

	var channel(internal::getfpNumAt(0));
	jam(channel);
}

void call_common_network_driver$__srt_global_read() {
	import(common_network_driver); 

	object $result = internal::getfpLocalAt(0);
	_int8_array $returnVal_ = read();
	set($result, $returnVal_);
}

void call_common_network_driver$__srt_global_listen() {
	import(common_network_driver); 

	object $result = internal::getfpLocalAt(0);
	_int8_array $returnVal_ = listen();
	set($result, $returnVal_);
}

void call_common_network_driver$__srt_global_send() {
	import(common_network_driver); 

	object $tmpField0 = internal::getfpLocalAt(0);
	_int8_array data(internal::getVarPtr($tmpField0), internal::getSize($tmpField0), $tmpField0);
	send(data);
}

void call_common_network_driver$__srt_global_get_last_error() {
	import(common_network_driver); 

	var $result(internal::getfpNumAt(0));
	$result = get_last_error();
}

void call_ui_driver$__srt_global_setup() {
	import(ui_driver); 

	setup();
}

void call_ui_driver$__srt_global_clear_display() {
	import(ui_driver); 

	clear_display();
}

void call_ui_driver$__srt_global_display() {
	import(ui_driver); 

	display();
}

void call_ui_driver$__srt_global_dim() {
	import(ui_driver); 

	var yes(internal::getfpNumAt(0));
	dim(yes);
}

void call_ui_driver$__srt_global_move_cursor() {
	import(ui_driver); 

	var x(internal::getfpNumAt(0));
	var y(internal::getfpNumAt(1));
	move_cursor(x, y);
}

void call_ui_driver$__srt_global_draw_word() {
	import(ui_driver); 

	object $tmpField0 = internal::getfpLocalAt(0);
	_int8_array str(internal::getVarPtr($tmpField0), internal::getSize($tmpField0), $tmpField0);
	var len(internal::getfpNumAt(1));
	var text_size(internal::getfpNumAt(2));
	draw_word(str, len, text_size);
}

void call_ui_driver$__srt_global_draw_img() {
	import(ui_driver); 

	object $tmpField0 = internal::getfpLocalAt(0);
	var_array bytes(internal::getVarPtr($tmpField0), internal::getSize($tmpField0), $tmpField0);
	var width(internal::getfpNumAt(1));
	var height(internal::getfpNumAt(2));
	var x(internal::getfpNumAt(3));
	var y(internal::getfpNumAt(4));
	var skip_count(internal::getfpNumAt(5));
	draw_img(bytes, width, height, x, y, skip_count);
}

EXPORTED void snb_main(long procAddr) {
	try {
		switch(procAddr) {
			case 3598: 
				call_common_gpio$__srt_global_setup();
				break;
			case 3599: 
				call_common_gpio$__srt_global_pin_mode();
				break;
			case 3600: 
				call_common_gpio$__srt_global_write_pin();
				break;
			case 3601: 
				call_common_gpio$__srt_global_read_pin();
				break;
			case 3602: 
				call_common_network_driver$__srt_global_setup();
				break;
			case 3603: 
				call_common_network_driver$__srt_global_set_transmission_lvl();
				break;
			case 3604: 
				call_common_network_driver$__srt_global_set_transmission_rate();
				break;
			case 3605: 
				call_common_network_driver$__srt_global_dump_details();
				break;
			case 3606: 
				call_common_network_driver$__srt_global_set_retry_count();
				break;
			case 3607: 
				call_common_network_driver$__srt_global_power_down();
				break;
			case 3608: 
				call_common_network_driver$__srt_global_get_signal_strength();
				break;
			case 3609: 
				call_common_network_driver$__srt_global_get_network_quality();
				break;
			case 3610: 
				call_common_network_driver$__srt_global_jam();
				break;
			case 3611: 
				call_common_network_driver$__srt_global_read();
				break;
			case 3612: 
				call_common_network_driver$__srt_global_listen();
				break;
			case 3613: 
				call_common_network_driver$__srt_global_send();
				break;
			case 3614: 
				call_common_network_driver$__srt_global_get_last_error();
				break;
			case 3777: 
				call_ui_driver$__srt_global_setup();
				break;
			case 3778: 
				call_ui_driver$__srt_global_clear_display();
				break;
			case 3779: 
				call_ui_driver$__srt_global_display();
				break;
			case 3780: 
				call_ui_driver$__srt_global_dim();
				break;
			case 3781: 
				call_ui_driver$__srt_global_move_cursor();
				break;
			case 3782: 
				call_ui_driver$__srt_global_draw_word();
				break;
			case 3783: 
				call_ui_driver$__srt_global_draw_img();
				break;
		}
	} catch(Exception &e) {
		snb_api::internal::prepareException(e.exceptionClass);
	}
}

#ifdef __cplusplus
}
#endif
