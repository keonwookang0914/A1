#include "Teams/A1TeamAgentInterface.h"

#include "A1LogChannels.h"
#include "Messages/LyraVerbMessage.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1TeamAgentInterface)

UA1TeamAgentInterface::UA1TeamAgentInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void IA1TeamAgentInterface::ConditionalBroadcastTeamChanged(TScriptInterface<IA1TeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID)
{
	if (OldTeamID != NewTeamID)
	{
		const int32 OldTeamIndex = GenericTeamIdToInteger(OldTeamID); 
		const int32 NewTeamIndex = GenericTeamIdToInteger(NewTeamID);

		UObject* ThisObj = This.GetObject();
		UE_LOG(LogA1Teams, Verbose, TEXT("[%s] %s assigned team %d"), *GetClientServerContextString(ThisObj), *GetPathNameSafe(ThisObj), NewTeamIndex);

		This.GetInterface()->GetTeamChangedDelegateChecked().Broadcast(ThisObj, OldTeamIndex, NewTeamIndex);
	}
}

ETeamAttitude::Type IA1TeamAgentInterface::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const APawn* OtherPawn = Cast<APawn>(&Other))
	{
		if (const IA1TeamAgentInterface* TeamAgent = Cast<IA1TeamAgentInterface>(OtherPawn->GetController()))
		{
			const int32 MyTeamID = GenericTeamIdToInteger(GetGenericTeamId());
			const int32 OtherTeamID = GenericTeamIdToInteger(TeamAgent->GetGenericTeamId());

			if ((MyTeamID == FGenericTeamId::NoTeam) || (OtherTeamID == FGenericTeamId::NoTeam) || (MyTeamID != OtherTeamID))
			{
				return ETeamAttitude::Hostile;
			}
			else
			{
				return ETeamAttitude::Friendly;
			}
		}
	}

	return ETeamAttitude::Neutral;
}
