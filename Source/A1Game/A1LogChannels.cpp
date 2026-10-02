#include "A1LogChannels.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogA1);
DEFINE_LOG_CATEGORY(LogA1System);
DEFINE_LOG_CATEGORY(LogA1Player);
DEFINE_LOG_CATEGORY(LogA1Raider);
DEFINE_LOG_CATEGORY(LogA1Experience);
DEFINE_LOG_CATEGORY(LogA1AbilitySystem);
DEFINE_LOG_CATEGORY(LogA1Teams);
DEFINE_LOG_CATEGORY(LogA1ScoreSystem);
DEFINE_LOG_CATEGORY(LogA1Cliff);
DEFINE_LOG_CATEGORY(LogA1Tutorial);
DEFINE_LOG_CATEGORY(LogMap);

FString GetClientServerContextString(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("[Server]") : TEXT("[Client]");
	}
	else
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			return GPlayInEditorContextString;
		}
#endif
	}

	return TEXT("[None]");
}

ENetMode GetLogContextNetMode(const UObject* ContextObject)
{
	if (const AActor* Actor = Cast<AActor>(ContextObject))
	{
		return Actor->GetNetMode();
	}
	else if (const UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		return Component->GetNetMode();
	}
	else if (const UWorld* World = ContextObject ? ContextObject->GetWorld() : nullptr)
	{
		return World->GetNetMode();
	}

	return NM_MAX;
}

FString GetNetModeLogString(ENetMode NetMode)
{
	switch (NetMode)
	{
	case NM_Client:
		return FString::Printf(TEXT("CLIENT_ID%d"), static_cast<int32>(GPlayInEditorID));
	case NM_Standalone:
		return TEXT("STANDALONE");
	case NM_DedicatedServer:
	case NM_ListenServer:
		return TEXT("SERVER");
	default:
		return TEXT("NONE");
	}
}
