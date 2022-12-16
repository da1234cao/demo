#pragma once

/* modified from : aurain, https://www.cnblogs.com/dhf327/p/4793674.html  */

#include <ntddk.h>

/**/
/** * ��������ļ�
* @param lpFileHandle ���ش򿪵��ļ����ָ��
* @param usFileName ��Ҫ�򿪵��ļ�·����ʹ�ö���·������\\??\\c:\test.txt
* @param dwDesiredAccess ����Ȩ�ޣ�������|����������²���
    д�ļ�����-FILE_WRITE_DATA��
    �����ļ�����-FILE_WRITE_ATTRIBUTES��ͨ��д-GENERIC_WRITE ���ļ�����-FILE_READ_DATA��
    �����ļ�����-FILE_READE_ATTRIBUTES��ͨ��д-GENERIC_READ ɾ���ļ�-DELETE ȫ��Ȩ��-GENERIC_ALL ͬ�����ļ�-SYNCHRONIZE
* @param dwShareAccess ����ʽ����ָ�����������ļ�ʱ�������Ĵ���ͬʱ������ļ������е�Ȩ�� ������|����������²���
    �����-FILE_SHARE_READ
    ����д-FILE_SHARE_WRITE
    ����ɾ��-FILE_SHARE_DELETE
* @param dwCreateDisposition ��������ļ���Ŀ��
    �½��ļ�-FILE_CREATE
    ���ļ�-FILE_OPEN
    �򿪻��½�-FILE_OPEN_IF
    ����-FILE_OVERWRITE
    �½��򸲸�-FILE_OVERWRITE_IF
    �½���ȡ��-FILE_SUPERSEDE
* @param dwCreateOptions ���ļ�ʱѡ������ һ����FILE_NOT_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT

* @return ��ȡ�ɹ��� STATUS_SUCCESS����ȡʧ�ܣ�NTSTATUS error code */

__inline NTSTATUS CreateFile(OUT PHANDLE lpFileHandle,
  IN PUNICODE_STRING usFileName,
  IN ULONG dwDesiredAccess,
  IN ULONG dwShareAccess,
  IN ULONG dwCreateDisposition,
  IN ULONG dwCreateOptions) {

  NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
  OBJECT_ATTRIBUTES oaName;
  IO_STATUS_BLOCK iosBlock;
  if (lpFileHandle != NULL && usFileName != NULL && usFileName->Buffer != NULL) {
    InitializeObjectAttributes(&oaName, usFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    ntStatus = ZwCreateFile(lpFileHandle, dwDesiredAccess, &oaName, &iosBlock, NULL, FILE_ATTRIBUTE_NORMAL, dwShareAccess, dwCreateDisposition, dwCreateOptions, NULL, 0);
  }
  return ntStatus;
}


/**/
/** * �رմ򿪵��ļ����
* @param hFile �ļ����
* @return ��ȡ�ɹ��� STATUS_SUCCESS����ȡʧ�ܣ�NTSTATUS error code
*/

__inline NTSTATUS CloseFile(IN HANDLE hFile) {
  return ZwClose(hFile);
}

/**/

/** * ��ȡ�ļ�����
* @param hFile �ļ����
* @param pBuffer ������
* @param ulBufferSize ��������С
* @param byteOffset ƫ����
* @param pulBytesRead ʵ�ʶ�ȡ�Ĵ�С
* @return ��ȡ�ɹ��� STATUS_SUCCESS����ȡʧ�ܣ�NTSTATUS error code
*/

__inline NTSTATUS ReadFile(IN HANDLE hFile, IN PVOID pBuffer, IN ULONG ulBufferSize, IN PLARGE_INTEGER byteOffset, OUT PULONG pulBytesRead) {
  IO_STATUS_BLOCK  iosBlock; 
  NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
  if (hFile == NULL || pBuffer == NULL) {
    return ntStatus;
  }
  *pulBytesRead = 0;
  ntStatus = ZwReadFile(hFile, NULL, NULL, NULL, &iosBlock, pBuffer, ulBufferSize, byteOffset, NULL);
  if (NT_SUCCESS(ntStatus)) {
    //��ȡʵ�ʶ�ȡ���Ĵ�С
    *pulBytesRead = (ULONG)iosBlock.Information;
  }
  return ntStatus;
}

/**/
/** * ���ļ�д������
* @param hFile �ļ����
* @param pBuffer ������
* @param ulBufferSize ��������С
* @param byteOffset ƫ����
* @return ��ȡ�ɹ��� STATUS_SUCCESS����ȡʧ�ܣ�NTSTATUS error code
*/
__inline NTSTATUS WriteFile(IN HANDLE hFile, IN PVOID pBuffer, IN ULONG ulBufferSize, IN PLARGE_INTEGER byteOffset, OUT PULONG pulBytesWrite) {
  IO_STATUS_BLOCK iosBlock; 
  NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
  if (hFile == NULL || pBuffer == NULL) {
    return ntStatus;
  }
  DbgPrint("before zwwritefile.");
  *pulBytesWrite = 0;
  ntStatus = ZwWriteFile(hFile, NULL, NULL, NULL, &iosBlock, pBuffer, ulBufferSize, byteOffset, NULL);
  if (NT_SUCCESS(ntStatus)) {
    *pulBytesWrite = (ULONG)iosBlock.Information;
  }
  return ntStatus;
}