// FileUtil.h : File Utility function
// 
// 
//

#pragma once

NHS_EXPORT void MyCreateDir(char* Path);
NHS_EXPORT BOOL MyCreateDirectory(LPCTSTR lpszPath);
NHS_EXPORT DWORD MyGetFileSize(CString& strFileName);
NHS_EXPORT int MyAccessFile(CString& strFileName);

NHS_EXPORT HANDLE MyOpenFile(CString& strFile, BOOL bCreate);
NHS_EXPORT HANDLE MyOpenFile2(CString& strFile, int nFlag);
NHS_EXPORT int MySetFilePointer(HANDLE hFile, int nOffset, DWORD dwMoveMethod);
NHS_EXPORT int MyWriteFile(HANDLE hFile, void *pData, int nDataLen);
NHS_EXPORT int MyReadFile(HANDLE hFile, void *pData, int nDataLen);
NHS_EXPORT int MyCloseFile(HANDLE hFile);

