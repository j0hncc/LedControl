#ifndef INPUT_PROTOCOLS_TPM2NET_H_
#define INPUT_PROTOCOLS_TPM2NET_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ledCallback)( char * buffer, int len) ;

void ICACHE_FLASH_ATTR tpm2net_init( ledCallback cb);

#ifdef __cplusplus
}
#endif

#endif /* INPUT_PROTOCOLS_TPM2NET_H_ */
