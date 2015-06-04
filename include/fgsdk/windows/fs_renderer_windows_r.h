/**
 * Copyright (C) 2003-2015, Foxit Software Inc..
 * All Rights Reserved.
 *
 * http://www.foxitsoftware.com
 *
 * The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
 * It isn't allowed to distribute any parts of Foxit PDF SDK to any third parties or general public, unless there 
 * is a license agreement between Foxit Software Inc. and customers.
 *
 * @file	fs_renderer_windows_r.h
 * @brief	Header file for \ref FSRENDERER_WINDOWS "Renderer For Windows" module of Foxit PDF SDK.
 * @details	This module defines methods about how to create a rendering engine on Windows. 
 *
 * @note	If you want to purchase Foxit PDF SDK license and use ANY of the following functions, please request for enabling <b>Standard module</b> explicitly.
 */

#ifndef _FSCRT_RENDERER_WINDOWS_R_H_
#define _FSCRT_RENDERER_WINDOWS_R_H_

/**
 * @defgroup	FSRENDERER_WINDOWS Renderer For Windows
 * @ingroup		FSRENDERER
 * @brief		Definitions for a rendering engine on windows.<br>
 *				Definitions and functions in this module are included in fs_renderer_windows_r.h.<br>
 *				Module: Renderer<br>
 *				License Identifier: Renderer/All<br>
 *				Available License Right: Unnecessary
 * @details		This module contains the following features:<br>
 *				<ul>
 *				<li>1. Renderer for windows:
 *					<ul>
 *					<li>a. Create a windows renderer object based upon a HDC object.</li>
 *					<li>b. HDC is a standard graphics object in WIN32 system. This provides a convenient function to call windows applications.</li>
 *					<li>c. Note: to set properties of a renderer object, please refer to functions defined in fs_renderer_r.h.</li>
 *					</ul>
 *				</li>
 *				</ul>
 */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

/******************************************************************************/
/* Windows renderer creation                                                  */
/******************************************************************************/
/**
 * @brief	Create a renderer on a Windows device context for printing.
 *
 * @details	Renderer is a term of graphics engine in Foxit PDF SDK, it provides basic management and drawing operations.<br>
 * 			There are two approaches to use a renderer: one is to draw on a renderer directly, and the other one is to output document contents by using a graphics context.<br>
 * 			PDF module provides a rendering context to output page contents. Please refer to function ::FSPDF_RenderContext_Create.
 * 
 * @param[in]	hDC			Handle to a <b>HDC</b> object which is a windows device.
 * @param[out]	renderer	Pointer to a <b>FSCRT_RENDERER</b> handle that receives the new renderer object.
 *
 * @return 	::FSCRT_ERRCODE_SUCCESS for success.<br>
 *			::FSCRT_ERRCODE_PARAM if the parameter <i>hDC</i> or <i>renderer</i> is <b>NULL</b>.<br>
 * 			::FSCRT_ERRCODE_UNRECOVERABLE if an unrecoverable error occurs.<br>
 * 			::FSCRT_ERRCODE_OUTOFMEMORY if there is not enough memory or invalid memory access.<br>
 * 			::FSCRT_ERRCODE_ERROR if fail to create renderer on the specified parameter <i>hDC</i> for other reasons.<br>
 *			For more error codes, please refer to macro definitions <b>FSCRT_ERRCODE_XXX</b>.
 * 
 * @note	This function can only be used on windows desktop platform and doesn't support Windows Store App.
 *			This function should be used only for printing, and caller should always use function ::FSCRT_Renderer_CreateOnBitmap for viewing the PDF.
 *			Foxit PDF SDK will extend supports on more platform-based native devices.
 *
 * @attention	<b>Thread Safety</b>: this function is thread safe.<br>
 *				<b>OOM Information</b>:<br>
 *				OOM handling is only for mobile platforms, not for server or desktop.<br>
 *				<ul>
 *				<li>This function is long-term recoverable.</li>
 *				<li> <i>renderer</i>: this handle is long-term recoverable.</li>
 *				</ul>
 *				Please refer to the document "Robust PDF Applications with Limited Memory" for more details.
 */
FS_RESULT	FSCRT_Renderer_CreateOnWindowsDC(HDC hDC, FSCRT_RENDERER* renderer);

#ifdef __cplusplus
};
#endif

/**@}*/ /* group FSRENDERER_WINDOWS */

#endif /* _FSCRT_RENDERER_WINDOWS_R_H_ */
