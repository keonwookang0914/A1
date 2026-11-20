#pragma once

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


#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define LOG_CALLINFO(Verbosity) UE_LOG(LogD1, Verbosity, TEXT("%s"), *CALLINFO)

#define LOG(Verbosity, Format, ...) UE_LOG(LogD1, Verbosity, TEXT("%s %s"), *CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_WARNING(Format, ...) UE_LOG(LogD1, Warning, TEXT("%s %s"), *CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_ERROR(Format, ...) UE_LOG(LogD1, Error, TEXT("%s %s"), *CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

#define	LOG_SCREEN(Format, ...) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(Format, ##__VA_ARGS__))
#define	LOG_SCREEN_CONTEXT(ContextObject, Format, ...) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s: %s"), *GetClientServerContextString(ContextObject), *FString::Printf(Format, ##__VA_ARGS__)))
#define	LOG_SCREEN_ORDER(Order, Format, ...) GEngine->AddOnScreenDebugMessage(Order, 5.f, FColor::Red, FString::Printf(Format, ##__VA_ARGS__))
#define	LOG_SCREEN_COLOR(Color, Format, ...) GEngine->AddOnScreenDebugMessage(-1, 5.f, Color, FString::Printf(Format, ##__VA_ARGS__))

A1GAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
