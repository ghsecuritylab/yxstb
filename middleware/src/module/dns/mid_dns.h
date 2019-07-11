
#ifndef __MID_DNS_H__
#define __MID_DNS_H__

//���ʱʱ��
#define MID_DNS_TIMEOUT		60

#ifdef __cplusplus
extern "C" {
#endif

enum {
	DNS_MSG_NOTFOUND = -3,//��dns�����������ɹ���������������
	DNS_MSG_TIMEOUT = -2,//��DNS���������ӳ�ʱ�����������ɴ
	DNS_MSG_ERROR = -1,//��������
	DNS_MSG_OK,//�ɹ�
};

typedef void (*mid_dns_f)(int arg, int dnsmsg, unsigned int hostip);
/*
	DNS�첽����
	����ֵ��
			-1 ��������ʧ��
			0  �첽����
			����ֵ��ʾ �Ѿ����������� ֱ�ӷ���IP��ַ
 */
int mid_dns_resolve(const char* hostname, mid_dns_f callback, int arg, int timeout);
/*
	������
	to_array[0] = 2;//
	to_array[1] = 4;//�ڶ���������μ��
	to_array[2] = 8;
	to_count = 3;//��ֵ��С

	���ֻ��һ��dns0����ô��ֻ��3�ν���
		dns0 ���2�� dns0 ���4�� dns0

	���������dns���ֱ�Ϊdns0��dns1����ô�ᷢ��6��
		dns0 ���2�� dns1 ���2�� dns0 ���4�� dns1 ���4�� dns0 ���8�� dns1

 */
int mid_dns_resolve_to(const char* hostname, mid_dns_f callfunc, int callarg, int timeout, int* to_array, int to_count);

//DNSͬ������
int mid_dns_gethost(const char* hostname, unsigned int *hostip, int timeout);
int mid_dns_gethost_to(const char* hostname, unsigned int *hostip, int timeout, int* to_array, int to_count);

void mid_dns_clean(const char* hostname);
unsigned int mid_dns_cache(const char* hostname);

void mid_dns_setsrv(char* dns1, char* dns2);
void mid_dns_init(void);

#ifdef __cplusplus
}
#endif

#endif//__MID_DNS_H__
