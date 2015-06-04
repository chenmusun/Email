#ifndef __FOXIT_FA_BASE_H__
#define __FOXIT_FA_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 *所有函数返回值，原有部分保持和GSDK一致外，ADK额外添加部分信息
 */
/** @brief	抽取文件没有抽出全部页面. */
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


