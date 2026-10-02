#pragma once

#include "Engine/EngineBaseTypes.h"
#include "Logging/LogMacros.h"

A1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogA1, Log, All);
A1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogA1System, Log, All);
A1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogA1Player, Log, All);
A1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogA1Raider, Log, All);
A1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogA1Experience, Log, All);
A1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogA1AbilitySystem, Log, All);
A1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogA1Teams, Log, All);
A1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogA1ScoreSystem, Log, All);
A1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogA1Cliff, Log, All);
A1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogA1Tutorial, Log, All);
A1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogMap, Log, All);

#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))

// 일반 로그. "함수명(줄번호) 메시지"로 출력한다.
#define A1_LOG(Category, Verbosity, Format, ...) UE_LOG(Category, Verbosity, TEXT("%s %s"), *CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

// 네트워크 처리가 들어간 코드의 로그. "[넷모드] 함수명(줄번호) 메시지"로 출력한다.
// this에서 넷모드를 구하므로 UObject의 멤버 함수 안에서만 쓸 수 있다.
#define A1_NETLOG(Category, Verbosity, Format, ...) UE_LOG(Category, Verbosity, TEXT("[%s] %s %s"), *GetNetModeLogString(GetLogContextNetMode(this)), *CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

#define LOG_SCREEN(Format, ...) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(Format, ##__VA_ARGS__))
#define LOG_SCREEN_CONTEXT(ContextObject, Format, ...) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s: %s"), *GetClientServerContextString(ContextObject), *FString::Printf(Format, ##__VA_ARGS__)))
#define LOG_SCREEN_ORDER(Order, Format, ...) GEngine->AddOnScreenDebugMessage(Order, 5.f, FColor::Red, FString::Printf(Format, ##__VA_ARGS__))
#define LOG_SCREEN_COLOR(Color, Format, ...) GEngine->AddOnScreenDebugMessage(-1, 5.f, Color, FString::Printf(Format, ##__VA_ARGS__))

A1GAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);

// 액터와 컴포넌트는 자신의 넷모드를, 그 밖의 객체는 월드의 넷모드를 돌려준다. 월드를 찾지 못하면 NM_MAX다.
A1GAME_API ENetMode GetLogContextNetMode(const UObject* ContextObject);

// SERVER, STANDALONE, CLIENT_ID<PIE 인스턴스 번호>, NONE 중 하나를 돌려준다.
A1GAME_API FString GetNetModeLogString(ENetMode NetMode);
