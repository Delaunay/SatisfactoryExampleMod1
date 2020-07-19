// ILikeBanas

#include "MACharacterPlayer.h"

// Original Class
#include "FGCharacterPlayer.h"

// SML
#include "mod/hooking.h"
#include "util/Logging.h"

#include "util/funchook.h"
// std::abort
#include <cstdlib>
#include <xkeycheck.h>

#include "util/funchook.h"
#include "util/Internal.h"
#include "SatisfactoryModLoader.h"
#include "CoreMinimal.h"
#include "util/bootstrapper_exports.h"


AFGCharacterPlayerExt::AFGCharacterPlayerExt()
{
}

AFGCharacterPlayerExt::~AFGCharacterPlayerExt()
{
}


//*
template <typename TCallable, TCallable Callable, typename TargetClass>
struct MethodTrait;


template <typename R, typename C, typename... A, R(C::*PMF)(A...), typename TargetClass>
struct MethodTrait<R(C::*)(A...), PMF, TargetClass> {
    typedef R* StructMethodSignature(C*, R*, A...);
    typedef R* UnionMethodSignature(C*, R*, A...);
    typedef R ScalarMethodSignature(C*, A...);

    typedef void StructHandlerSignature(C const*, R&, A...);
    typedef void UnionHandlerSignature (C const*, R&, A...);
    typedef void ScalarHandlerSignature(C const*, R&, A...);
    typedef void VoidHandlerSignature  (C const*, A...);

    typedef typename std::conditional <std::is_same<R, void>::value, ScalarMethodSignature,
        typename std::conditional<std::is_class<R>::value, StructMethodSignature,
        typename std::conditional<std::is_union<R>::value, UnionMethodSignature, ScalarMethodSignature>::type>::type>::type FunctionType;

    typedef typename std::conditional <std::is_same<R, void>::value, VoidHandlerSignature,
        typename std::conditional<std::is_class<R>::value, StructHandlerSignature,
        typename std::conditional<std::is_union<R>::value, UnionHandlerSignature, ScalarHandlerSignature>::type>::type>::type HandlerType;
};

template <typename R, typename C, typename... A, R(C::*PMF)(A...) const, typename TargetClass>
struct MethodTrait<R(C::*)(A...) const, PMF, TargetClass> {
    typedef R* StructMethodSignature(C const*, R*, A...);
    typedef R* UnionMethodSignature(C const*, R*, A...);
    typedef R  ScalarMethodSignature(C const*, A...);

    typedef void StructHandlerSignature(C const*, R&, A...);
    typedef void UnionHandlerSignature (C const*, R&, A...);
    typedef void ScalarHandlerSignature(C const*, R&, A...);
    typedef void VoidHandlerSignature  (C const*, A...);

    typedef typename std::conditional <std::is_same<R, void>::value, ScalarMethodSignature,
        typename std::conditional<std::is_class<R>::value, StructMethodSignature,
        typename std::conditional<std::is_union<R>::value, UnionMethodSignature, ScalarMethodSignature>::type>::type>::type FunctionType;

    typedef typename std::conditional <std::is_same<R, void>::value, VoidHandlerSignature,
        typename std::conditional<std::is_class<R>::value, StructHandlerSignature,
        typename std::conditional<std::is_union<R>::value, UnionHandlerSignature, ScalarHandlerSignature>::type>::type>::type HandlerType;
};

//*/


// static TArray<AFGEquipment*>(AFGCharacterPlayer::*original)() const = &AFGCharacterPlayer::GetActiveEquipments;
// typedef TArray<AFGEquipment*>*(*Trampoline)(AFGCharacterPlayer const*, TArray<AFGEquipment*>*);


using Trampoline = typename MethodTrait<
    decltype(&AFGCharacterPlayer::GetActiveEquipments), &AFGCharacterPlayer::GetActiveEquipments, None>::FunctionType;

static Trampoline* funptr;



template <typename TCallable, TCallable Callable, typename TargetClass>
class HandlerRegistry;


template <typename R, typename C, typename... A, R(C::*PMF)(A...) const, typename TargetClass>
class HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass> {
public:
    using Trampoline = typename MethodTrait<R(C::*)(A...) const, PMF, TargetClass>::FunctionType;
    using HandlerSignature = typename MethodTrait<R(C::*)(A...) const, PMF, TargetClass>::HandlerType;

    typedef std::function<HandlerSignature> HandlerType;

    static Trampoline*         funptr;
    static bool                bHookInitialized;
    static TArray<HandlerType> handlers;

    static void InstallHook(const FString& SymbolSearchName) {
#if !WITH_EDITOR
        if (!bHookInitialized) {
            auto FunOverride = getApplyCall();
            RegisterHookFunction(SymbolSearchName, (void*)FunOverride, (void**)&funptr);
            bHookInitialized = true;
        }
#endif
    }

    static void AddHandler(HandlerType fun) {
#if !WITH_EDITOR
        handlers.Add(fun);
#endif
    }

    //Methods which return class/struct/union by value have out pointer inserted
    // as first parameter after this pointer, with all arguments shifted right by 1 for it
    static R* ClassOverride(C* self, R* outReturnValue, A... args) {
        reinterpret_cast<Trampoline*>(funptr)(self, outReturnValue, args...);

        for (auto& handler : handlers) {
            handler(self, *outReturnValue, args...);
        }
        return outReturnValue;
    }
   
    //Normal scalar type call, where no additional arguments are inserted
    static R ScalarOverride(C* self, A... args) {
        auto r = reinterpret_cast<Trampoline*>(funptr)(self, args...);

        for (auto& handler : handlers) {
            handler(self, r, args...);
        }

        return r;
    }

    //Call for void return type - nothing special to do with void
    static void VoidOverride(C* self, A... args) {
        reinterpret_cast<Trampoline*>(funptr)(self, args...);

        for (auto& handler : handlers) {
            handler(self, args...);
        }
    }

    static void* getApplyCall1(std::true_type) {
        return (void*)VoidOverride; //true - type is void
    }

    static void* getApplyCall1(std::false_type) {
        return getApplyCall2(std::is_class<R>{}); //not a void, try call 2
    }

    static void* getApplyCall2(std::true_type) {
        return (void*)ClassOverride; //true - type is class
    }

    static void* getApplyCall2(std::false_type) {
        return getApplyCall3(std::is_union<R>{});
    }

    static void* getApplyCall3(std::true_type) {
        return (void*)ClassOverride; //true - type is union
    }

    static void* getApplyCall3(std::false_type) {
        return (void*)ScalarOverride; //false - type is scalar type
    }

    static void* getApplyCall() {
        return getApplyCall1(std::is_same<R, void>{});
    }
};

template <typename R, typename C, typename... A, R(C::*PMF)(A...) const, typename TargetClass>
typename HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::Trampoline *
    HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::funptr = nullptr;

template <typename R, typename C, typename... A, R(C::*PMF)(A...) const, typename TargetClass>
bool HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::bHookInitialized = false;

template <typename R, typename C, typename... A, R(C::*PMF)(A...) const, typename TargetClass>
TArray<typename HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::HandlerType> 
    HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::handlers = TArray<typename HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::HandlerType>();


#define CUST_SUBSCRIBE_METHOD_AFTER(MethodReference, Handler) \
HandlerRegistry<decltype(&MethodReference), &MethodReference, None>::InstallHook(#MethodReference); \
HandlerRegistry<decltype(&MethodReference), &MethodReference, None>::AddHandler(Handler);


/*
void registerManual() {
//#if !WITH_EDITOR
    if (!set){
        SML::Logging::debug(TEXT("Registering Manual Hooks"));
        const wchar_t* name = L"AFGCharacterPlayer::GetActiveEquipments";
        FString SymbolSearchName = name;

        Trampoline* handler = [](AFGCharacterPlayer const* self, TArray<AFGEquipment*>*return_value) -> TArray<AFGEquipment*>* {
            if (set){
                reinterpret_cast<Trampoline*>(funptr)(self, return_value);
                return return_value;
            }
            return return_value;
        };

        RegisterHookFunction(SymbolSearchName, (void*)handler, (void**)&funptr);
        set = true;
    }
    else {
        SML::Logging::debug(TEXT("Called Twice!"));
    }
//#endif
} //*/


#include "FGHookshot.h"

void registerCharacterHooks() {
    // friend class FGCharacterPlayer;

    // using namespace SML;
    SML::Logging::debug(TEXT("Registering Character Hooks"));
    // registerManual();
  //  static AFGHookshot hook = AFGHookshot();

    /*/ typedef void StructHandlerSignature(C const*, R&, A...);
    CUST_SUBSCRIBE_METHOD_AFTER(AFGCharacterPlayer::GetActiveEquipments, 
        [](AFGCharacterPlayer const* self, TArray<AFGEquipment*>& equipments) {
            equipments.Add(&hook);
        }
    ); //*/

    /*
    SUBSCRIBE_METHOD_AFTER(AFGCharacterPlayer::GetActiveEquipments, [](TArray<AFGEquipment*> equipments, AFGCharacterPlayer const* self) {
        for (auto& equip : equipments) {
            SML::Logging::debug(TEXT("Eqip"), *equip->GetPathName());
        }
    });*/



    // Before
    // SUBSCRIBE_METHOD(AFGCharacterPlayer::EquipEquipment, [](auto&, AFGCharacterPlayer* self, AFGEquipment* equipment) {
    //    // do some nice stuff there
    //    SML::Logging::debug(TEXT("EquipEquipment_BEFORE"));
    //});

    /*/ GetActor_Implementation()
    SUBSCRIBE_VIRTUAL_FUNCTION_AFTER(AFGCharacterPlayer, AFGCharacterPlayer::GetActor_Implementation,
        [](AActor* actor, AFGCharacterPlayer* self) {
        SML::Logging::debug(TEXT("GetActor_Implementation: "), *actor->GetPathName());
    });

    SUBSCRIBE_VIRTUAL_FUNCTION_AFTER(AFGCharacterPlayer, AFGCharacterPlayer::IsAlive_Implementation,
        [](bool alive, AFGCharacterPlayer const* self) {
        SML::Logging::debug(TEXT("IsAlive_Implementation: "), alive);
    }); */

    // After
    SUBSCRIBE_METHOD_AFTER(AFGCharacterPlayer::EquipEquipment, [](AFGCharacterPlayer* self, AFGEquipment* equipment) {
        // do some nice stuff there
        SML::Logging::debug(TEXT("EquipEquipment_AFTER: "), *equipment->GetPathName());

        /*
        I am looking at all UFGInventoryComponent in CharacterPlayer and I only see 3 of them.

UFGInventoryComponent is the class that holds Items and
UFGInventoryComponentEquipment is the version that can have multiple equipment but it can only have one active.
EquipementComponents can only holds one type of items (Arms or Back not both)
and it is accessed through `GetEquipmentSlot`.
So to increase the size of either you can simply do `GetEquipementSlot(ES_BACK)->Resize(7328939)`.
It will work in the UI as well as both Back & Arms are using the same widget class.
Nevertheless, you are still using `UFGInventoryComponentEquipment` which means you still only have one equipement active for each components.
        
Given my objective is to enable multiple equiped body item this will not do.
I have to either create more UFGInventoryComponentEquipment that will tick through some undefined mechanic
Or I hoping to modify the returned Array of `GetActiveEquipements()` with my additional items to get them to tick.



        I really do not see where that body inventory is but does not look to be part of the character

        The body inventory is not a property.
I guess you did`GetEquipementSlot(ES_BACK)` and then resized it but you still are using the `UFGInventoryComponentEquipment` class and that limits you to a single item active so it does not really achieve what I want.
        */


        /*
        UFGInventoryComponentEquipment* none = self->GetEquipmentSlot(EEquipmentSlot::ES_NONE);
        auto arms = self->GetEquipmentSlot(EEquipmentSlot::ES_ARMS);
        auto back = self->GetEquipmentSlot(EEquipmentSlot::ES_BACK);

        TArray<UFGInventoryComponentEquipment*> all;
        all.Add(none);
        all.Add(arms);
        all.Add(back);

        none->

        for (UFGInventoryComponentEquipment* inventory : all) {
            if (inventory != nullptr){
                SML::Logging::debug(
                    *inventory->GetPathName(),
                    inventory->
                );
            }
            else {
                SML::Logging::debug("Nullptr");
            }
        } */

   


        /* String Formating Example
         */
        // FString message = FString::Printf(TEXT("Failed to initialize module %s: InitializeModule() function not found"), *modid);
        // SML::Logging::error(*message);


        /* Returns
         * [DEBUG] Equipment: /Game/[...].BP_ResourceScanner_C_0
         * [DEBUG] Equipment: /Game/[...].Equip_NobeliskDetonator_C_1
         * [DEBUG] Equipment: /Game/[...].Equip_JumpingStilts_C_0
         */ /*
        TArray< AFGEquipment* > equipments = self->GetActiveEquipments();
        for (auto& equipment : equipments) {
            SML::Logging::debug("Equipment: ", *equipment->GetPathName());
        } */
    });


    //*/ Start >>>>
    // AFGCharacterPlayer::GetActiveEquipments


    /*/
    const SymbolDigestInfo DigestInfo = 
        SML::GetBootstrapperAccessors().DigestGameSymbol(*SymbolSearchName);

    SML::Logging::info(*FString::Printf(TEXT("Hooking symbol with search name %s"), *SymbolSearchName));
    if (DigestInfo.bSymbolNotFound) {
        SML::Logging::fatal(*FString::Printf(TEXT("Hooking symbol %s failed: symbol not found in game executable"), *SymbolSearchName));
        return;
    }
    if (DigestInfo.bSymbolOptimizedAway) {
        SML::Logging::fatal(*FString::Printf(TEXT("Hooking symbol %s failed: Symbol is not present in game executable as it was optimized away (inlined/stripped)"), *SymbolSearchName));
        return;
    }
    if (DigestInfo.bMultipleSymbolsMatch) {
        SML::Logging::fatal(*FString::Printf(TEXT("Hooking symbol %s failed: Multiple symbols matching that name found. Please use exact decorated name with MANUAL macros instead"), *SymbolSearchName));
        return;
    }
    FString SymbolId = DigestInfo.SymbolName.String;
    if (DigestInfo.bSymbolVirtual) {
        //Warn about hooking virtual function implementation without using SUBSCRIBE_VIRTUAL_METHOD
        SML::Logging::warning(*FString::Printf(TEXT("Warning: Hooking virtual function implementation with SUBSCRIBE_METHOD macro. You are hooking it for all classes who don't specifically have overrides, so be very careful with it. Use SUBSCRIBE_VIRTUAL_METHOD for more wise control and ability to override virtual function for exact class directly. Function: %s"), *SymbolSearchName));
    }

    /*
    HookStandardFunction(
        SymbolId,
        DigestInfo.SymbolImplementationPointer,
        HookFunctionPointer,
        OutTrampolineFunction);
    * /
#define CHECK_FUNCHOOK_ERR(arg) \
	if (arg != FUNCHOOK_ERROR_SUCCESS) SML::Logging::fatal(*FString::Printf(TEXT("Hooking symbol %s failed: funchook failed: %hs"), *SymbolId, funchook_error_message(funchook)));

    TArray<AFGEquipment*>(*handler)(AFGCharacterPlayer const*) = [](AFGCharacterPlayer const* self) -> TArray<AFGEquipment*> {
        auto r = self->GetActiveEquipments();
        return r;
    };

    void* c_handler = (void*)handler;

    funchook* funchook = funchook_create();  
    if (funchook == nullptr) {
        SML::Logging::fatal(*FString::Printf(TEXT("Hooking symbol %s failed: funchook_create() returned NULL"), *SymbolId));
    } else {
        CHECK_FUNCHOOK_ERR(funchook_prepare(
            funchook, 
            (void**)&DigestInfo.SymbolImplementationPointer, 
            c_handler));
        CHECK_FUNCHOOK_ERR(funchook_install(funchook, 0));
    }

#undef CHECK_FUNCHOOK_ERR
    SML::Logging::info(*FString::Printf(TEXT("Successfully hooked normal function %s"), *SymbolId));
    DigestInfo.SymbolName.Free();
    */
    // Done <<<<
    //*/

    /*
    SUBSCRIBE_METHOD(AFGCharacterPlayer::GetActiveEquipments, [](auto&, AFGCharacterPlayer const* self) {
        // do some nice stuff there
        SML::Logging::debug(TEXT("GetActiveEquipments_Before"));


    }); //*/

    //*

    // HookInvoker<decltype(&MethodReference), &MethodReference, None>::
    //  -> InstallHook(#MethodReference); 
    //      Only executed if the Method was not hooked before
    //      Get the ApplyCall we should use to execute the original function
    // 
    //
    //  Fetch where the function we are hooking to is
    //  SML_API FString RegisterHookFunction(
    //      const FString& SymbolSearchName, 
    //      void* HookFunctionPointer, 
    //      void** OutTrampolineFunction)
    //
    //  bool HookStandardFunction(
    //      const FString& SymbolId, 
    //      void* OriginalFunctionPointer, 
    //      void* HookFunctionPointer,        // Our hook
    //      void** OutTrampolineFunction)     // Set on success
	


    // HookInvoker<decltype(&MethodReference), &MethodReference, None>::
    //  -> addHandlerBefore(Handler);
    //      Insert our lambda into a list of function to call for that method

    // ScopeType scope(
    //      handlersBefore, 
    //      reinterpret_cast<R(*)(C const*, A...)>(&TrampolineFunctionCall));
    // Entry Point of when the function is called


    /*/ AFTER
    // This is not called anymore
    SUBSCRIBE_METHOD_AFTER(AFGCharacterPlayer::GetActiveEquipments, [](TArray< AFGEquipment*> elems, AFGCharacterPlayer const* self) {
        // do some nice stuff there
        SML::Logging::debug(TEXT("GetActiveEquipments"), elems.Num());
    }); //*/ 

    //*

    /*
0x00007ff7bdfa8a97 FactoryGame-Win64-Shipping.exe!ReportAssert() [engine\source\runtime\core\private\windows\windowsplatformcrashcontext.cpp:553]
0x00007ff7bdfaaa08 FactoryGame-Win64-Shipping.exe!FWindowsErrorOutputDevice::Serialize() [engine\source\runtime\core\private\windows\windowserroroutputdevice.cpp:79]
0x00007ff7bdf64cea FactoryGame-Win64-Shipping.exe!FOutputDevice::LogfImpl() [engine\source\runtime\core\private\misc\outputdevice.cpp:71]
0x00007ff7bdf262a7 FactoryGame-Win64-Shipping.exe!AssertFailedImplV() [engine\source\runtime\core\private\misc\assertionmacros.cpp:101]
0x00007ff7bdf26350 FactoryGame-Win64-Shipping.exe!FDebug::CheckVerifyFailedImpl() [engine\source\runtime\core\private\misc\assertionmacros.cpp:439]
0x00007ff7bdeb18de FactoryGame-Win64-Shipping.exe!FMallocBinned2::MallocExternalLarge() [engine\source\runtime\core\private\hal\mallocbinned2.cpp:788]
0x00007ff7bdeb406c FactoryGame-Win64-Shipping.exe!FMallocBinned2::ReallocExternal() [engine\source\runtime\core\private\hal\mallocbinned2.cpp:856]
0x00007ff7bdeb3d62 FactoryGame-Win64-Shipping.exe!FMallocBinned2::Realloc() [engine\source\runtime\core\public\hal\mallocbinned2.h:534]
0x00007ff7bd6d8109 FactoryGame-Win64-Shipping.exe!TArray<UNetReplicationGraphConnection *,FDefaultAllocator>::ResizeForCopy() [engine\source\runtime\core\public\containers\array.h:2486]
0x00007ff7bdbc92df FactoryGame-Win64-Shipping.exe!AFGCharacterPlayer::GetActiveEquipments() [factorygame\source\factorygame\private\fgcharacterplayer.cpp:1287]
0x00007ff9ffb750fc UE4-ExampleMod-Win64-Shipping.dll!CallScope<TArray<AFGEquipment *,FDefaultAllocator> (__cdecl*)(AFGCharacterPlayer const *)>::operator()() [c:\users\newton\work\satisfactory_mod\satisfactorymodloader-2.2.0\source\sml\mod\hooking.h:117]
0x00007ff9ffb753a7 UE4-ExampleMod-Win64-Shipping.dll!std::_Func_impl_no_alloc<<lambda_8797026833e804d7d502090ade4983d3>,void,CallScope<TArray<AFGEquipment *,FDefaultAllocator> (__cdecl*)(AFGCharacterPlayer const *)> &,AFGCharacterPlayer const *>::_Do_call() [c:\program files (x86)\microsoft visual studio\2017\community\vc\tools\msvc\14.16.27023\include\functional:16707566]
0x00007ff9ffb750a8 UE4-ExampleMod-Win64-Shipping.dll!CallScope<TArray<AFGEquipment *,FDefaultAllocator> (__cdecl*)(AFGCharacterPlayer const *)>::operator()() [c:\users\newton\work\satisfactory_mod\satisfactorymodloader-2.2.0\source\sml\mod\hooking.h:123]
0x00007ff9ffb755c3 UE4-ExampleMod-Win64-Shipping.dll!HookInvoker<TArray<AFGEquipment *,FDefaultAllocator> (__cdecl AFGCharacterPlayer::*)(void)const ,{AFGCharacterPlayer::GetActiveEquipments,0},None>::applyCallScalar() [c:\users\newton\work\satisfactory_mod\satisfactorymodloader-2.2.0\source\sml\mod\hooking.h:455]

*/

    /*
    SUBSCRIBE_METHOD(AFGCharacterPlayer::GetActiveEquipments, 
        [](auto& Call, AFGCharacterPlayer const* self) -> void {
            
            auto mem = Call(self);

            // TArray< AFGEquipment* > mem = Call(self);
            SML::Logging::debug(TEXT("GetActiveEquipments_After: "), mem.Num());

            Call.Override(mem);
    }); 
    //*/

    // Before
    /*/  GetActiveEquipments is const and this hooks does not work
    SUBSCRIBE_METHOD(AFGCharacterPlayer::GetActiveEquipments, [](auto&) {
        // do some nice stuff there
        SML::Logging::fatal(TEXT("IsDrivingVehicle"));
    });

    // AFTER
    SUBSCRIBE_METHOD_AFTER(AFGCharacterPlayer::GetActiveEquipments, [](TArray< AFGEquipment* >&, AFGCharacterPlayer const* self) {
        // do some nice stuff there
        SML::Logging::fatal(TEXT("IsDrivingVehicle"));
    });*/
}

