#ifndef __FOXIT_FA_LIBRARY_H__
#define __FOXIT_FA_LIBRARY_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  @return	::FSCRT_ERRCODE_SUCCESS 0 for success.<br>
 *			::FSCRT_ERRCODE_PARAM if the parameter is <b>NULL</b>.<br>
 *			::FSCRT_ERRCODE_ERROR if fail to get the current frame bitmap for any other reasons.<br>
 *			For more error codes, please refer to fgsdk fs_base_r.h macro definitions <b>FSCRT_ERRCODE_XXX</b>.
 */
int			FSADK_InitializeLibrary(char* licenseKey,char* unlockCode);

void		FSADK_FinalizeLibrary();

#ifdef __cplusplus
};
#endif


#endif//__FOXIT_FA_LIBRARY_H__