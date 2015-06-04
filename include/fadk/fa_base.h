#ifndef __FOXIT_FA_BASE_H__
#define __FOXIT_FA_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 *���к�������ֵ��ԭ�в��ֱ��ֺ�GSDKһ���⣬ADK������Ӳ�����Ϣ
 */
/** @brief	��ȡ�ļ�û�г��ȫ��ҳ��. */
#define FSCRT_ERRCODE_EXTRACT_NOT_COMPLETE			-101


/** @brief	ARGB color type, 32 bits, ((b) | ((g) << 8) | ((r) << 16)) | ((a) << 24) */
typedef unsigned int			FA_ARGB;

typedef struct _FSADK_RECTF
{
	/** @brief	Left coordinate. */
	float		left;
	/** @brief	Top coordinate. */
	float		top;
	/** @brief	Right coordinate. */
	float		right;
	/** @brief	Bottom coordinate. */
	float		bottom;
} FSADK_RECTF;

#ifdef __cplusplus
};
#endif

#endif//__FOXIT_FA_BASE_H__


