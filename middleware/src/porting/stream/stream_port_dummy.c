
#include "app_include.h"
#include "stream_port.h"
#if defined(INCLUDE_TR069) ||defined(INCLUDE_HMWMGMT) 
#else

void stream_port_post_streamgap(void) { }

void stream_port_post_vodreq(int ms) { }
void stream_port_post_vodstop(int ms) { }
void stream_port_post_vodpause(int ms) { }
void stream_port_post_vodresume(int ms) { }
void stream_port_post_vodforward(int ms) { }
void stream_port_post_vodbackward(int ms) { }
void stream_port_post_channelzap(int ms) { }

void stream_port_post_bitrate(int mult, int width, int rate) { }

void stream_port_post_pklosts(int flag, int width, int totals, int losts) { }
void stream_port_post_bitrate_percent(int mult, int width, int rate) { }

void stream_port_post_ok(int mult, int rrt) { }
void stream_port_post_fail(int mult, char *url, int err_no) { }

void stream_port_post_abend_fail(char *url) { }
void stream_port_post_abend_end(void) { }

void stream_port_post_http(void) { }
void stream_port_post_http_fail(char *url, int err_no) { }

#endif
