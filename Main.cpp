#include "ntifs.h"

typedef NTSTATUS(*IRP_MJ_SERIES)
(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP pIrp
	);

IRP_MJ_SERIES g_OriFunc = NULL;
PDEVICE_OBJECT g_pDeviceObject = NULL;

NTSTATUS
DefaultDispatch(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp)
{
	//��������
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(pIrp);

	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}

VOID
DriverUnload(
	PDRIVER_OBJECT pDriverObject)
{
	//��������
	UNREFERENCED_PARAMETER(pDriverObject);

	KdPrint(("[DiskSector ����:caizhe666 ��������:%s] [%S] ������ж��!\n", __DATE__, __FUNCTIONW__));

	InterlockedExchange64((PLONG64)(&g_pDeviceObject->DriverObject->MajorFunction[IRP_MJ_WRITE]), (LONG64)g_OriFunc);
	ObDereferenceObject(g_pDeviceObject);

	return;
}

NTSTATUS
CurrentFunc(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp)
{
	NTSTATUS Status = STATUS_SUCCESS;

	__try {
		PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(pIrp);
		if (!IrpSp)
		{
			KdPrint(("[%S] ��Ч�Ĳ���:IrpSp!\n", __FUNCTIONW__));
			pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return pIrp->IoStatus.Status;
		}
		PEPROCESS pEProc = IoThreadToProcess(pIrp->Tail.Overlay.Thread);
		if (!pEProc)
		{
			KdPrint(("[%S] ��Ч�Ĳ���:pEProc!\n", __FUNCTIONW__));
			pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return pIrp->IoStatus.Status;
		}
		HANDLE hProc = PsGetCurrentProcessId();
		PUNICODE_STRING puniProcImageName = NULL;

		Status = SeLocateProcessImageName(pEProc, &puniProcImageName);
		if (!NT_SUCCESS(Status))
		{
			KdPrint(("[%S] SeLocateProcessImageNameʧ��! ������:%X\n", __FUNCTIONW__, Status));
			pIrp->IoStatus.Status = Status;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return pIrp->IoStatus.Status;
		}

		UINT64 Sector = (UINT64)IrpSp->Parameters.Write.ByteOffset.QuadPart / 512;

		if (hProc == (HANDLE)4 || hProc == (HANDLE)0)
			KdPrint(("[%S] ProcPath:System ,Sector:%I64u ,ProcId:%I64u ,ProcAddr:%p\n",
				__FUNCTIONW__,
				Sector,
				(UINT64)hProc,
				pEProc
				));
		else
			KdPrint(("[%S] ProcPath:%wZ ,Sector:%I64u ,ProcId:%I64u ,ProcAddr:%p\n",
				__FUNCTIONW__,
				puniProcImageName,
				Sector,
				(UINT64)hProc,
				pEProc
				));

		if (Sector == 0)
		{
			KdPrint(("[%S] ������ͼ��дMBR,�Ѿܾ�!\n", __FUNCTIONW__));
			pIrp->IoStatus.Status = STATUS_ACCESS_DENIED;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return pIrp->IoStatus.Status;
		}

		return g_OriFunc(pDeviceObject, pIrp);
	}
	__except (1)
	{
		KdPrint(("[%S] δ֪����,������:%X\n", __FUNCTIONW__, GetExceptionCode()));
	}
	pIrp->IoStatus.Status = STATUS_ACCESS_DENIED;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}

#ifdef __cplusplus
EXTERN_C
#endif
NTSTATUS
DriverEntry(
	PDRIVER_OBJECT pDriverObject,
	PUNICODE_STRING pRegistryPath)
{
	//��������
	UNREFERENCED_PARAMETER(pRegistryPath);

	KdPrint(("[DiskSector ����:caizhe666 ��������:%s] [%S] �����Ѽ���!\n", __DATE__, __FUNCTIONW__));

	//У�����
	if (!pDriverObject)
	{
		KdPrint(("[%S] ��Ч�Ĳ���:pDriverObject!\n", __FUNCTIONW__));
		return STATUS_INVALID_PARAMETER;
	}

	NTSTATUS Status = STATUS_SUCCESS;

	for (USHORT i = 0; i < IRP_MJ_MAXIMUM_FUNCTION + 1; i++)
		pDriverObject->MajorFunction[i] = DefaultDispatch;

	pDriverObject->DriverUnload = DriverUnload;

	UNICODE_STRING uniDriveName = RTL_CONSTANT_STRING(L"\\Device\\Harddisk0\\DR0");
	PFILE_OBJECT pFileObject = NULL;
	Status = IoGetDeviceObjectPointer(&uniDriveName, OBJ_OPENIF, &pFileObject, &g_pDeviceObject);
	if (NT_SUCCESS(Status))
	{
		g_OriFunc = (IRP_MJ_SERIES)InterlockedExchange64((PLONG64)(&g_pDeviceObject->DriverObject->MajorFunction[IRP_MJ_WRITE]), (LONG64)CurrentFunc);
		ObDereferenceObject(pFileObject);
		KdPrint(("[%S] Hook�ɹ�!\n", __FUNCTIONW__));
	}
	else
		KdPrint(("[%S] Hookʧ��! ������:%X\n", __FUNCTIONW__, Status));

	return Status;
}