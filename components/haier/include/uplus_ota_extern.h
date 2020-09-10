
/**
 * @file uplus_ota_extern.h
 * @brief
 *
 * @date 2017-03-20
 * @author fanming
 *
 */

#ifndef __UPLUS_OTA_EXTERN_H__
#define __UPLUS_OTA_EXTERN_H__

#ifdef __cplusplus
extern "C" {
#endif

/*for ota config*/
typedef uplus_s32 (*ota_check)(void *para, uplus_u8 *buf, uplus_u32 len, uplus_u8 force_upgrade);
typedef enum
{
	OTA_DIGEST_CHECK_RET_OK = 0, /*digest verify ok*/
	OTA_DIGEST_CHECK_RET_FAIL = -1, /*digest verify fail*/
	OTA_DIGEST_CHECK_RET_CONTINUE = 1, /*continue to verify*/
	OTA_DIGEST_CHECK_RET_RELOAD = 2 /*verify from start*/
} ota_digest_check_result_t;
/*
 * start: buf is not NULL, len > 0, offset = 0;
 * end: buf is not NULL, len > 0, offset + len >= total_len;
 * normal: buf is not NULL, len > 0, offset > 0, offset + len < total_len;
 * exit: buf is NULL, len = 0, offset = 0;
 */
typedef uplus_s32 (*ota_digest_check)(uplus_u8 image_zone, uplus_u8 *buf, uplus_u32 len, uplus_u32 offset, uplus_u32 total_len);

typedef uplus_s32 (* ota_location_func)(uplus_u8 *info, uplus_u32 info_len, uplus_u8 *is_content, uplus_u32 * offset_start, uplus_u32 * offset_end, uplus_u8 *image_zone);

/*
 * @brief
 * @param
 * @return
 */
extern uplus_s32 uplus_ota_init(void);

/*
 * @brief
 * @param
 * @return
 */
extern uplus_s32 uplus_ota_register_check(ota_check check, void *para, uplus_u32 offset, uplus_u32 len);

/*
 * @brief
 * @param
 * @return
 */
extern uplus_s32 uplus_ota_register_none_content_check(ota_check check, void *para);

/*
 * @brief
 * @param
 * @return
 */
extern uplus_s32 uplus_ota_register_digest_check(ota_digest_check digest_check);

/*
 * @brief
 * @param
 * @return
 */
extern uplus_s32 uplus_ota_register_location(ota_location_func location);

typedef struct
{
	uplus_s32 key_offset;
	uplus_s32 digest_offset;

	uplus_s32 kg_check_offset;
	uplus_u32 kg_check_len;

	uplus_s32 digest_check_offset;
	uplus_u32 digest_check_len;
} ota_digest_check_config_t;

/*
 * @brief
 * @param
 * @return
 */
extern uplus_s32 uplug_ota_digest_check_default(uplus_u8 image_zone, uplus_u8 *buf, uplus_u32 len, uplus_u32 offset, uplus_u32 total_len);

/*
 * @brief
 * @param
 * @return
 */
extern void uplug_ota_digest_check_config(ota_digest_check_config_t * ota_digest_check_config);

/*
 * @brief
 * @param
 * @return
 */
extern uplus_u8 uplug_ota_is_upgrading(uplus_u8 *from_where);

#ifdef __cplusplus
}
#endif

#endif /*__UPLUS_OTA_EXTERN_H__*/

