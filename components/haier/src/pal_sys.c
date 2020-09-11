
/**
 * @file pal_sys.c
 * @brief
 *
 * @date
 * @author
 *
 */

#include "pal_common.h"

//#define uplus_sys_log iot_debug_print


/*!
 * \brief MD5上下文初始化。
 * \param [in] ctx 如果成功初始化，返回上下文标识；否则不填写。
 * \return 成功返回0，失败返回-1。
 */
ZK_MD5_CTX md5_ctx;
uplus_s32 uplus_tool_md5_init(uplus_ctx_id *ctx)
{
	MD5Init(&md5_ctx);
	
	*ctx = &md5_ctx;
	
	uplus_sys_log("[zk u+] md5_init");
	
	return 0;
}

/*!
 * \brief MD5增量计算。
 * \param [in] ctx 上下文标识。
 * \param [in] buf 数据指针。
 * \param [in] len 数据长度。
 * \return N/A。
 */
void uplus_tool_md5_update(uplus_ctx_id ctx, uplus_u8 *buf, uplus_s32 len)
{
	MD5Update(ctx, buf, len);
	
	uplus_sys_log("[zk u+] md5_update len=%d", len);
}


/*!
 * \brief 结束MD5计算。
 * \param [in] ctx 上下文标识。
 * \param [out] output MD5计算的摘要值，16字节。
 * \return N/A。
 */
void uplus_tool_md5_finish(uplus_ctx_id ctx, uplus_u8 *output/*16*/)
{
	MD5Final(ctx, output);

	uplus_sys_log("[zk u+] md5_finish");
}

/*!
 * \brief SHA256上下文初始化。
 * \param [in] ctx 如果成功初始化，返回上下文标识；否则不填写。
 * \return 成功返回0，失败返回-1。
 */
zk_SHA256_CTX sha256_ctx;
uplus_s32 uplus_tool_sha256_init(uplus_ctx_id *ctx)
{
	zk_SHA256_Init(&sha256_ctx);
	
	*ctx = &sha256_ctx;

	uplus_sys_log("[zk u+] sha256_init");
		
	return 0;
}

/*!
 * \brief SHA256增量计算。
 * \param [in] ctx 上下文标识。
 * \param [in] buf 数据指针。
 * \param [in] len 数据长度。
 * \return N/A。
 */
void uplus_tool_sha256_update(uplus_ctx_id ctx, uplus_u8 *buf, uplus_s32 len)
{
	zk_SHA256_Update(ctx, buf, len);

	uplus_sys_log("[zk u+] sha256_update");
}


/*!
 * \brief 结束SHA256计算。
 * \param [in] ctx 上下文标识。
 * \param [out] output SHA256计算的摘要值，32字节。
 * \return N/A。
 */
void uplus_tool_sha256_finish(uplus_ctx_id ctx, uplus_u8 *output/*32*/)
{
	zk_SHA256_Final(output, ctx);

	uplus_sys_log("[zk u+] sha256_finish");
}

/*!
 * \brief AES CBC加解密计算。
 * \param [in] mode 操作类型，MODE_ENCRYPT/MODE_DECRYPT。
 * \param [in] key 密钥。
 * \param [in] key_len 密钥长度，128比特或者256比特。
 * \return N/A。
 */
void uh_crypt_aes_crypt_cbc_with_iv(int mode, uint8_t * key, uint32_t key_bit_len,
	uint8_t * in, size_t in_len, uint8_t *iv, size_t iv_len, uint8_t *out)
{
	mbedtls_aes_context aes;

	mbedtls_aes_init(&aes);
	if (mode == MODE_ENCRYPT)
	{
		uplus_sys_log("[zk u+] aes_crypt_cbc:ENCRYPT start");
		
		mbedtls_aes_setkey_enc(&aes, key, key_bit_len);
		mbedtls_aes_crypt_cbc(&aes, mode, in_len, iv, in, out);
		
		uplus_sys_log("[zk u+] aes_crypt_cbc:ENCRYPT end");
	}
	else
	{
		uplus_sys_log("[zk u+] aes_crypt_cbc:DECRYPT start");
		
		mbedtls_aes_setkey_dec(&aes, key, key_bit_len);
		mbedtls_aes_crypt_cbc(&aes, mode, in_len, iv, in, out);
		
		uplus_sys_log("[zk u+] aes_crypt_cbc:DECRYPT end");
	}
	mbedtls_aes_free(&aes);
}

void uplus_tool_aes_crypt_cbc(uplus_s32 mode, uplus_u8 * key, uplus_s32 key_len/*128 or 256*/, uplus_u8 * in, uplus_s32 in_len, uplus_u8 *out)
{
	uint8_t iv[16] = {0};

	uplus_sys_log("[zk u+] aes_crypt_cbc:start");
	
	uh_crypt_aes_crypt_cbc_with_iv(mode, key, key_len, in, in_len, iv, sizeof(iv), out);	
	
	uplus_sys_log("[zk u+] aes_crypt_cbc:end");
}

/*!
 * \brief 初始化RSA公钥上下文。
 *
 * \param [in] key RSA公钥，通常是PEM格式。
 * \param [in] key_len RSA公钥长度。
 * \param [out] out_len 输出的报文长度。
 *
 * \return RSA公钥上下文。
 */
void * uplus_tool_rsa_public_init(const uplus_u8 *key, uplus_size_t key_len, uplus_size_t *out_len)
{
	int ret;
	mbedtls_pk_context *pk;
	mbedtls_rsa_context *rsa;

	uplus_sys_log("[zk u+] rsa_public_init:start");
	
	pk = malloc(sizeof(mbedtls_pk_context));
	if (pk == NULL)
	{
		uplus_sys_log("[zk u+] rsa_public_init_1:malloc fail");
		return (NULL);
	}
	mbedtls_pk_init(pk);
	ret = mbedtls_pk_parse_public_key(pk, key, key_len + 1);
	if (ret)
	{
		free(pk);
		return (NULL);
	}
	rsa = pk->pk_ctx;
	*out_len = rsa->len;

	uplus_sys_log("[zk u+] rsa_public_init:end");
	
	return pk;
}

static int mbedtls_self_entropy_poll(void *data, uint8_t *output, size_t len, size_t *olen )
{
	uint32_t rand_val = rand();

	*olen = 0;
	if (len < sizeof(uint32_t))
		return (0);
	
	memcpy(output, &rand_val, sizeof(uint32_t));
	*olen = sizeof(uint32_t);
	
	return (0);
}

void * uh_ctr_drbg_create(void)
{
	mbedtls_ctr_drbg_context *ctr_drbg = NULL;
	mbedtls_entropy_context *entropy = NULL;

	ctr_drbg = calloc(1, sizeof(mbedtls_ctr_drbg_context));
	if (ctr_drbg == NULL)
		goto exit;
	mbedtls_ctr_drbg_init(ctr_drbg);

	entropy = calloc(1, sizeof(mbedtls_entropy_context));
	if (entropy == NULL)
		goto exit;
	
	mbedtls_entropy_init(entropy);
	mbedtls_entropy_add_source(entropy, mbedtls_self_entropy_poll, NULL, sizeof(uint32_t), MBEDTLS_ENTROPY_SOURCE_STRONG);
	mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy, (unsigned char *)"uh_ctr_drbg", strlen("uh_ctr_drbg"));

	return (ctr_drbg);
exit:
	if (ctr_drbg)
		mbedtls_ctr_drbg_free(ctr_drbg);
	if (entropy)
		mbedtls_entropy_free(entropy);
	return (NULL);
}

int uh_ctr_drbg_random(void * ctx, uint8_t *output, size_t output_len)
{
	mbedtls_ctr_drbg_context *ctr_drbg = ctx;

	if (mbedtls_ctr_drbg_random(ctr_drbg, output, output_len) != 0)
		return (-1);
	return (0);
}

void uh_ctr_drbg_free(void * ctx)
{
	mbedtls_ctr_drbg_context *ctr_drbg = ctx;
	mbedtls_entropy_context *entropy;

	entropy = ctr_drbg->p_entropy;
	if (entropy)
	{
		mbedtls_entropy_free(entropy);
		free(entropy);
	}
	mbedtls_ctr_drbg_free(ctr_drbg);
	free(ctr_drbg);
}

/*!
 * \brief RSA加密。
 *
 * \param [in] ctx RSA公钥/私钥上下文。
 * \param [in] mode UH_RSA_XXX，。
 * \param [in] in 待加密的数据。
 * \param [in] in_len 待加密的数据长度。
 * \param [out] out 输出，加密后的数据。
 *
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_tool_rsa_encrypt(void *ctx, uplus_s32 mode,const uplus_u8 *in, uplus_size_t in_len,uplus_u8 *out)
{
	int ret;
	mbedtls_pk_context *pk;
	mbedtls_rsa_context *rsa;
	void * ctr_drbg;

	uplus_sys_log("[zk u+] rsa_encrypt:start");
	
	ctr_drbg = uh_ctr_drbg_create();
	if (ctr_drbg == NULL)
		return (-1);
	
	pk = (mbedtls_pk_context *)ctx;
	rsa = pk->pk_ctx;
	ret = mbedtls_rsa_pkcs1_encrypt(rsa, uh_ctr_drbg_random, ctr_drbg,mode, in_len, in, out);
	
	uh_ctr_drbg_free(ctr_drbg);

	uplus_sys_log("[zk u+] rsa_encrypt:end ret=%d", ret);
	
	if (ret)
		return (-1);
	
	return (0);
}

/*!
 * \brief RSA解密。
 *
 * \param [in] ctx RSA公钥/私钥上下文。
 * \param [in] mode UH_RSA_XXX，。
 * \param [in] in 待解密的数据。
 * \param [in] in_len 待解密的数据长度。
 * \param [out] out 输出，解密后的数据。
 * \param [in] out_max_len 存放解密数据缓存的最大长度。
 *
 * \return 成功返回UH_OK，失败返回UH_FAIL。
 */
uplus_s32 uplus_tool_rsa_decrypt(
	void *ctx, uplus_s32 mode,
	const uplus_u8 *in, uplus_size_t *out_len, uplus_u8 *out, uplus_size_t out_max_len)
{
	int ret;
	mbedtls_pk_context *pk;
	mbedtls_rsa_context *rsa;
	void * ctr_drbg;

	uplus_sys_log("[zk u+] rsa_decrypt:start");
	
	ctr_drbg = uh_ctr_drbg_create();
	if (ctr_drbg == NULL)
		return (-1);
	
	pk = (mbedtls_pk_context *)ctx;
	rsa = pk->pk_ctx;
	ret = mbedtls_rsa_pkcs1_decrypt(rsa, uh_ctr_drbg_random, ctr_drbg, mode, out_len, in, out, out_max_len);

	uh_ctr_drbg_free(ctr_drbg);

	uplus_sys_log("[zk u+] rsa_decrypt:end ret=%d", ret);

	if (ret)
		return (-1);
	
	return (0);
}

/*!
 * \brief 释放RSA公钥上下文。
 *
 * \param [in] ctx RSA公钥/私钥上下文。
 *
 * \return N/A。
 */
void uplus_tool_rsa_public_finish(void *ctx)
{
	uplus_sys_log("[zk u+] rsa_public_finish:start");
	
	mbedtls_pk_free(ctx);
	
	free(ctx);

	uplus_sys_log("[zk u+] rsa_public_finish:end");
}

/*!
 * \brief 生成随机数据。
 *
 * \param [out] output 输出的随机数据。
 * \param [in] output_len 输出缓存的大小。
 *
 * \return N/A。
 */
void uplus_tool_random_generate(uplus_u8 *output, uplus_size_t output_len)
{
	uplus_sys_log("[zk u+] random_generate start len=%d", output_len);

	if(output != NULL)
	{
		(void)os_get_random(output, output_len);
	
		uplus_sys_log("[zk u+] random_generate end len=%d 0x%x 0x%x 0x%x 0x%x", output_len, output[0], output[1], output[2], output[3]);
	}
}

/*!
 * \}
 */

/*!
 * \defgroup SYSTEM PAL APIs
 * \details SYSTEM相关PAL接口。
 * \note
 * \{
 */

/*! \def LED_XXX
 *
 * \brief 系统状态指示，可以用于指示灯控制。按比特表示。
 DEV_STATUS 串行设备（与底板通信）状态。 1-通信正常，0-通信错误。
 POWER_SAVE 省电模式。1-省电，0-正常。
 POWER_STATUS 电源指示。1-电源正常，0-电源故障。
 STATUS_MASK 状态掩码，3个比特。
 	STATUS_CONFIG 配置模式。
 	STATUS_NO_WIFI WIFI关闭。
 	STATUS_AP_FAIL 无法连接AP。
 	STATUS_AP_OK AP连接正常。
 	STATUS_SVR_OK 服务器连接正常。
 */

/*!
 * \brief 状态灯控制
 * \param [in] led_status_ind 状态指示，按比特定义不同含义，LED_XXX。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_led_control(uplus_u16 led_status_ind)
{
	//uplus_sys_log("[zk u+] sys_led_control %d", led_status_ind);
	return 0;
}

/*!
 * \brief 启动看门狗。
 * \details 看门狗用于系统崩溃或者系统不响应时的恢复。
 * \param [in] timeout 看门狗超时时间，单位毫秒。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_watchdog_start(uplus_u32 timeout)
{
	uplus_sys_log("[zk u+] watchdog_start");
	//return wdg_init();
	return 0;
}

/*!
 * \brief 喂狗。
 * \return N/A。
 */
void uplus_sys_watchdog_feed(void)
{
	uplus_sys_log("[zk u+] watchdog_feed");
	//(void)wdg_feed();
}

/*!
 * \brief 打开串行设备（与底板通信）。
 * \details 打开串行设备。串行设备硬件参数根据硬件平台和协议自行确定。串行设备发送缓存至少256字节，接收缓存至少1K字节。串行设备读数据需支持select机制。
 如果设备的读写是同一个描述符，则返回一个描述符；否则，需要返回两个描述符，第一个是用于读，第二个用于写，两个描述符都必须合法有效。
 * \param [in] fd 设备描述符，如果成功返回设备描述符；否则不填写。
 * \param [in] baudrate 波特率。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_serial_open(uplus_s32 *fd,uplus_u32 baudrate)
{	
	uplus_sys_log("[zk u+] sys_serial_open");
	return -1;
}

/*!
 * \brief 关闭串行设备（与底板通信）。
 * \details 关闭串行设备。如果打开串行设备返回了两个描述符，则uplugSDK会主动调用两次本接口以关闭串行设备。
 * \param [in] fd 设备描述符。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_serial_close(uplus_s32 fd)
{
	uplus_sys_log("[zk u+] sys_serial_close");
	return -1;
}

/*!
 * \brief 从串行设备读取数据（与底板通信）。
 * \details 从串行设备读取数据。如果接收缓存中数据的长度大于len，返回len长度数据；否则返回实际数据长度。
 * \param [in] fd 设备描述符。
 * \param [out] buf 数据缓存。
 * \param [in] len 数据缓存大小（期望读取的数据长度）。
 * \return 成功返回实际读取的数据长度，失败返回-1。
 */
uplus_s32 uplus_sys_serial_read(uplus_s32 fd, uplus_u8 *buf, uplus_size_t len)
{
	uplus_sys_log("[zk u+] sys_serial_read");
	return -1;
}

/*!
 * \brief 向串行设备写入数据（与底板通信）。
 * \param [in] fd 设备描述符。
 * \param [in] buf 数据缓存。
 * \param [in] len 数据缓存大小（期望写入的数据长度）。
 * \return 成功返回实际写入的数据长度，失败返回-1。
 */
uplus_s32 uplus_sys_serial_write(uplus_s32 fd, uplus_u8 *buf, uplus_size_t len)
{
	uplus_sys_log("[zk u+] sys_serial_write");
	return -1;
}

/*!
 * \brief 系统复位。
 * \return N/A。
 */
void uplus_sys_system_reset(void)
{
	//uplus_sys_log("[zk u+] sys_system_reset");
	restart(3);
}

/*!
 * \brief 获取系统平台SDK版本信息
 * \return 描述系统平台SDK版本信息的字符串。
 */
char SDK_VER_INFO[] = "2.0.0";
uplus_s8 *uplus_sys_sdk_ver_get(void)
{
	return SDK_VER_INFO;
}

/*!
 * \brief 关闭系统日志及调试输出，用于安全目的。
 * \param [in] on_off 开关，1-开，0-关。
 * \return N/A。
 */
void uplus_sys_log_on_off(uplus_u8 on_off)
{
}

/*! \def ZONE_0/ZONE_1
 *
 * \brief 互为备份的两个区域，用于配置数据和OTA镜像。
 */
#define ZONE_1 1
#define ZONE_2 2

/*!
 * \brief 读取配置，从非易失性存储器中读取配置到内存中。
 * \param [in] config_zone 配置区域，ZONE_1/ZONE_2。
 * \param [out] conf 内存指针，存放读取的配置数据。
 * \param [in] len 长度，读取的配置数据长度。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_config_read(uplus_u8 config_zone, uplus_u8 *conf, uplus_size_t len)
{
	char *f_name = NULL;
	int fd = 0;

	if(config_zone == ZONE_1)
		f_name = FILE_NAME_U_CONFIG;
	else if(config_zone == ZONE_2)
		f_name = FILE_NAME_U_CONFIG_BAT;

	fd = vfs_open(f_name, O_RDWR | O_CREAT);
	if(fd < 0)
	{
		uplus_sys_log("[zk u+] sys_config_read_0 open %s fail", f_name);
		return -1;
	}

	if (vfs_lseek(fd, 0, SEEK_SET) < 0)
    {
        uplus_sys_log("[zk u+] sys_config_read_1: lseek Fail");
        vfs_close(fd);
        return -1;
    }

	if (vfs_read(fd, conf, len) != len)
    {
        uplus_sys_log("[zk u+] sys_config_read_2: read Fail");

        vfs_close(fd);

        return -1;
    }
	vfs_close(fd);
	uplus_sys_log("[zk u+] sys_config_read_3:len=%d", len);
	return 0;
}

/*!
 * \brief 写配置，将配置写入非易失性存储器中。
 * \param [in] config_zone 配置区域，ZONE_1/ZONE_2。
 * \param [in] conf 内存指针，存放待写入的配置数据。
 * \param [in] len 长度，待写入的配置数据长度。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_config_write(uplus_u8 config_zone, uplus_u8 *conf, uplus_size_t len)
{
	char *f_name = NULL;
	int fd = 0;

	if(config_zone == ZONE_1)
		f_name = FILE_NAME_U_CONFIG;
	else if(config_zone == ZONE_2)
		f_name = FILE_NAME_U_CONFIG_BAT;
		
	fd = vfs_open(f_name, O_RDWR | O_CREAT);
	if(fd < 0)
	{
		uplus_sys_log("[zk u+] sys_config_write_0 open %s fail", f_name);
		return -1;
	}

	if (vfs_lseek(fd, 0, SEEK_SET) < 0)
    {
        uplus_sys_log("[zk u+] sys_config_write_1: lseek Fail");
        vfs_close(fd);
        return -1;
    }
	
	if(vfs_write(fd, conf, len) != len)
    {
        OSI_LOGE(0, "[zk u+] sys_config_write_2: write Fail");
        vfs_close(fd);
        return -1;
    }
    vfs_close(fd);
	uplus_sys_log("[zk u+] sys_config_write_3:len=%d", len);
	return 0;
}


/*!
 * \brief 获取当前运行镜像区域。
 * \param [in] image_zone，当前运行的版本镜像区，ZONE_1/ZONE_2/ZONE_OTHER。如果成功，返回当前运行的镜像区域；否则不填写。如果当前运行版本既不是镜像1，也不是镜像2，返回ZONE_OTHER。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_run_image_get(uplus_u8 *image_zone)
{
	uplus_sys_log("[zk u+] sys_run_image_get");
	return -1;
}

/*!
 * \brief 读取镜像数据。
 * \param image_zone，镜像区域，ZONE_XXX。
 * \param [in] offset 偏移量。
 * \param [out] buf 内存指针。
 * \param [in] len 读取的长度。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_image_read(uplus_u8 image_zone, uplus_u32 offset, uplus_u8 *buf, uplus_size_t len)
{
	uplus_sys_log("[zk u+] sys_image_read");
	return -1;
}

/*!
 * \brief 写入镜像数据。
 * \param image_zone，镜像区域，ZONE_XXX。
 * \param [in] offset 偏移量。
 * \param [in] buf 待写入数据的内存指针。
 * \param [in] len 写入的长度。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_image_write(uplus_u8 image_zone, uplus_u32 offset, uplus_u8 *buf, uplus_size_t len)
{
	uplus_sys_log("[zk u+] sys_image_write");
	return -1;
}

/*!
 * \brief 准备写入镜像数据。
 * \param [in] image_zone，镜像区域，ZONE_XXX。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_image_start(uplus_u8 image_zone)
{
	uplus_sys_log("[zk u+] sys_image_start");
	return -1;
}

/*!
 * \brief 完成写入镜像数据。
 * \param [in] image_zone，镜像区域，ZONE_XXX。
 * \param [in] result 写入镜像数据的结果，OTA_IMAGE_OK/OTA_IMAGE_FAIL
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_image_finish(uplus_u8 image_zone, uplus_s32 result)
{
	uplus_sys_log("[zk u+] sys_image_finish");
	return -1;
}

/*!
 * \brief 镜像版本切换
 * \param [in] image_zone 镜像区域，ZONE_1/ZONE_2。下一次启动的版本区域是image_zone。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_image_switch(uplus_u8 image_zone)
{
	uplus_sys_log("[zk u+] sys_image_switch");
	return -1;
}

/*!
 * \brief 打印调试信息。
 * \details 调试信息格式化输出。可根据需要，自由输出日志信息到串口，网络等。
 * \param [in] fmt 格式化字符串。
 * \param [in] ... 可变参数。
 * \return 成功返回实际输出信息的长度，失败返回-1。
 */
char sys_debug_buff[500];
uplus_s32 uplus_sys_debug_printf(const uplus_s8 *fmt, ...)
{
	memset(sys_debug_buff, 0, sizeof(sys_debug_buff));
	
	va_list ap;
    va_start(ap, fmt);

	vsnprintf(sys_debug_buff, 500, fmt, ap);
    OSI_LOGXI(OSI_LOGPAR_S, 0, "%s", sys_debug_buff);
	
    va_end(ap);

	return 0;
}

/*!
 * \brief 系统日志输出。
 * \details 日志信息格式化输出。可根据需要，自由输出日志信息到串口，网络等。
 * \param [in] fmt 格式化字符串。
 * \param [in] ... 可变参数。
 * \return 成功返回实际输出信息的长度，失败返回-1。
 */
char sys_log_buff[500];
uplus_s32 uplus_sys_log(const uplus_s8 *fmt, ...)
{
	memset(sys_log_buff, 0, sizeof(sys_log_buff));

	va_list ap;
    va_start(ap, fmt);

	vsnprintf(sys_log_buff, 500, fmt, ap);

	OSI_LOGXI(OSI_LOGPAR_S, 0, "%s", sys_log_buff);

    va_end(ap);

	return 0;
}

/*!
 * \brief 系统相关初始化。
 * \details 执行与API接口相关的初始化动作。该接口会在uplugSDK初始化时调用。
 * \return N/A。
 */
uplus_s32 uplus_sys_init(void)
{
	return 0;
}

/*!
 * \brief 系统周期执行动作。
 * \details 周期是10秒。该接口每个周期被调用。
 * \return N/A。
 */
void uplus_sys_period(void)
{
	uplus_sys_log("[zk u+] sys_period");
}

/*!
 * \brief 获取系统复位类型。
 * \return 系统复位类型，如果不支持，返回SYSTEM_RESET_TYPE_UNDEF。
 */
uplus_s32 uplus_sys_reset_type_get(void)
{
	uplus_sys_log("[zk u+] sys_reset_type_get");
	
	/*E_AMOPENAT_POWERON_REASON cause;

	cause = iot_pmd_get_poweronCasue();
	
	if(cause == OPENAT_PM_POWERON_BY_RESET)
		return SYSTEM_RESET_TYPE_RESET;
	else if(cause == OPENAT_PM_POWERON_BY_WATCHDOG)
		return SYSTEM_RESET_TYPE_WATCHDOG;
	else*/
		return SYSTEM_RESET_TYPE_UNDEF;
}

/*!
 * \brief 获取系统保留区域大小。
 * \details 系统保留区域是软件重启不丢失数据的区域。
 * 如果不支持，返回0。
 * 如果支持，必须是256字节的整倍数。
 * param [out] res_area 如果支持直接访问，返回直接访问指针；否则返回NULL。
 * \return 系统保留区域大小。
 */
uplus_u16 uplus_sys_res_area_get_size(void **mem)
{
	uplus_sys_log("[zk u+] sys_res_area_get_size");
	*mem = NULL;
	return 0;
}

/*!
 * \brief 获取系统保留区域的数据。
 * \details 如果可以直接访问保留区域，实现空功能即可。
 * \param [in] offset 偏移量。
 * \param [out] buf 内存指针。
 * \param [in] len 读取的长度。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_res_area_read(uplus_u32 offset, uplus_u8 *buf, uplus_size_t len)
{
	int fd = 0;
	char *file_name = "/res_area";

	fd = vfs_open(file_name, O_RDWR | O_CREAT);
	if(fd < 0)
	{
		uplus_sys_log("[zk u+] sys_res_area_read_0 open %s fail", file_name);
		return -1;
	}

	if (vfs_lseek(fd, offset, SEEK_SET) < 0)
    {
        uplus_sys_log("[zk u+] sys_res_area_read_1: lseek Fail");
        vfs_close(fd);
        return -1;
    }

	if (vfs_read(fd, buf, len) != len)
    {
        uplus_sys_log("[zk u+] sys_res_area_read_0_2: read Fail");

        vfs_close(fd);

        return -1;
    }
	vfs_close(fd);
	uplus_sys_log("[zk u+] sys_res_area_read_0_3:len=%d offset=%d", len, offset);
	return 0;
}

/*!
 * \brief 向系统保留区域写入数据。
 * \details 如果可以直接访问保留区域，实现空功能即可。
 * \param [in] offset 偏移量。
 * \param [in] buf 待写入数据的内存指针。
 * \param [in] len 写入的长度。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_res_area_write(uplus_u32 offset, uplus_u8 *buf, uplus_size_t len)
{
	int fd = 0;
	char *file_name = "/res_area";
	
	fd = vfs_open(file_name, O_RDWR | O_CREAT);
	if(fd < 0)
	{
		uplus_sys_log("[zk u+] sys_res_area_write_0 open %s fail", file_name);
		return -1;
	}

	if (vfs_lseek(fd, offset, SEEK_SET) < 0)
    {
        uplus_sys_log("[zk u+] sys_res_area_write_1: lseek Fail");
        vfs_close(fd);
        return -1;
    }
	
	if(vfs_write(fd, buf, len) != len)
    {
        OSI_LOGE(0, "[zk u+] sys_res_area_write_2: write Fail");
        vfs_close(fd);
        return -1;
    }
    vfs_close(fd);
	uplus_sys_log("[zk u+] sys_res_area_write_3:len=%d offset=%d", len, offset);
	return 0;
}

/*!
 * \brief 进入低功耗模式。
 * \details 进入低功耗后该接口不会返回，直到被唤醒。进入低功耗可能会有三种方式，
 * 一是被唤醒后，程序在原来的位置上继续运行。
 * 二是被唤醒后，软件重启。
 * 三是被唤醒后，重新上电。
 * \return N/A。
 */
void uplus_sys_low_power_enter(void)
{
	uplus_sys_log("[zk u+] sys_low_power_enter");
}

/*!
 * \brief 退出低功耗模式。
 * \return N/A。
 */
void uplus_sys_low_power_exit(void)
{
	uplus_sys_log("[zk u+] sys_low_power_exit");
}

/*!
 * \brief 打开SOC片选开关，用于外部关闭SOC芯片。
 * \param [in] enable 1-打开，0-关闭。
 * \return N/A。
 */
void uplus_sys_chip_enable(uplus_u8 enable)
{
	uplus_sys_log("[zk u+] sys_chip_enable");
}

/*!
 * \brief 打开文件。
 * \details 不要求系统支持文件系统。
 * \param [out] fd 文件描述符，能唯一标识文件。
 * \param [in] file_type 文件类型，参见FILE_TYPE_XXX。
 * \param [in] file_name 文件名称，通常是字符串形式的TYPEID。
 * \param [in] flags 文件标志，参见FILE_FLAGS_XXX。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_file_open(uplus_s32 *fd, uplus_u8 file_type, const char *file_name, uplus_s32 flags)
{
	if((fd == NULL) || (file_name == NULL))
		return -1;

	int fd_1 = vfs_open(HAIER_FILE_NAME, O_RDWR | O_CREAT);
	if(fd_1 < 0)
	{
		uplus_sys_log("[zk u+] sys_file_open %s fail", file_name);
		return -1;
	}
	uplus_sys_log("[zk u+] sys_file_open %s suc", file_name);
	*fd = fd_1;

	return 0;
}

/*!
 * \brief 关闭文件。
 * \param [in] fd 文件描述符。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_sys_file_close(uplus_s32 fd)
{
	uplus_sys_log("[zk u+] sys_file_close");
	return vfs_close(fd);
}

/*!
 * \brief 文件定位。
 * \param [in] fd 文件描述符。
 * \param [in] offset 相对于whence的偏移量，以字节为单位。
 * \param [in] whence 给定的位置，参见FILE_SEEK_XXX。
 * \return 成功返回0，失败返回-1。
 */
uplus_u32 uplus_sys_file_seek(uplus_s32 fd, uplus_s32 offset, uplus_s32 whence)
{
	int off = vfs_lseek(fd, offset, whence-1);
	uplus_sys_log("[zk u+] sys_file_seek:%d", off);
	if(off < 0)
		return -1;
	else
		return 0;
}

/*!
 * \brief 读文件。
 * \details 从文件读取数据。如果接收缓存中数据的长度大于len，返回len长度数据；否则返回实际数据长度。
 * \param [in] fd 文件描述符。
 * \param [out] buf 数据缓存。
 * \param [in] len 数据缓存大小（期望读取的数据长度）。
 * \return 成功返回实际读取的数据长度，失败返回-1。
 */
uplus_s32 uplus_sys_file_read(uplus_s32 fd, uplus_u8 *buf, uplus_size_t len)
{
	uplus_sys_log("[zk u+] sys_file_read:%d", len);
	return vfs_read(fd, buf, len);
}

/*!
 * \brief 写文件。
 * \param [in] fd 文件描述符。
 * \param [in] buf 数据缓存。
 * \param [in] len 数据缓存大小（期望写入的数据长度）。
 * \return 成功返回实际写入的数据长度，失败返回-1。
 */
uplus_s32 uplus_sys_file_write(uplus_s32 fd, uplus_u8 *buf, uplus_size_t len)
{
	uplus_sys_log("[zk u+] sys_file_write:%d", len);
	return vfs_write(fd, buf, len);
}

/*!
 * \}
 */


