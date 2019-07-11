#ifndef __CERTUS_QOS_H__
#define __CERTUS_QOS_H__

#ifdef __cplusplus
extern "C" {
#endif

int startCertusQos(void);
int stopCertusQos(void);
int writeCertusConfig(void);
unsigned int sqm_get_listen_port(void);
unsigned int sqm_get_server_port(void);
void sqm_set_listen_port(unsigned int port);
void sqm_set_server_port(unsigned int port);

#ifdef __cplusplus
}
#endif

#endif
