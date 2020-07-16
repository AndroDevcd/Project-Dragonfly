#include "native_mapping.h"

using namespace std;
using namespace snb_api;

#ifdef __cplusplus
extern "C" {
#endif

EXPORTED short snb_link_proc(const char* funcName, int32_t addr) {
	string name = funcName;

	if(
		name == "main#__srt_global.get_dst_cm" && addr == 3020
		|| name == "gpio#__srt_global.setup" && addr == 3016
		|| name == "gpio#__srt_global.pin_mode" && addr == 3017
		|| name == "gpio#__srt_global.write_pin" && addr == 3018
		|| name == "gpio#__srt_global.read_pin" && addr == 3019
	) {
		return 1;
	}

return 0;
}

void call_main$__srt_global_get_dst_cm() {
	import(main); 

	var trg(internal::getfpNumAt(0));
	var echo(internal::getfpNumAt(1));
	var $result(internal::getfpNumAt(0));
	$result = get_dst_cm(trg, echo);
}

void call_gpio$__srt_global_setup() {
	import(gpio); 

	setup();
}

void call_gpio$__srt_global_pin_mode() {
	import(gpio); 

	var pin(internal::getfpNumAt(0));
	var mode(internal::getfpNumAt(1));
	pin_mode(pin, mode);
}

void call_gpio$__srt_global_write_pin() {
	import(gpio); 

	var pin(internal::getfpNumAt(0));
	var value(internal::getfpNumAt(1));
	write_pin(pin, value);
}

void call_gpio$__srt_global_read_pin() {
	import(gpio); 

	var pin(internal::getfpNumAt(0));
	var $result(internal::getfpNumAt(0));
	$result = read_pin(pin);
}

EXPORTED void snb_main(long procAddr) {
	switch(procAddr) {
		case 3020: 
			call_main$__srt_global_get_dst_cm();
			break;
		case 3016: 
			call_gpio$__srt_global_setup();
			break;
		case 3017: 
			call_gpio$__srt_global_pin_mode();
			break;
		case 3018: 
			call_gpio$__srt_global_write_pin();
			break;
		case 3019: 
			call_gpio$__srt_global_read_pin();
			break;
	}
}

#ifdef __cplusplus
}
#endif
