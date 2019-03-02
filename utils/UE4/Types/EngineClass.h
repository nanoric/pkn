#pragma once
#include <stdint.h>


namespace UE4
{

#pragma pack(push, 8)
class FNameEntry
{
public:
    int Index;
    FNameEntry* HashNext;
    union
    {
        char AnsiName[1024];
        wchar_t WideName[1024];
    };
};

template<typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
class TStaticIndirectArrayThreadSafeRead
{
public:
    constexpr static const int ElementsPerChunk = ElementsPerChunk;
    constexpr static const int ChunkTableSize = (MaxTotalElements + ElementsPerChunk - 1) / ElementsPerChunk;
public:
    //ElementType** Chunks[ChunkTableSize];
    rptr_t Chunks[ChunkTableSize];
    int NumElements;
    int NumChunks;
};

using TNameEntryArray = TStaticIndirectArrayThreadSafeRead<FNameEntry, 2097152, 16384>;
static_assert(sizeof(TNameEntryArray) == 0x408);

struct FName
{
    union
    {
        struct
        {
            int32_t ComparisonIndex;
            int32_t Number;
        };

        uint64_t CompositeComparisonValue;
    };
};

template<class T>
struct TArray
{
public:
    T* Data;
    int32_t Count;
    int32_t Max;
};
struct FString : public TArray<wchar_t>
{
};

enum EObjectFlags
{
    RF_Public = 0x1,
    RF_Standalone = 0x2,
    RF_Native = 0x4,
    RF_Transactional = 0x8,
    RF_ClassDefaultObject = 0x10,
    RF_ArchetypeObject = 0x20,
    RF_Transient = 0x40,
    RF_RootSet = 0x80,
    RF_Unreachable = 0x100,
    RF_TagGarbageTemp = 0x200,
    RF_NeedLoad = 0x400,
    RF_AsyncLoading = 0x800,
    RF_NeedPostLoad = 0x1000,
    RF_NeedPostLoadSubobjects = 0x2000,
    RF_PendingKill = 0x4000,
    RF_BeginDestroyed = 0x8000,
    RF_FinishDestroyed = 0x10000,
    RF_BeingRegenerated = 0x20000,
    RF_DefaultSubObject = 0x40000,
    RF_WasLoaded = 0x80000,
    RF_TextExportTransient = 0x100000,
    RF_LoadCompleted = 0x200000,
    RF_WhiteListed = 0x400000,
    RF_AsyncLoadingRef = 0x800000,
    RF_MarkedByCooker = 0x1000000,
    RF_ForceTagExp = 0x2000000,
    RF_OlderObject = 0x4000000,
    RF_AllFlags = 0x7FFFFFF,
    RF_NoFlags = 0x0,
    RF_Load = 0x14003F,
    RF_PropagateToSubObjects = 0x69,
};

struct UClass;
struct UProperty;
struct UClass;
struct UField;
struct UFunction;
struct __declspec(align(8)) UObject
{
    uint64_t vfptr;
    EObjectFlags ObjectFlags;
    int InternalIndex;
    UClass *Class;
    FName Name;
    UObject *Outer;
};


struct  UField : public UObject
{
    UField *Next;
};
static_assert(sizeof(UField) == 0x30);

struct  UStruct : public UField
{
    UStruct *SuperStruct;
    UField *Children;
    int PropertiesSize;
    TArray<unsigned char> Script;
    int MinAlignment;
    UProperty *PropertyLink;
    UProperty *RefLink;
    UProperty *DestructorLink;
    UProperty *PostConstructLink;
    TArray<UObject *> ScriptObjectReferences;
};
static_assert(sizeof(UStruct) == 0x90);

#pragma pack(push, 4)
struct UProperty : UField
{
    int ArrayDim;
    int ElementSize;
    unsigned __int64 PropertyFlags;
    unsigned __int16 RepIndex;
    FName RepNotifyFunc;
    int Offset_Internal;
    UProperty *PropertyLinkNext;
    UProperty *NextRef;
    UProperty *DestructorLinkNext;
    UProperty *PostConstructLinkNext;
};
#pragma pack(pop)

static_assert(sizeof(UProperty) == 0x70);



struct UFunction : public UStruct
{
    unsigned int FunctionFlags;
    unsigned __int16 RepOffset;
    char NumParms;
    unsigned __int16 ParmsSize;
    unsigned __int16 ReturnValueOffset;
    unsigned __int16 RPCId;
    unsigned __int16 RPCResponseId;
    UProperty *FirstPropertyToInit;
    void(__fastcall *Func)(UObject *_this, void *, void *const);
};

struct FGCReferenceTokenStream
{
    TArray<unsigned int> Tokens;
};
struct FNativeFunctionLookup
{
    FName Name;
    void(__fastcall *Pointer)(UObject *_this, void *, void *const);
};

struct  UClass : public UStruct
{
    void(__fastcall *ClassConstructor)(void *);
    void(__fastcall *ClassAddReferencedObjects)(UObject *, void *);
    unsigned int ClassFlags;
    unsigned __int64 ClassCastFlags;
    int ClassUnique;
    UClass *ClassWithin;
    UObject *ClassGeneratedBy;
    bool bIsGameClass;
    FName ClassConfigName;
    //TArray<FRepRecord> ClassReps;
    //TArray<UField *> NetFields;
    //UObject *ClassDefaultObject;
    //bool bCooked;
    //TMap<FName, UFunction *> FuncMap;
    //TArray<FImplementedInterface> Interfaces;
    //FGCReferenceTokenStream ReferenceTokenStream;
    //TArray<FNativeFunctionLookup> NativeFunctionLookupTable;
};
struct FQuat
{
    float X;
    float Y;
    float Z;
    float W;
};
struct FRotator
{
    float Pitch;
    float Yaw;
    float Roll;
};

struct FVector
{
    float X;
    float Y;
    float Z;
};

}

#pragma pack(pop)
