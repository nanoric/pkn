/**
*
* WOW64Ext Library
*
* Copyright (c) 2014 ReWolf
* http://blog.rewolf.pl/
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#pragma once

#ifndef STATUS_SUCCESS
#   define STATUS_SUCCESS 0
#endif

#pragma warning(push)
#pragma warning(disable : 4201 )

namespace pkn
{
#pragma pack(push)
#pragma pack(1)

    template <class T>
    struct _LIST_ENTRY_T
    {
        T Flink;
        T Blink;
    };

    template <class T>
    struct _UNICODE_STRING_T
    {
        union
        {
            struct
            {
                uint16_t Length;
                uint16_t MaximumLength;
            };
            T dummy;
        };
        T Buffer;
    };

    template <class T>
    struct _NT_TIB_T
    {
        T ExceptionList;
        T StackBase;
        T StackLimit;
        T SubSystemTib;
        T FiberData;
        T ArbitraryUserPointer;
        T Self;
    };

    template <class T>
    struct _CLIENT_ID
    {
        T UniqueProcess;
        T UniqueThread;
    };

    template <class T>
    struct _TEB_T_
    {
        _NT_TIB_T<T> NtTib;
        T EnvironmentPointer;
        _CLIENT_ID<T> ClientId;
        T ActiveRpcHandle;
        T ThreadLocalStoragePointer;
        T ProcessEnvironmentBlock;
        uint32_t LastErrorValue;
        uint32_t CountOfOwnedCriticalSections;
        T CsrClientThread;
        T Win32ThreadInfo;
        uint32_t User32Reserved[26];
        //rest of the structure is not defined for now, as it is not needed
    };

    template <class T>
    struct _LDR_DATA_TABLE_ENTRY_T
    {
        _LIST_ENTRY_T<T> InLoadOrderLinks;
        _LIST_ENTRY_T<T> InMemoryOrderLinks;
        _LIST_ENTRY_T<T> InInitializationOrderLinks;
        T DllBase;
        T EntryPoint;
        union
        {
            uint32_t SizeOfImage;
            T dummy01;
        };
        _UNICODE_STRING_T<T> FullDllName;
        _UNICODE_STRING_T<T> BaseDllName;
        uint32_t Flags;
        uint16_t LoadCount;
        uint16_t TlsIndex;
        union
        {
            _LIST_ENTRY_T<T> HashLinks;
            struct
            {
                T SectionPointer;
                T CheckSum;
            };
        };
        union
        {
            T LoadedImports;
            uint32_t TimeDateStamp;
        };
        T EntryPointActivationContext;
        T PatchInformation;
        _LIST_ENTRY_T<T> ForwarderLinks;
        _LIST_ENTRY_T<T> ServiceTagLinks;
        _LIST_ENTRY_T<T> StaticLinks;
        T ContextInformation;
        T OriginalBase;
        _LARGE_INTEGER LoadTime;
    };

    template <class T>
    struct _PEB_LDR_DATA_T
    {
        uint32_t Length;
        uint32_t Initialized;
        T SsHandle;
        _LIST_ENTRY_T<T> InLoadOrderModuleList;
        _LIST_ENTRY_T<T> InMemoryOrderModuleList;
        _LIST_ENTRY_T<T> InInitializationOrderModuleList;
        T EntryInProgress;
        uint32_t ShutdownInProgress;
        T ShutdownThreadId;

    };

    template <class T, class NGF, int A>
    struct _PEB_T
    {
        union
        {
            struct
            {
                uint8_t InheritedAddressSpace;
                uint8_t ReadImageFileExecOptions;
                uint8_t BeingDebugged;
                uint8_t BitField;
            };
            T dummy01;
        };
        T Mutant;
        T ImageBaseAddress;
        T Ldr;
        T ProcessParameters;
        T SubSystemData;
        T ProcessHeap;
        T FastPebLock;
        T AtlThunkSListPtr;
        T IFEOKey;
        T CrossProcessFlags;
        T UserSharedInfoPtr;
        uint32_t SystemReserved;
        uint32_t AtlThunkSListPtr32;
        T ApiSetMap;
        T TlsExpansionCounter;
        T TlsBitmap;
        uint32_t TlsBitmapBits[2];
        T ReadOnlySharedMemoryBase;
        T HotpatchInformation;
        T ReadOnlyStaticServerData;
        T AnsiCodePageData;
        T OemCodePageData;
        T UnicodeCaseTableData;
        uint32_t NumberOfProcessors;
        union
        {
            uint32_t NtGlobalFlag;
            NGF dummy02;
        };
        LARGE_INTEGER CriticalSectionTimeout;
        T HeapSegmentReserve;
        T HeapSegmentCommit;
        T HeapDeCommitTotalFreeThreshold;
        T HeapDeCommitFreeBlockThreshold;
        uint32_t NumberOfHeaps;
        uint32_t MaximumNumberOfHeaps;
        T ProcessHeaps;
        T GdiSharedHandleTable;
        T ProcessStarterHelper;
        T GdiDCAttributeList;
        T LoaderLock;
        uint32_t OSMajorVersion;
        uint32_t OSMinorVersion;
        uint16_t OSBuildNumber;
        uint16_t OSCSDVersion;
        uint32_t OSPlatformId;
        uint32_t ImageSubsystem;
        uint32_t ImageSubsystemMajorVersion;
        T ImageSubsystemMinorVersion;
        T ActiveProcessAffinityMask;
        T GdiHandleBuffer[A];
        T PostProcessInitRoutine;
        T TlsExpansionBitmap;
        uint32_t TlsExpansionBitmapBits[32];
        T SessionId;
        ULARGE_INTEGER AppCompatFlags;
        ULARGE_INTEGER AppCompatFlagsUser;
        T pShimData;
        T AppCompatInfo;
        _UNICODE_STRING_T<T> CSDVersion;
        T ActivationContextData;
        T ProcessAssemblyStorageMap;
        T SystemDefaultActivationContextData;
        T SystemAssemblyStorageMap;
        T MinimumStackCommit;
        T FlsCallback;
        _LIST_ENTRY_T<T> FlsListHead;
        T FlsBitmap;
        uint32_t FlsBitmapBits[4];
        T FlsHighIndex;
        T WerRegistrationData;
        T WerShipAssertPtr;
        T pContextData;
        T pImageHeaderHash;
        T TracingFlags;
    };

    typedef _LDR_DATA_TABLE_ENTRY_T<uint32_t> LDR_DATA_TABLE_ENTRY32;
    typedef _LDR_DATA_TABLE_ENTRY_T<uint64_t> LDR_DATA_TABLE_ENTRY64;

    typedef _TEB_T_<uint32_t> TEB32;
    typedef _TEB_T_<uint64_t> TEB64;

    typedef _PEB_LDR_DATA_T<uint32_t> PEB_LDR_DATA32;
    typedef _PEB_LDR_DATA_T<uint64_t> PEB_LDR_DATA64;

    typedef _PEB_T<uint32_t, uint64_t, 34> PEB32;
    typedef _PEB_T<uint64_t, uint32_t, 30> PEB64;

    struct _XSAVE_FORMAT64
    {
        uint16_t ControlWord;
        uint16_t StatusWord;
        uint8_t TagWord;
        uint8_t Reserved1;
        uint16_t ErrorOpcode;
        uint32_t ErrorOffset;
        uint16_t ErrorSelector;
        uint16_t Reserved2;
        uint32_t DataOffset;
        uint16_t DataSelector;
        uint16_t Reserved3;
        uint32_t MxCsr;
        uint32_t MxCsr_Mask;
        _M128A FloatRegisters[8];
        _M128A XmmRegisters[16];
        uint8_t Reserved4[96];
    };

    struct _CONTEXT64
    {
        uint64_t P1Home;
        uint64_t P2Home;
        uint64_t P3Home;
        uint64_t P4Home;
        uint64_t P5Home;
        uint64_t P6Home;
        uint32_t ContextFlags;
        uint32_t MxCsr;
        uint16_t SegCs;
        uint16_t SegDs;
        uint16_t SegEs;
        uint16_t SegFs;
        uint16_t SegGs;
        uint16_t SegSs;
        uint32_t EFlags;
        uint64_t Dr0;
        uint64_t Dr1;
        uint64_t Dr2;
        uint64_t Dr3;
        uint64_t Dr6;
        uint64_t Dr7;
        uint64_t Rax;
        uint64_t Rcx;
        uint64_t Rdx;
        uint64_t Rbx;
        uint64_t Rsp;
        uint64_t Rbp;
        uint64_t Rsi;
        uint64_t Rdi;
        uint64_t R8;
        uint64_t R9;
        uint64_t R10;
        uint64_t R11;
        uint64_t R12;
        uint64_t R13;
        uint64_t R14;
        uint64_t R15;
        uint64_t Rip;
        _XSAVE_FORMAT64 FltSave;
        _M128A Header[2];
        _M128A Legacy[8];
        _M128A Xmm0;
        _M128A Xmm1;
        _M128A Xmm2;
        _M128A Xmm3;
        _M128A Xmm4;
        _M128A Xmm5;
        _M128A Xmm6;
        _M128A Xmm7;
        _M128A Xmm8;
        _M128A Xmm9;
        _M128A Xmm10;
        _M128A Xmm11;
        _M128A Xmm12;
        _M128A Xmm13;
        _M128A Xmm14;
        _M128A Xmm15;
        _M128A VectorRegister[26];
        uint64_t VectorControl;
        uint64_t DebugControl;
        uint64_t LastBranchToRip;
        uint64_t LastBranchFromRip;
        uint64_t LastExceptionToRip;
        uint64_t LastExceptionFromRip;
    };

    // Below defines for .ContextFlags field are taken from WinNT.h
#ifndef CONTEXT_AMD64
#define CONTEXT_AMD64 0x100000
#endif

#define CONTEXT64_CONTROL (CONTEXT_AMD64 | 0x1L)
#define CONTEXT64_INTEGER (CONTEXT_AMD64 | 0x2L)
#define CONTEXT64_SEGMENTS (CONTEXT_AMD64 | 0x4L)
#define CONTEXT64_FLOATING_POINT  (CONTEXT_AMD64 | 0x8L)
#define CONTEXT64_DEBUG_REGISTERS (CONTEXT_AMD64 | 0x10L)
#define CONTEXT64_FULL (CONTEXT64_CONTROL | CONTEXT64_INTEGER | CONTEXT64_FLOATING_POINT)
#define CONTEXT64_ALL (CONTEXT64_CONTROL | CONTEXT64_INTEGER | CONTEXT64_SEGMENTS | CONTEXT64_FLOATING_POINT | CONTEXT64_DEBUG_REGISTERS)
#define CONTEXT64_XSTATE (CONTEXT_AMD64 | 0x20L)

#pragma pack(pop)

}
#pragma warning(pop)
