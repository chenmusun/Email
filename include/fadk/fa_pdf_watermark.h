#ifndef __FOXIT_FA_PDF_WATERMARK_H__
#define __FOXIT_FA_PDF_WATERMARK_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _FADK_PDF_OBJECT_INFO
{
	int pageIndex;//页面索引
	int type;//PDF object类型，请参考 fpdf_pageobjects_r.h FSPDF_PAGEOBJECT_XXX定义
	FSADK_RECTF rcObject;
	FSADK_RECTF rcClip;
	void* graphState;//pointer to FSPDF_GRAPHSTATE
	FA_ARGB strokeColor;
	FA_ARGB fillColor;
	bool hasTransparency;
	void*	textState;// pointer to FSPDF_TEXTSTATE
	char*	textContent; //UTF-8编码
	int		textContentLength;
} FADK_PDF_OBJECT_INFO;

/**
 * @brief	Structure for watermark object to delete or not.
 */
typedef struct _FADK_WATERMARK_HANDLER
{
	/**
	 * @brief	User-defined data.
	 * @note	Callers can use this field to track controls.
	 */
	void*	clientData;

	/**
	 * @brief	Callback function to determinate PDF object or not
	 *
	 * @param[in]	clientData	Pointer to user-defined data.
	 * @param[in]	objectInfo	Pointer to PDF object information.
	 *
	 * @return	true to delete object,false means not to delete
	 */
	bool	(*DeletePDFObject)(void* clientData,FADK_PDF_OBJECT_INFO* objectInfo);
} FADK_WATERMARK_HANDLER;

/*
 *  @return	::FSCRT_ERRCODE_SUCCESS 0 for success.<br>
 *			::FSCRT_ERRCODE_PARAM if the parameter is <b>NULL</b>.<br>
 *			::FSCRT_ERRCODE_ERROR if fail to get the current frame bitmap for any other reasons.<br>
 *			For more error codes, please refer to fgsdk fs_base_r.h macro definitions <b>FSCRT_ERRCODE_XXX</b>.
 */
int		FSADK_PDF_RemoveWatermark(FADK_WATERMARK_HANDLER* watermarkHandler,char* outputPDFPath,char* inputPDFPath,char* inputPDFPassword=NULL);


#ifdef __cplusplus
};
#endif

#endif//__FOXIT_FA_PDF_WATERMARK_H__


