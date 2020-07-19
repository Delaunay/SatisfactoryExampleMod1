#pragma once

#include "CoreMinimal.h"
#include "SatisfactoryModLoader.h"
#include "mod/hooking.h"


template <typename TCallable, TCallable Callable, typename TargetClass>
struct MethodTrait;


template <typename R, typename C, typename... A, R(C::*PMF)(A...), typename TargetClass>
struct MethodTrait<R(C::*)(A...), PMF, TargetClass> {
    typedef R* StructMethodSignature(C*, R*, A...);
    typedef R* UnionMethodSignature(C*, R*, A...);
    typedef R ScalarMethodSignature(C*, A...);

    typedef void StructHandlerSignature(C const*, R&, A...);
    typedef void UnionHandlerSignature(C const*, R&, A...);
    typedef void ScalarHandlerSignature(C const*, R&, A...);
    typedef void VoidHandlerSignature(C const*, A...);

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
    typedef void UnionHandlerSignature(C const*, R&, A...);
    typedef void ScalarHandlerSignature(C const*, R&, A...);
    typedef void VoidHandlerSignature(C const*, A...);

    typedef void StructOverrideSignature(std::function<void()>,  C const*, R&, A...);
    typedef void UnionOverrideSignature(std::function<void()>, C const*, R&, A...);
    typedef R    ScalarOverrideSignature(std::function<void()>, C const*, A...);
    typedef void VoidOverrideSignature(std::function<void()>, C const*, A...);

    typedef typename std::conditional <std::is_same<R, void>::value, ScalarMethodSignature,
        typename std::conditional<std::is_class<R>::value, StructMethodSignature,
        typename std::conditional<std::is_union<R>::value, UnionMethodSignature, ScalarMethodSignature>::type>::type>::type FunctionType;

    typedef typename std::conditional <std::is_same<R, void>::value, VoidHandlerSignature,
        typename std::conditional<std::is_class<R>::value, StructHandlerSignature,
        typename std::conditional<std::is_union<R>::value, UnionHandlerSignature, ScalarHandlerSignature>::type>::type>::type HandlerType;

    typedef typename std::conditional <std::is_same<R, void>::value, VoidOverrideSignature,
        typename std::conditional<std::is_class<R>::value, StructOverrideSignature,
        typename std::conditional<std::is_union<R>::value, UnionOverrideSignature, ScalarOverrideSignature>::type>::type>::type OverrideType;
};


template <typename TCallable, TCallable Callable, typename TargetClass>
class HandlerRegistry;


template <typename R, typename C, typename... A, R(C::*PMF)(A...) const, typename TargetClass>
class HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass> {
public:
    using Trampoline = typename MethodTrait<R(C::*)(A...) const, PMF, TargetClass>::FunctionType;
    using HandlerSignature = typename MethodTrait<R(C::*)(A...) const, PMF, TargetClass>::HandlerType;
    using OverrideSignature = typename MethodTrait<R(C::*)(A...) const, PMF, TargetClass>::OvererideType;

    typedef std::function<HandlerSignature> HandlerType;
    typedef std::function<OverrideSignature> OverrideType;

    static Trampoline*          funptr;
    static bool                 bHookInitialized;
    static TArray<HandlerType>  handlers;
    static OverrideType         funoverride;

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

    static void AddOverride(OverrideType fun) {
#if !WITH_EDITOR
        funoverride = fun;
#endif
    }

    //Methods which return class/struct/union by value have out pointer inserted
    // as first parameter after this pointer, with all arguments shifted right by 1 for it
    static R* ClassOverride(C* self, R* outReturnValue, A... args) {
        auto original = [&]() {
            reinterpret_cast<Trampoline*>(funptr)(self, outReturnValue, args...);
        };

        if (!funoverride){
            original();
        }
        else {
            funoverride(original, self, *outReturnValue, args...);
        }

        for (auto& handler : handlers) {
            handler(self, *outReturnValue, args...);
        }
        return outReturnValue;
    }

    //Normal scalar type call, where no additional arguments are inserted
    static R ScalarOverride(C* self, A... args) {
        R r;

        auto original = [&]() -> R {
            return reinterpret_cast<Trampoline*>(funptr)(self, args...);
        }

        if (!funoverride) {
            r = original();
        }
        else {
            r = funoverride(original, self, args...);
        }
        
        for (auto& handler : handlers) {
            handler(self, r, args...);
        }

        return r;
    }

    //Call for void return type - nothing special to do with void
    static void VoidOverride(C* self, A... args) {
        auto original = [&]() {
            reinterpret_cast<Trampoline*>(funptr)(self, args...);
        }

        if (!funoverride) {
            original();
        }
        else {
            funoverride(original, self, args...);
        }

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
typename HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::OverrideType
HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::funoverride = nullptr;

template <typename R, typename C, typename... A, R(C::*PMF)(A...) const, typename TargetClass>
bool HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::bHookInitialized = false;

template <typename R, typename C, typename... A, R(C::*PMF)(A...) const, typename TargetClass>
TArray<typename HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::HandlerType>
HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::handlers = TArray<typename HandlerRegistry<R(C::*)(A...) const, PMF, TargetClass>::HandlerType>();


#define CUST_SUBSCRIBE_METHOD_AFTER(MethodReference, Handler) \
HandlerRegistry<decltype(&MethodReference), &MethodReference, None>::InstallHook(#MethodReference); \
HandlerRegistry<decltype(&MethodReference), &MethodReference, None>::AddHandler(Handler);

#define CUST_OVERRIDE_METHOD_AFTER(MethodReference, Handler) \
HandlerRegistry<decltype(&MethodReference), &MethodReference, None>::InstallHook(#MethodReference); \
HandlerRegistry<decltype(&MethodReference), &MethodReference, None>::AddOverride(Handler);

