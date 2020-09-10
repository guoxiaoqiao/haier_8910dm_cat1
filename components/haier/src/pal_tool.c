
/**
 * @file pal_tool.c
 * @brief
 *
 * @date
 * @author
 *
 */

#include "pal_common.h"

/*!
 * \brief C库memcpy。
 */
void *uplus_tool_memcpy(void *dest, const void *src, uplus_size_t n)
{
	return memcpy(dest, src, n);
}

/*!
 * \brief C库memmove。
 */
void *uplus_tool_memmove(void *dest, const void *src, uplus_s32 n)
{
	return memmove(dest, src, n);
}

/*!
 * \brief C库memset。
 */
void *uplus_tool_memset(void *s, uplus_s32 c, uplus_s32 n)
{
	return memset(s, c, n);
}

/*!
 * \brief C库memcmp。
 */
uplus_s32 uplus_tool_memcmp(const void *s1, const void *s2, uplus_size_t n)
{
	return memcmp(s1, s2, n);
}

/*!
 * \brief C库strncmp。
 */
uplus_s32 uplus_tool_strncmp(const uplus_s8 *s1, const uplus_s8 *s2, uplus_size_t n)
{
	return strncmp(s1, s2, n);
}

/*!
 * \brief C库strcmp。
 */
uplus_s32 uplus_tool_strcmp(const uplus_s8 *s1, const uplus_s8 *s2)
{
	return strcmp(s1, s2);
}

/*!
 * \brief C库strncpy。
 */
uplus_s8 * uplus_tool_strncpy(uplus_s8 *dest, const uplus_s8 *src, uplus_size_t n)
{
	return strncpy(dest, src, n);
}

/*!
 * \brief C库strlen。
 */
uplus_size_t uplus_tool_strlen(uplus_s8 *s)
{
	return strlen(s);
}

/*!
 * \brief C库snprintf。
 */
uplus_s32 uplus_tool_snprintf(uplus_s8* str, uplus_size_t size, const uplus_s8* fmt, ...)
{
	va_list ap;
    va_start(ap, fmt);

	int num = vsnprintf(str, size, fmt, ap);
    
    va_end(ap);
	
	return num;
}

/*!
 * \brief C库rand。
 */
uplus_s32 uplus_tool_rand(void)
{
	uplus_s32 rd = rand();

	//uplus_sys_log("[zk u+] tool_rand=%d", rd);
	
	return rd;
}

/*!
 * \brief C库srand。
 */
void uplus_tool_srand(uplus_u32 seed)
{
	//uplus_sys_log("[zk u+] tool_srand:seed=%d", seed);
	
	srand(seed);
}

/*!
 * \brief C库strtol。
 */
uplus_s32 uplus_tool_strtol(const uplus_s8 *str, uplus_s8 **endptr, uplus_s32 base)
{
	return strtol(str,  endptr, base);
}

/*!
 * \brief C库strchr。
 */
uplus_s8 * uplus_tool_strchr(const uplus_s8 *str, uplus_s32 c)
{
	return strchr(str,c);
}

/*!
 * \brief 申请内存。
 * \param [in] n 申请内存的大小，单位字节。
 * \return 成功返回内存指针，失败返回NULL。
 */
void *uplus_tool_malloc(uplus_size_t n)
{
	return malloc(n);
}

/*!
 * \brief 申请数据内存。
 * \note 数据内存是可以与外设交互的内存，例如可用于DMA的内存，硬件加解密可以访问的内存等等。
 * \param [in] n 申请内存的大小，单位字节。
 * \return 成功返回内存指针，失败返回NULL。
 */
void *uplus_tool_data_malloc(uplus_size_t n)
{
	return malloc(n);
}

/*!
 * \brief 释放内存。
 * \param [in] ptr，内存指针。
 * \return N/A。
 */
void uplus_tool_free(void *ptr)
{
	free(ptr);
}

