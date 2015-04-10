/**
 * Copyright (C) 2001-2015, Foxit Software Inc..
 * All Rights Reserved.
 *
 * http://www.foxitsoftware.com
 *
 * The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
 * You cannot distribute any part of Foxit PDF SDK to any third party or general public, 
 * unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
 *
 * This file is a  header to provide functions how to trans pdf to text by examples.
 */
#pragma once
#ifndef _FSDK_PDF2TEXT_H_
#define _FSDK_PDF2TEXT_H_



class CPDFTransToText
{
public:
	CPDFTransToText();
	~CPDFTransToText();

	int		PDFToText(string fileName, string inputFilesPath, string outputFilesPath,string& outputname,int& nPage,int& time);
	void		BrowseFolder(void* folder, string inputFilesPath, string outputFilesPath);

public:
	string m_strSrcFile ;
	string m_strPageRange ;
	string m_strDestFolder ;
};

int PDF2Text(string inputFilePath, string outputFilePath, string& outputname, int& nPageNum, int& time);

#endif
