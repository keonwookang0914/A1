// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1CmdWidget.h"

#include "A1LogChannels.h"
#include "A1GameplayTags.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Actors/A1RepairBase.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Components/ProgressBar.h"
#include "Data/A1CmdData.h"
#include "Data/A1UIData.h"
#include "EngineUtils.h"
#include "Components/Image.h"
#include "GameModes/LyraGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"
#include "Score/A1ScoreManager.h"

UA1CmdWidget::UA1CmdWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    EscapeMode = false;
    TutoMode = false; //default
}

void UA1CmdWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageListenerHandle = MessageSubsystem.RegisterListener(MessageChannelTag, this, &UA1CmdWidget::ConstructUI);
    
}

void UA1CmdWidget::NativeDestruct()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.UnregisterListener(MessageListenerHandle);
}

void UA1CmdWidget::ConstructUI(FGameplayTag Channel, const FASCInitializeMessage& Message)
{
	if (Message.ASC == nullptr)
		return;

	//GA_Interact_Cmd에서 ScoreManager를 통해 설정됨.
	ASC = Message.ASC;
	TutoMode = Message.bTutorial;

    if (TutoMode)
    {
        ShowTutoMode();
    }
    else
    {
        ShowMenu();
        InputText->SetFocus();
    }
}

void UA1CmdWidget::InputEnded(FText InText)
{
    if (InText.ToString() == TEXT(""))
        return;

    UE_LOG(LogA1System, Log, TEXT("%s"), *InText.ToString());

    SuperviseText->SetVisibility(ESlateVisibility::Hidden);
    SuperviseText->SetText(FText::FromString(""));
    InputText->SetText(FText::FromString(""));

    if (EscapeMode)
    {
        if (InText.ToString() == TEXT("Confirm"))
        {
            EscapeScreen->SetVisibility(ESlateVisibility::Hidden);
            if (TutoMode)    // tutorial
            {
                // TODO eric1306
                // Fade Out / Temp               
                UE_LOG(LogA1, Log, TEXT("Fade Out!"));
                if (ALyraGameMode* GameMode = Cast<ALyraGameMode>(GetWorld()->GetAuthGameMode()))
                {
                    GameMode->TriggerFadeOnAllPlayer(0.f, 1.f, 1.f, FLinearColor::White);
                    LoadStory();
                }
            }
            else             // InGame
            {
                TSubclassOf<UUserWidget> EndingCutSceneClass = UA1UIData::Get().EndingCutSceneClass;
                UUserWidget* EndingCutScene = CreateWidget<UUserWidget>(GetWorld(), EndingCutSceneClass);
                if (EndingCutScene)
                    EndingCutScene->AddToViewport();

                // TEMP
                if (ASC)
                {
                    FGameplayEventData Payload;
                    ASC->HandleGameplayEvent(A1GameplayTags::GameplayEvent_Cmd_Exit, &Payload);
                }

                // // TODO eric1306 
                // GameOver 처리
                UA1ScoreManager::Get()->SetRepairedBeforeEscape(true);
                UA1ScoreBlueprintFunctionLibrary::EndGame(EGameEndReason::Escape);
            }
            Destruct();
            return;
        }
        else if (InText.ToString() == TEXT("Deny"))
        {
            ShowMenu();
            ShowRepairPercent(0.0f);
            return;
        }
        EscapeMode = false;
    }

    if (TutoMode == false)
    {
        if (InText.ToString() == TEXT("Menu"))
        {
            if (MenuBox->GetVisibility() != ESlateVisibility::Visible)
                ShowMenu();
        }

        // Map Open
        else if (InText.ToString() == TEXT("Map"))
        {
            MapScreen->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            HiddenMenu();
            SuperviseText->SetVisibility(ESlateVisibility::Visible);

            // 현 상황에 맞는 도움말 제공
            if (InText.ToString() == TEXT("Help"))
            {
                const FCmdTextSet* TextSet = UA1CmdData::Get().GetTextSetByLabel("Help");
                const FString* Text = TextSet->TextEntries.Find("Tutorial");

                if (Text != nullptr)
                    AffectTypingEffect(SuperviseText, *const_cast<FString*>(Text), TypingDelta / 4, 0.0f);
            }
            //else if (InText.ToString() == TEXT("Document"))
            //{
            //
            //}
            else if (InText.ToString() == TEXT("Escape"))
            {
                EscapeScreen->SetVisibility(ESlateVisibility::Visible);

                UWorld* World = GetWorld();

                int Total = 0;
                int32 Count = 0;
                for (TActorIterator<AA1RepairBase> It(World); It; ++It)
                {
                    if (It->CurrentState == RepairState::NotBroken)
                        continue;

                    Total++;

                    if (It->CurrentState == RepairState::Complete)
                        ++Count;
                }

                float percent = (Total == 0) ? 0 : ((float)Count / Total);
                ShowRepairPercent(percent);

                EscapeGuideTxt->SetVisibility(ESlateVisibility::Visible);
                EscapeKeyTxt->SetVisibility(ESlateVisibility::Visible);

                FName EntryLable = "";
                FString KeyText = "";
                if (Total != Count)
                {
                    EntryLable = "Cannot";
                    KeyText = ">  Menu";
                }
                else
                {
                    EscapeMode = true;
                    EntryLable = "Can";
                    KeyText = ">  Confirm / Deny";
                }

                const FCmdTextSet* TextSet = UA1CmdData::Get().GetTextSetByLabel("Escape");
                const FString* Text = TextSet->TextEntries.Find(EntryLable);
                if (Text != nullptr)
                {
                    AffectTypingEffect(EscapeGuideTxt, *const_cast<FString*>(Text), TypingDelta, 0.0f);
                    AffectTypingEffect(EscapeKeyTxt, KeyText, TypingDelta, Text->Len() * TypingDelta);
                }
            }
        }
    }
    if (InText.ToString() == TEXT("Exit"))
    {
        MapScreen->SetVisibility(ESlateVisibility::Hidden);
        Destruct();
        if (ASC)
        {
            FGameplayEventData Payload;
            ASC->HandleGameplayEvent(A1GameplayTags::GameplayEvent_Cmd_Exit, &Payload);

			if (UA1ScoreManager::Get()->GetDoTutorial())
			{
				FGameplayEventData EventData;
				UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
				MessageSubsystem.BroadcastMessage(A1GameplayTags::Tutorial_Interact_CMD, EventData);
			}
        }

        return;
    }
    // 없는 명령어 입력
    else
    {
        SuperviseText->SetVisibility(ESlateVisibility::Visible);

        const FCmdTextSet* TextSet = UA1CmdData::Get().GetTextSetByLabel("Error");
        const FString* Text = TextSet->TextEntries.Find("Invalid");

        EscapeScreen->SetVisibility(ESlateVisibility::Hidden);
        EscapeGuideTxt->SetText(FText::FromString(""));
        EscapeKeyTxt->SetText(FText::FromString(""));

        if (Text != nullptr)
            AffectTypingEffect(SuperviseText, *const_cast<FString*>(Text), TypingDelta, 0.0f);
    }


    InputText->SetFocus();
}

void UA1CmdWidget::ShowTutoMode()
{
    EscapeGuideTxt->SetText(FText::FromString(""));
    EscapeKeyTxt->SetText(FText::FromString(""));
    SuperviseText->SetText(FText::FromString(""));

    MenuBox->SetVisibility(ESlateVisibility::Visible);
    EscapeScreen->SetVisibility(ESlateVisibility::Hidden);
    SuperviseText->SetVisibility(ESlateVisibility::Hidden);

    AffectTypingEffect(MenuText1, TEXT("[ 시스템 상태 ]"), TypingDelta, 0.5f);
    AffectTypingEffect(MenuText2, TEXT("  냉각 시스템 : 오류"), TypingDelta, 1.0f);
    AffectTypingEffect(MenuText3, TEXT("  선체 손상: 감지됨 "), TypingDelta, 1.5f);
    AffectTypingEffect(MenuText4, TEXT("  항해 가능 여부 : 불가능 "), TypingDelta, 2.0f);
    AffectTypingEffect(MenuText5, TEXT("※ 고장 해결 전까지 재수면 불가 ※ "), TypingDelta, 2.7f);

    InputText->SetText(FText::FromString("Exit"));
}

void UA1CmdWidget::ShowMenu()
{
    EscapeGuideTxt->SetText(FText::FromString(""));
    EscapeKeyTxt->SetText(FText::FromString(""));
    SuperviseText->SetText(FText::FromString(""));

    MenuBox->SetVisibility(ESlateVisibility::Visible);
    EscapeScreen->SetVisibility(ESlateVisibility::Hidden);
    SuperviseText->SetVisibility(ESlateVisibility::Hidden);

    AffectTypingEffect(MenuText1, ">  Map", TypingDelta/6, 0.5f);
    AffectTypingEffect(MenuText2, ">  Help", TypingDelta/7, 0.5f);
    //AffectTypingEffect(MenuText3, ">  Documents", TypingDelta/12, 0.5f);
    AffectTypingEffect(MenuText3, ">  Escape", TypingDelta/9, 0.5f);
    AffectTypingEffect(MenuText4, ">  Exit", TypingDelta/7, 0.5f);
}

void UA1CmdWidget::HiddenMenu()
{
    MenuBox->SetVisibility(ESlateVisibility::Hidden);
    MenuText1->SetText(FText::FromString(""));
    MenuText2->SetText(FText::FromString(""));
    MenuText3->SetText(FText::FromString(""));
    MenuText4->SetText(FText::FromString(""));
    MenuText5->SetText(FText::FromString(""));
}