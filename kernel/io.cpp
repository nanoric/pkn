#include <ntdef.h>
#include <ntifs.h>

#include <memory>

#include <pkn/core/marcos/debug_print.h>
#include "io.h"
#include "io_code.h"
#include "mem.h"
#include "ps.h"
#include "system.h"
#include "thread.h"
#include "mouse.h"
#include "protect.h"
#include "unloaded_drivers.h"

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

#define MAKE_COMMENT_IDENT_LOOKS_NICE(...)

#define CHECK_INPUT_LENGTH(ProcName) \
            if (inlength != sizeof(ProcName##Input))\
            {\
                status = STATUS_INVALID_PARAMETER;\
                break;\
            }\
            ProcName##Input * pin = (ProcName##Input *)inbuf

#define CHECK_VARIADIC_INPUT_LENGTH(ProcName) \
            if ((inlength < sizeof(ProcName##InputHead))\
             || (inlength - sizeof(ProcName##InputHead)) % sizeof(ProcName##InputData) != 0)\
            {\
                status = STATUS_INVALID_PARAMETER;\
                break;\
            }\
            ProcName##InputHead * pin = (ProcName##InputHead *)inbuf;\
            ProcName##InputData * pdata = (ProcName##InputData *)((char *)inbuf + sizeof(ProcName##InputHead))

#define CHECK_OUTPUT_LENGTH(ProcName) \
            if (outlength != sizeof(ProcName##Output))\
            {\
                status = STATUS_INVALID_PARAMETER;\
                break;\
            }\
            ProcName##Output * pout = (ProcName##Output *)outbuff

#define CHECK_INPUT_OUTPUT_LENGTH(ProcName) \
    CHECK_INPUT_LENGTH(ProcName);\
    CHECK_OUTPUT_LENGTH(ProcName);

#define XorMemory(start, size, xor_val)  { UINT64 __xor = (xor_val);for(SIZE_T i=0; i + 8 <= (size); i+= 8) {*(UINT64 *)((char *)(start) + i) ^= (__xor);}}
#define DecryptInputByXor()  UINT64 __xor_val = pin->xor_val; __xor_val; XorMemory((char *)pin + 8, inlength - 8, __xor_val)/*xor memory besides xor value itself*/
#define EncryptOutputByXor() XorMemory(pout, outlength, __xor_val)

NTSTATUS IoDispatchSlow(
    _In_ struct _DEVICE_OBJECT *DeviceObject,
    _Inout_ struct _IRP *Irp
)
{
    NTSTATUS				status = STATUS_SUCCESS;
    UINT64					bytesIO = 0;
    PIO_STACK_LOCATION		stack;
    BOOLEAN					False = FALSE;

    UNREFERENCED_PARAMETER(DeviceObject);
    stack = IoGetCurrentIrpStackLocation(Irp);

    void *inbuf = Irp->AssociatedIrp.SystemBuffer;
    void *outbuff = Irp->AssociatedIrp.SystemBuffer;
    UINT64 inlength = stack->Parameters.DeviceIoControl.InputBufferLength;
    UINT64 outlength = stack->Parameters.DeviceIoControl.OutputBufferLength;

    do
    {

        if (stack == NULL)
        {
            status = STATUS_INTERNAL_ERROR;
            break;
        }

        switch (stack->Parameters.DeviceIoControl.IoControlCode)
        {
        //case IOCTL_PLAYERKNOWNS_READ_PROCESS_MEMORIES:
        //{
        //    CHECK_VARIADIC_INPUT_LENGTH(ReadProcessMemories);
        //    DecryptInputByXor();
        //    status = read_process_memories(pin->processid, pin->count, pdata);
        //    bytesIO = 0;
        //    break;
        //}
        case IOCTL_PLAYERKNOWNS_WRITE_PROCESS_MEMORY:
        {
            CHECK_INPUT_LENGTH(WriteProcessMemory);
            DecryptInputByXor();
            // !!!!!!note: write process memory is not xor protected!!!!!
            status = write_process_memory(pin->processid, pin->startaddress, pin->bytestowrite, pin->buffer);
            bytesIO = 0;
            break;
        }

        // system memory
        case IOCTL_PLAYERKNOWNS_ALLOCATE_NONPAGED_MEMORY:
        {
            CHECK_INPUT_OUTPUT_LENGTH(AllocateNonpagedMemory);
            DecryptInputByXor();
            auto size = pin->size;
            if (auto res = allocate_nonpaged_memory(size); res)
            {
                auto addr = (uint64_t)*res;
                pout->address = addr;
                DebugPrint("Nonpaged pool allocated at %llx, size %llx", addr, size);
                bytesIO = sizeof(*pout);
                status = STATUS_SUCCESS;
                EncryptOutputByXor();
                break;
            }
            status = STATUS_UNSUCCESSFUL;
            break;
        }

        case IOCTL_PLAYERKNOWNS_READ_SYSTEM_MEMORY:
        {
            CHECK_INPUT_OUTPUT_LENGTH(ReadSystemMemory);
            DecryptInputByXor();
            SIZE_T ncopy = copy_virtual_memory((void *)pin->buffer, (void *)pin->addresstoread, pin->bytestoread);
            DebugPrint("read system memory returns : %llx, ncopy: %llx", status, ncopy);
            XorMemory(pin->buffer, pin->bytestoread, __xor_val);
            if (ncopy)
            {
                pout->bytesread = ncopy;
                bytesIO = sizeof(*pout);
                status = STATUS_SUCCESS;
                EncryptOutputByXor();
                break;
            }
            bytesIO = 0;
            break;
        }
        case IOCTL_PLAYERKNOWNS_WRITE_SYSTEM_MEMORY:
        {
            CHECK_INPUT_OUTPUT_LENGTH(WriteSystemMemory);
            DecryptInputByXor();
            // !!!!!!note: write system memory is not xor protected!!!!!
            DebugPrint("write system memory at : %llx, size: %llx", pin->addresstowrite, pin->bytestowrite);
            auto ncopy = copy_virtual_memory((void *)pin->addresstowrite, (void *)pin->buffer, pin->bytestowrite);
            pout->byteswritten = ncopy;
            bytesIO = sizeof(*pout);
            status = STATUS_SUCCESS;
            EncryptOutputByXor();
            break;
        }

        // virtual memory
        case IOCTL_PLAYERKNOWNS_QUERY_VIRTUAL_MEMORY:
        {
            CHECK_INPUT_OUTPUT_LENGTH(QueryVirtualMemory);
            DecryptInputByXor();
            status = query_virtual_memory(pin->processid, pin->address, &pout->mbi);
            if (NT_SUCCESS(status))
            {
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }
        case IOCTL_PLAYERKNOWNS_GET_MAPPED_FILE:
        {
            CHECK_INPUT_OUTPUT_LENGTH(GetMappedFile);
            DecryptInputByXor();
            status = get_mapped_file(pin->processid, pin->address, pout->image_path, sizeof(pout->image_path));
            if (NT_SUCCESS(status))
            {
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }

        case IOCTL_PLAYERKNOWNS_GET_PROCESS_BASE:
        {
            CHECK_INPUT_OUTPUT_LENGTH(GetProcessBase);
            DecryptInputByXor();
            status = get_process_base(pin->processid, &pout->base);
            if (NT_SUCCESS(status))
            {
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }

        // process
        case IOCTL_PLAYERKNOWNS_GET_PROCESS_EXIT_STATUS:
        {
            CHECK_INPUT_OUTPUT_LENGTH(TestProcess);
            DecryptInputByXor();
            status = get_process_exit_status(pin->processid, &pout->status);
            if (NT_SUCCESS(status))
            {
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }
        case IOCTL_PLAYERKNOWNS_WAIT_FOR_PROCESS:
        {
            CHECK_INPUT_OUTPUT_LENGTH(WaitProcess);
            DecryptInputByXor();
            status = wait_for_process(pin->processid, pin->timeout_nanosec, &pout->status);
            if (NT_SUCCESS(status))
            {
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }
        case IOCTL_PLAYERKNOWNS_GET_PROCESS_TIMES:
        {
            CHECK_INPUT_OUTPUT_LENGTH(GetProcessTimes);
            DecryptInputByXor();
            status = get_process_times(pin->processid, &pout->creation_time, &pout->exit_time, &pout->kernel_time, &pout->user_time);
            if (NT_SUCCESS(status))
            {
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }
        case IOCTL_PLAYERKNOWNS_GET_PROCESS_NAME:
        {
            CHECK_INPUT_OUTPUT_LENGTH(GetProcessName);
            DecryptInputByXor();
            status = query_process_name(pin->processid, pout->process_name, sizeof(pout->process_name));
            if (NT_SUCCESS(status))
            {
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }
        case IOCTL_PLAYERKNOWNS_QUERY_PROCESS_INFORMATION:
        {
            CHECK_INPUT_LENGTH(QueryProcessInformation);
            DecryptInputByXor();
            char *pout = (char *)pin;
            ULONG ret_size = 0;
            UINT64 buffer_size = pin->buffer_size;
            buffer_size = min(buffer_size, outlength);
            status = query_process_information(pin->id, pin->information_class, pout, buffer_size, &ret_size);
            if (NT_SUCCESS(status))
            {
                bytesIO = ret_size;
                EncryptOutputByXor();
            }
            break;
        }
        case IOCTL_PLAYERKNOWNS_CREATE_USER_THREAD:
        {
            CHECK_INPUT_OUTPUT_LENGTH(CreateUserThread);
            DecryptInputByXor();
            CLIENT_ID ids;
            status = create_user_thread(pin->processid,
                                        pin->has_sd ? &pin->sd : nullptr,
                                        pin->create_suspended,
                                        pin->maximun_stack_size,
                                        pin->commited_stack_size,
                                        (PVOID)pin->start_address,
                                        (PVOID)pin->parameter,
                                        &ids
            );
            if (NT_SUCCESS(status))
            {
                pout->pid = (UINT64)ids.UniqueProcess;
                pout->tid = (UINT64)ids.UniqueThread;
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }
        case IOCTL_PLAYERKNOWNS_ALLOCATE_VIRTUAL_MEMORY:
        {
            CHECK_INPUT_OUTPUT_LENGTH(AllocateVirtualMemory);
            DecryptInputByXor();
            PVOID address = (PVOID)pin->address;
            SIZE_T size = pin->size;
            status = allocate_virtual_memory(pin->processid, &address, &size, pin->type, pin->protect);
            if (NT_SUCCESS(status))
            {
                pout->address = (UINT64)address;
                pout->size = size;
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }
        case IOCTL_PLAYERKNOWNS_FREE_VIRTUAL_MEMORY:
        {
            CHECK_INPUT_OUTPUT_LENGTH(FreeVirtualMemory);
            DecryptInputByXor();
            PVOID address = (PVOID)pin->address;
            SIZE_T size = pin->size;
            status = free_virtual_memory(pin->processid, &address, &size, pin->type);
            if (NT_SUCCESS(status))
            {
                pout->address = (UINT64)address;
                pout->size = size;
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }
        case IOCTL_PLAYERKNOWNS_PROTECT_VIRTUAL_MEMORY:
        {
            CHECK_INPUT_OUTPUT_LENGTH(ProtectVirtualMemory);
            DecryptInputByXor();
            PVOID address = (PVOID)pin->address;
            SIZE_T size = pin->size;
            status = protect_virtual_memory(pin->processid, &address, &size, pin->protect, &pout->old_protect);
            if (NT_SUCCESS(status))
            {
                pout->address = (UINT64)address;
                pout->size = size;
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }

        // thread
        case IOCTL_PLAYERKNOWNS_QUERY_THREAD_INFORMATION:
        {
            CHECK_INPUT_LENGTH(QueryThreadInformation);
            DecryptInputByXor();
            auto pout = pin;
            if (pin->size_only)
            {
                status = query_thread_information(
                    pin->id,
                    pin->information_class,
                    nullptr,
                    0,
                    reinterpret_cast<ULONG*>(pout)
                );
                if (status == STATUS_INFO_LENGTH_MISMATCH)
                {
                    bytesIO = sizeof(ULONG);
                    status = STATUS_SUCCESS;
                    break;
                }
            }
            else
            {
                ULONG out_size = pin->buffer_size;
                status = query_thread_information(
                    pin->id,
                    pin->information_class,
                    pout,
                    pin->buffer_size,
                    &out_size);
                if (NT_SUCCESS(status))
                {
                    EncryptOutputByXor();
                    bytesIO = out_size;
                    break;
                }
            }
            break;
        }
        case IOCTL_PLAYERKNOWNS_WAIT_FOR_THREAD:
        {
            CHECK_INPUT_OUTPUT_LENGTH(WaitThread);
            DecryptInputByXor();
            status = wait_for_thread(pin->tid, pin->timeout_nanosec, &pout->status);
            if (NT_SUCCESS(status))
            {
                bytesIO = sizeof(*pout);
                EncryptOutputByXor();
            }
            break;
        }
        /*
        // failed, maybe calling wrong ntoskrnl function
        case IOCTL_PLAYERKNOWNS_SET_THREAD_CONTEXT:
            {
                CHECK_INPUT_LENGTH(SetThreadContext);
                status = set_thread_context(pin->tid, pin->context);
                bytesIO = 0;
                break;
            }
            case IOCTL_PLAYERKNOWNS_GET_THREAD_CONTEXT:
            {
                CHECK_INPUT_OUTPUT_LENGTH(GetThreadContext);
                status = get_thread_context(pin->tid, &pout->context);
                bytesIO = sizeof(*pout);
                break;
            }
            case IOCTL_PLAYERKNOWNS_SUSPEND_THREAD:
            {
                CHECK_INPUT_OUTPUT_LENGTH(SuspendThread);
                status = suspend_thread(pin->tid, &pout->previois_suspend_count);
                bytesIO = sizeof(*pout);
                break;
            }
            case IOCTL_PLAYERKNOWNS_RESUME_THREAD:
            {
                CHECK_INPUT_OUTPUT_LENGTH(ResumeThread);
                status = suspend_thread(pin->tid, &pout->previois_suspend_count);
                bytesIO = sizeof(*pout);
                break;
            }*/

            // system
        case IOCTL_PLAYERKNOWNS_QUERY_SYSTEM_INFORMATION:
        {
            CHECK_INPUT_LENGTH(QuerySystemInformation);
            DecryptInputByXor();
            auto pout = pin;
            if (pin->size_only)
            {
                status = query_system_information(
                    pin->information_class,
                    nullptr,
                    0,
                    reinterpret_cast<ULONG*>(pout)
                );
                if (status == STATUS_INFO_LENGTH_MISMATCH)
                {
                    bytesIO = sizeof(ULONG);
                    status = STATUS_SUCCESS;
                    break;
                }
            }
            else
            {
                ULONG out_size = pin->buffer_size;
                status = query_system_information(
                    pin->information_class,
                    pout,
                    pin->buffer_size,
                    &out_size);
                if (NT_SUCCESS(status))
                {
                    EncryptOutputByXor();
                    bytesIO = out_size;
                    break;
                }
            }
            break;
        }

        // mouse
        case IOCTL_PLAYERKNOWNS_GET_MOUSE_POS:
        {
            CHECK_OUTPUT_LENGTH(GetMousePos);
            get_mouse_pos(&pout->x, &pout->y);
            status = STATUS_SUCCESS;
            bytesIO = sizeof(*pout);
            break;
        }
        case IOCTL_PLAYERKNOWNS_GET_MOUSE_STATE:
        {
            CHECK_INPUT_OUTPUT_LENGTH(GetMouseState);
            pout->pressed = get_mouse_state(pin->key);
            status = STATUS_SUCCESS;
            bytesIO = sizeof(*pout);
            break;
        }
        case IOCTL_PLAYERKNOWNS_SYNTHESIZE_MOUSE:
        {
            CHECK_VARIADIC_INPUT_LENGTH(SynthesizeMouse);
            synthesize_mouse(pdata, pin->count, pin->fill);
            status = STATUS_SUCCESS;
            bytesIO = 0;
            break;
        }

        // process protect
        case IOCTL_PLAYERKNOWNS_PROTECT_PROCESS:
        {
            CHECK_INPUT_LENGTH(ProtectProcess);
            DecryptInputByXor();
            status = protect_process(pin->pid);
            bytesIO = 0;
            break;
        }
        case IOCTL_PLAYERKNOWNS_UNPROTECT_PROCESS:
        {
            status = unprotecct_process();
            bytesIO = 0;
            break;
        }

        // unloaded drivers
        case IOCTL_PLAYERKNOWNS_DELETE_UNLOADED_DRIVERS:
        {
            CHECK_INPUT_OUTPUT_LENGTH(DeleteUnloadedDrivers);
            DecryptInputByXor();
            status = delete_unloaded_drivers(pin->pMmUnloadedDrivers, pin->pMmLastUnloadedDriver, pin->name, &pout->ndelete);
            bytesIO = sizeof(*pout);
            break;
        }

        // run driver entry
        case IOCTL_PLAYERKNOWNS_RUN_DRIVER_ENTRY:
        {
            CHECK_INPUT_OUTPUT_LENGTH(RunDriverEntry);
            DecryptInputByXor();
            using Func = NTSTATUS(*)(UINT64, UINT64);
            auto entry = (Func)pin->start_address;
            auto retv = entry(pin->parameter1, pin->parameter2);
            pout->ret_val = retv;
            status = STATUS_SUCCESS;
            bytesIO = sizeof(*pout);
            EncryptOutputByXor();
            break;
        }

        case IOCTL_PLAYERKNOWNS_MAP_AND_RUN_DRIVER:
        {
            status = STATUS_CANCELLED;
            break;
            //CHECK_INPUT_OUTPUT_LENGTH(MapAndRunDriver);
            //DecryptInputByXor();

            //status = STATUS_UNSUCCESSFUL;

            //void *image_buffer = reinterpret_cast<void *>(pin->image_buffer);
            //size_t image_size = pin->image_size;
            //uint64_t entry_rva = pin->entry_rva;
            //uint64_t p1 = pin->parameter1;
            //uint64_t p2 = pin->parameter2;
            //bool p1_is_rva = pin->parameter1_is_rva;
            //bool p2_is_rva = pin->parameter2_is_rva;

            //PHYSICAL_ADDRESS low, high, skip;
            //low.QuadPart = 0;
            //high.QuadPart = INT64_MAX;
            //skip.QuadPart = 0;
            //PMDL pmdl = MmAllocatePagesForMdl(low, high, skip, image_size);
            //if (nullptr == pmdl)
            //{
            //    DebugPrint("MapAndRunDriver: MmAllocatePagesForMdl failed : return nullptr");
            //    break;
            //}
            //auto ptr = MmGetSystemAddressForMdlSafe(pmdl, NormalPagePriority);
            //ImagePEStructure64 s;
            //if (nullptr == ptr)
            //{
            //    DebugPrint("MapAndRunDriver: MmGetSystemAddressForMdlSafe failed : return nullptr");
            //    break;
            //}
            //if (auto ncopy = copy_virtual_memory(ptr, image_buffer, image_size); ncopy <= image_size)
            //{
            //    DebugPrint("copy memory failed: size copied (%llx) is less than requested (%llx)",
            //               image_size, ncopy);
            //    break;
            //}
            //using Func = UINT64(*)(UINT64, UINT64);
            //auto mapped_entry = (Func)((uint64_t)ptr + entry_rva);
            //if (p1_is_rva)
            //    p1 += (uint64_t)ptr;
            //if (p2_is_rva)
            //    p2 += (uint64_t)ptr;
            //auto retv = mapped_entry(p1, p2);
            //pout->ret_val = retv;
            //status = STATUS_SUCCESS;
            //bytesIO = sizeof(*pout);
            //break;
        }

        case IOCTL_PLAYERKNOWNS_FORCE_WRITE_PROCESS_MEMORY:
        {
            CHECK_INPUT_LENGTH(WriteProcessMemory);
            DecryptInputByXor();
            if (pin->bytestowrite == 1)
            {
                if (force_write_8(pin->processid, (void*)pin->startaddress, *(uint8_t*)pin->buffer))
                    status = STATUS_SUCCESS;
                else
                    status = STATUS_UNSUCCESSFUL;

            }
            else if (pin->bytestowrite == 8)
            {
                if (force_write_64(pin->processid, (void*)pin->startaddress, *(uint64_t*)pin->buffer))
                    status = STATUS_SUCCESS;
                else
                    status = STATUS_UNSUCCESSFUL;
            }
            else if (pin->bytestowrite <= 0x1000)
            {
                std::unique_ptr<char> buffer(new char[0x1000]);
                auto p = buffer.get();
                memcpy(p, (void *)pin->buffer, pin->bytestowrite);
                if (force_write(pin->processid, (void*)pin->startaddress, p, pin->bytestowrite))
                    status = STATUS_SUCCESS;
                else
                    status = STATUS_UNSUCCESSFUL;
            }
            else
            {
                status = STATUS_INVALID_PARAMETER;
            }
            bytesIO = 0;
            break;
        }
        // default
        default:
            status = STATUS_INVALID_PARAMETER;
        };

    } while (False);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = bytesIO;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS IoDispatch(
    _In_ struct _DEVICE_OBJECT *DeviceObject,
    _Inout_ struct _IRP *Irp
)
{
    NTSTATUS				status = STATUS_UNSUCCESSFUL;
    UINT64					bytesIO = 0;
    PIO_STACK_LOCATION		stack;

    UNREFERENCED_PARAMETER(DeviceObject);
    stack = IoGetCurrentIrpStackLocation(Irp);

    void *inbuf = Irp->AssociatedIrp.SystemBuffer;
    void *outbuff = Irp->AssociatedIrp.SystemBuffer;
    auto &inlength = stack->Parameters.DeviceIoControl.InputBufferLength;
    auto &outlength = stack->Parameters.DeviceIoControl.OutputBufferLength;

    if (stack != NULL)
    {

        switch (stack->Parameters.DeviceIoControl.IoControlCode)
        {
            // read process  memory
        case IOCTL_PLAYERKNOWNS_READ_PROCESS_MEMORY:
        {

            CHECK_INPUT_LENGTH(ReadProcessMemory);
            DecryptInputByXor();
            status = read_process_memory(pin->processid, pin->startaddress, pin->bytestoread, pin->buffer);
            XorMemory(pin->buffer, pin->bytestoread, __xor_val);
            bytesIO = 0;
            break;
        }

        // acquire a spin lock
        case IOCTL_PLAYERKNOWNS_ACQUIRE_LOCK:
        {
            CHECK_INPUT_OUTPUT_LENGTH(AcquireLock);
            DecryptInputByXor();
            pout->succeed = acquire_lock(pin->processid, pin->remote_lock_address);
            status = STATUS_SUCCESS;
            bytesIO = sizeof(*pout);
            break;
        }
        default:
            return IoDispatchSlow(DeviceObject, Irp);
        }
    }
    else
        status = STATUS_INTERNAL_ERROR;
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = bytesIO;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}